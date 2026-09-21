#include "../test_source_library.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/definition.hpp>
#include <givm/executor.hpp>
#include <givm/table.hpp>

namespace
{
    struct dependency_observation
    {
        bool handled = false;
        givm::definition_id<givm::support_view> alpha_support;
        givm::definition_id<givm::support_view> beta_support;
        givm::tag_id chosen_tag;
        std::vector<givm::definition_id<givm::support_view>> filtered_supports;
    };

    struct tagged_support_source
    {
        using definition_category = givm::support_view;

        struct definition_type{};

        std::string_view source_name;
        std::array<std::string_view, 2> source_tags{};
        std::size_t tag_count = 0;

        constexpr std::string_view name() const noexcept
        {
            return source_name;
        }

        constexpr std::span<const std::string_view> tags() const noexcept
        {
            return { source_tags.data(), tag_count };
        }

        constexpr definition_type compile(givm::definition_compile_context&) const noexcept
        {
            return {};
        }
    };

    struct dependent_card_source
    {
        using definition_category = givm::card_definition;

        struct definition_type
        {
            dependency_observation* observation;
            givm::definition_id<givm::support_view> alpha_support;
            givm::definition_id<givm::support_view> beta_support;
            givm::tag_id chosen_tag;
            std::vector<givm::definition_id<givm::support_view>> filtered_supports;
        };

        dependency_observation* observation;

        constexpr std::string_view name() const noexcept
        {
            return "DependentCard";
        }

        constexpr auto support_dependencies() const noexcept
        {
            return std::array<std::string_view, 2>{ "AlphaSupport", "BetaSupport" };
        }

        constexpr auto tag_dependencies() const noexcept
        {
            return std::array<std::string_view, 1>{ "chosen" };
        }

        constexpr auto support_dependencies_by_tag() const noexcept
        {
            return std::array<std::string_view, 1>{ "selected & !excluded" };
        }

        definition_type compile(givm::definition_compile_context& context) const
        {
            return {
                .observation = observation,
                .alpha_support = context.resolve_id<givm::support_view>("AlphaSupport"),
                .beta_support = context.resolve_id<givm::support_view>("BetaSupport"),
                .chosen_tag = context.resolve_tag("chosen"),
                .filtered_supports =
                    context.resolve_ids_by_tag<givm::support_view>("selected & !excluded")
            };
        }

        static givm::program_entry handle(
            const definition_type& definition,
            const givm::hand_card_view&,
            givm::test_event&,
            givm::handle_context&)
        {
            definition.observation->handled = true;
            definition.observation->alpha_support = definition.alpha_support;
            definition.observation->beta_support = definition.beta_support;
            definition.observation->chosen_tag = definition.chosen_tag;
            definition.observation->filtered_supports = definition.filtered_supports;
            return {};
        }
    };

    struct program_observation
    {
        bool event_compiled = false;
        bool onpay_compiled = false;
        givm::definition_id<givm::support_view> resolved_support;
        bool first_event_entry_set = false;
        bool second_event_entry_set = false;
        bool onpay_entry_set = false;
    };

    struct programmed_card_source
    {
        using definition_category = givm::card_definition;

        struct definition_type
        {
            program_observation* observation;
            givm::definition_id<givm::support_view> support;
            givm::program_entry first_entry;
            givm::program_entry second_entry;
        };

        program_observation* observation;

        constexpr std::string_view name() const noexcept
        {
            return "ProgrammedCard";
        }

        constexpr auto support_dependencies() const noexcept
        {
            return std::array<std::string_view, 1>{ "ProgrammedSupport" };
        }

        definition_type compile(givm::definition_compile_context& context) const
        {
            observation->event_compiled = true;
            const auto support = context.resolve_id<givm::support_view>("ProgrammedSupport");
            observation->resolved_support = support;

            const auto first_entry = context.add_program(std::tuple{
                givm::draw_cards{ .count = 0 },
                givm::shuffle_deck{ .player = givm::player_id{ 0 } }
            });
            const auto second_entry = context.add_program(
                std::vector{ givm::draw_cards{ .count = 0 } }
            );
            observation->first_event_entry_set = bool{ first_entry };
            observation->second_event_entry_set = bool{ second_entry };
            return {
                .observation = observation,
                .support = support,
                .first_entry = first_entry,
                .second_entry = second_entry
            };
        }
    };

