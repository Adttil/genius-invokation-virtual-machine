#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

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

        static givm::handler_program_entry_t<givm::test_event> handle(
            const definition_type& definition,
            const givm::hand_card_view&,
            givm::test_event&,
            const givm::table&,
            givm::random_fn&
        )
        {
            definition.observation->handled = true;
            definition.observation->alpha_support = definition.alpha_support;
            definition.observation->beta_support = definition.beta_support;
            definition.observation->chosen_tag = definition.chosen_tag;
            definition.observation->filtered_supports = definition.filtered_supports;
            return givm::handler_program_entry_t<givm::test_event>::null();
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
            givm::program_entry<givm::test_event> first_entry;
            givm::program_entry<givm::test_event> second_entry;
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

            const auto first_entry = context.add_program<givm::test_event>(std::tuple{
                givm::draw_cards{ .count = 0 },
                givm::shuffle_deck{ .player = givm::player_id{ 0 } }
            });
            const auto second_entry = context.add_program<givm::test_event>(
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
            givm::program_entry<givm::onpay_context<givm::cost_of_switch>> onpay_entry;
        };

        program_observation* observation;

        constexpr std::string_view name() const noexcept
        {
            return "ProgrammedSupport";
        }

        definition_type compile(givm::definition_compile_context& context) const
        {
            observation->onpay_compiled = true;
            using context_type = givm::onpay_context<givm::cost_of_switch>;
            using instruction_type = givm::any_instruction_for<context_type>;
            const auto entry = context.add_program<context_type>(std::vector{
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
            return enabled;
        }

        static givm::handler_program_entry_t<givm::test_event> handle(
            const definition_type&,
            const givm::support_view&,
            givm::test_event&,
            const givm::table&,
            givm::random_fn&
        )
        {
            return givm::handler_program_entry_t<givm::test_event>::null();
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

    givm::definition_source_library source_library;
    REQUIRE(source_library.add(card, alpha, beta));
    const auto program = std::tuple{ givm::draw_cards{ .count = 1 }, givm::end_game{ givm::game_result::both_loss } };
    const auto [library, id_map] = compile(source_library, program, program);
    const auto card_id = id_map.get_id<givm::card_definition>(card.name());

    givm::table table{};
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .cards = { card_id } });
    zero_random random_source;
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(executor.run(library, table, random_source) == givm::execution_state::finished);
    REQUIRE(table[givm::player_id{ 0 }].hand_card_count() == 1);
    givm::random_fn random{ random_source };
    givm::test_event event;
    const auto card_view = *table[givm::player_id{ 0 }].hand_cards().begin();
    CHECK_FALSE(library[card_view.definition_id()].handle<givm::test_event>(card_view, event, table, random));

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
    givm::definition_source_library source_library;
    REQUIRE(source_library.add(source));
    const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
    REQUIRE_THROWS_AS(compile(source_library, program, program), std::invalid_argument);
}

TEST_CASE("compiled definitions expose only enabled source handlers", "[source_view]")
{
    const selectable_handler_source enabled{ "Enabled", true };
    const selectable_handler_source disabled{ "Disabled", false };

    givm::definition_source_library source_library;
    REQUIRE(source_library.add(enabled, disabled));
    const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
    const auto [library, id_map] = compile(source_library, program, program);

    CHECK(library[id_map.get_id<givm::support_view>(enabled.name())].can_handle<givm::test_event, givm::support_view>());
    CHECK_FALSE(
        library[id_map.get_id<givm::support_view>(enabled.name())].can_handle<givm::damage_effect, givm::support_view>()
    );
    CHECK_FALSE(
        library[id_map.get_id<givm::support_view>(disabled.name())].can_handle<givm::test_event, givm::support_view>()
    );
}

TEST_CASE("definition compile context accepts heterogeneous tuples and homogeneous ranges", "[source_view]")
{
    program_observation observation;
    const programmed_card_source card{ &observation };
    const programmed_support_source support{ &observation };

    givm::definition_source_library source_library;
    REQUIRE(source_library.add(card, support));
    const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
    const auto [library, id_map] = compile(source_library, program, program);

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