    struct programmed_support_source
    {
        using definition_category = givm::support_view;

        struct definition_type
        {
            program_observation* observation;
            givm::program_entry onpay_entry;
        };

        program_observation* observation;

        constexpr std::string_view name() const noexcept
        {
            return "ProgrammedSupport";
        }

        definition_type compile(givm::definition_compile_context& context) const
        {
            observation->onpay_compiled = true;
            using instruction_type = givm::any_command;
            const auto entry = context.add_program(std::vector{
                instruction_type{ givm::draw_cards{ .count = 0 } },
                instruction_type{ givm::draw_cards{ .count = 0 } }
            });
            observation->onpay_entry_set = bool{ entry };
            return { .observation = observation, .onpay_entry = entry };
        }
    };

    struct undeclared_dependency_source
    {
        using definition_category = givm::card_definition;

        struct definition_type{};

        constexpr std::string_view name() const noexcept
        {
            return "UndeclaredDependency";
        }

        definition_type compile(givm::definition_compile_context& context) const
        {
            (void)context.resolve_id<givm::support_view>("MissingDeclaration");
            return {};
        }
    };

    struct selectable_handler_source
    {
        using definition_category = givm::support_view;
        static constexpr bool is_dynamic = true;

        struct definition_type{};

        std::string_view source_name;
        bool enabled;

        constexpr std::string_view name() const noexcept
        {
            return source_name;
        }

        constexpr definition_type compile(givm::definition_compile_context&) const noexcept
        {
            return {};
        }

        template<class TView, class TEvent>
        constexpr bool can_handle() const noexcept
        {
            return enabled && std::is_same_v<TView, givm::support_view>
                && std::is_same_v<TEvent, givm::test_event>;
        }

        template<class TQuery>
        constexpr bool can_query() const noexcept
        {
            return false;
        }

        static givm::program_entry handle(
            const definition_type&,
            const givm::support_view&,
            givm::test_event&,
            givm::handle_context&)
        {
            return {};
        }
    };

    struct static_handler_source
    {
        using definition_category = givm::support_view;

        struct definition_type{};

        std::string_view source_name;
        std::uint32_t* capability_checks;

        std::string_view name() const noexcept { return source_name; }

        definition_type compile(givm::definition_compile_context&) const noexcept { return {}; }

        template<class TView, class TEvent>
        bool can_handle() const noexcept
        {
            ++*capability_checks;
            return false;
        }

        static givm::program_entry handle(const definition_type&, const givm::support_view&,
            givm::test_event&, givm::handle_context&)
        {
            return {};
        }
    };

    struct explicitly_static_handler_source : static_handler_source
    {
        static constexpr bool is_dynamic = false;
    };

    struct missing_dynamic_handler_source : selectable_handler_source
    {
        template<class TView, class TEvent>
        constexpr bool can_handle() const noexcept
        {
            return std::is_same_v<TEvent, givm::damage_effect>;
        }
    };

    struct zero_random
    {
        std::uint32_t operator()() const noexcept
        {
            return 0;
        }
    };
}

TEST_CASE("definition compile context resolves declared dependencies", "[source_view]")
{
    dependency_observation observation;
    const dependent_card_source card{ .observation = &observation };
    const tagged_support_source alpha{
        .source_name = "AlphaSupport",
        .source_tags = { "selected", "ordinary" },
        .tag_count = 2
    };
    const tagged_support_source beta{
        .source_name = "BetaSupport",
        .source_tags = { "selected", "excluded" },
        .tag_count = 2
    };

    auto source_library = givm_test::make_source_library();
    REQUIRE(source_library.add(card, alpha, beta));
    const auto program = std::tuple{ givm::draw_cards{ .count = 1 }, givm::test_command{}, givm::end_game{ givm::game_result::both_loss } };
    const auto [library, id_map] = compile(source_library, program, program, givm::compile_mode::normal);
    const auto card_id = id_map.get_id<givm::card_definition>(card.name());

    givm::table table{};
    load_deck(table, library, givm::linked_deck{ .cards = { card_id } }, {});
    zero_random random_source;
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(executor.step(library, table, random_source) == givm::execution_state::finished);
    REQUIRE(table[givm::player_id{ 0 }].hand_card_count() == 1);
    REQUIRE(observation.handled);
    CHECK(observation.alpha_support.value() == id_map.get_id<givm::support_view>("AlphaSupport").value());
    CHECK(observation.beta_support.value() == id_map.get_id<givm::support_view>("BetaSupport").value());
    CHECK(observation.chosen_tag.value() == id_map.get_tag_id("chosen").value());

    REQUIRE(observation.filtered_supports.size() == 1);
    CHECK(
        observation.filtered_supports.front().value()
        == id_map.get_id<givm::support_view>("AlphaSupport").value()
    );
}

TEST_CASE("definition compile context rejects undeclared dependency queries", "[source_view]")
{
    const undeclared_dependency_source source;
    auto source_library = givm_test::make_source_library();
    REQUIRE(source_library.add(source));
    const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
    REQUIRE_THROWS_AS(compile(source_library, program, program, givm::compile_mode::normal), std::invalid_argument);
}

TEST_CASE("compiled definitions expose only enabled source handlers", "[source_view]")
{
    const selectable_handler_source enabled{ "Enabled", true };
    const selectable_handler_source disabled{ "Disabled", false };

    auto source_library = givm_test::make_source_library();
    REQUIRE(source_library.add(enabled, disabled));
    const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
    const auto [library, id_map] = compile(source_library, program, program, givm::compile_mode::normal);

    CHECK(library[id_map.get_id<givm::support_view>(enabled.name())].can_handle<givm::test_event, givm::support_view>());
    CHECK_FALSE(
        library[id_map.get_id<givm::support_view>(enabled.name())].can_handle<givm::damage_effect, givm::support_view>()
    );
    CHECK_FALSE(
        library[id_map.get_id<givm::support_view>(disabled.name())].can_handle<givm::test_event, givm::support_view>()
    );
}

TEST_CASE("static handler availability depends on the implementation alone", "[source_view]")
{
    std::uint32_t capability_checks = 0;
    const static_handler_source implicit_source{ "ImplicitStatic", &capability_checks };
    const explicitly_static_handler_source explicit_source{ { "ExplicitStatic", &capability_checks } };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(implicit_source, explicit_source));
    const auto [library, ids] = compile(sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal);

    CHECK(library[ids.get_id<givm::support_view>(implicit_source.name())]
        .can_handle<givm::test_event, givm::support_view>());
    CHECK(library[ids.get_id<givm::support_view>(explicit_source.name())]
        .can_handle<givm::test_event, givm::support_view>());
    CHECK_FALSE(library[ids.get_id<givm::support_view>(implicit_source.name())]
        .can_handle<givm::damage_effect, givm::support_view>());
    CHECK(capability_checks == 0);
}

TEST_CASE("dynamic sources cannot enable a missing handler implementation", "[source_view]")
{
    const missing_dynamic_handler_source source{ { "MissingDynamicHandler", true } };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(source));
    REQUIRE_THROWS_AS(compile(sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal),
        std::invalid_argument);
}

TEST_CASE("definition compile context accepts heterogeneous tuples and homogeneous ranges", "[source_view]")
{
    program_observation observation;
    const programmed_card_source card{ &observation };
    const programmed_support_source support{ &observation };

    auto source_library = givm_test::make_source_library();
    REQUIRE(source_library.add(card, support));
    const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
    const auto [library, id_map] = compile(source_library, program, program, givm::compile_mode::normal);

    CHECK(observation.event_compiled);
    CHECK(observation.onpay_compiled);
    CHECK(
        observation.resolved_support.value()
        == id_map.get_id<givm::support_view>(support.name()).value()
    );
    CHECK(observation.first_event_entry_set);
    CHECK(observation.second_event_entry_set);
    CHECK(observation.onpay_entry_set);
}

#ifndef NDEBUG
TEST_CASE("root programs reject commands that consume invocation inputs", "[source_view][program-input][debug]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool runtime_commands = GENERATE(false, true);
    const bool initialization = GENERATE(false, true);
    CAPTURE(mode, runtime_commands, initialization);
    const auto sources = givm_test::make_source_library();
    const auto valid = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
    const auto check = [&](const auto& invalid)
    {
        if(initialization)
            REQUIRE_THROWS_AS(compile(sources, invalid, valid, mode), std::invalid_argument);
        else
            REQUIRE_THROWS_AS(compile(sources, valid, invalid, mode), std::invalid_argument);
    };
    if(runtime_commands)
        check(std::vector<givm::any_command>{ givm::set_active_character{} });
    else
        check(std::tuple{ givm::set_active_character{} });
}
#endif
