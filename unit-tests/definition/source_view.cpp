#include <array>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition/source_library.hpp>
#include <givm/executor/events.hpp>
#include <givm/executor/executor.hpp>
#include <givm/table.hpp>

using namespace givm;

namespace
{
    struct dependency_observation
    {
        bool handled = false;
        definition_id<support_view> alpha_support;
        definition_id<support_view> beta_support;
        tag_id chosen_tag;
        std::vector<definition_id<support_view>> filtered_supports;
    };

    struct tagged_support_source
    {
        using definition_category = support_view;

        struct definition_type{};

        std::string_view source_name;
        std::array<std::string_view, 2> source_tags{};
        size_t tag_count = 0;

        constexpr std::string_view name() const noexcept
        {
            return source_name;
        }

        constexpr std::span<const std::string_view> tags() const noexcept
        {
            return { source_tags.data(), tag_count };
        }

        constexpr definition_type compile(definition_compile_context&) const noexcept
        {
            return {};
        }
    };

    struct dependent_card_source
    {
        using definition_category = card_definition;

        struct definition_type
        {
            dependency_observation* observation;
            definition_id<support_view> alpha_support;
            definition_id<support_view> beta_support;
            tag_id chosen_tag;
            std::vector<definition_id<support_view>> filtered_supports;
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

        definition_type compile(definition_compile_context& context) const
        {
            return {
                .observation = observation,
                .alpha_support = context.resolve_id<support_view>("AlphaSupport"),
                .beta_support = context.resolve_id<support_view>("BetaSupport"),
                .chosen_tag = context.resolve_tag("chosen"),
                .filtered_supports =
                    context.resolve_ids_by_tag<support_view>("selected & !excluded")
            };
        }

        static handler_program_entry_t<test_event> handle(
            const definition_type& definition,
            const hand_card_view&,
            test_event&,
            const card_table&,
            random_fn&
        )
        {
            definition.observation->handled = true;
            definition.observation->alpha_support = definition.alpha_support;
            definition.observation->beta_support = definition.beta_support;
            definition.observation->chosen_tag = definition.chosen_tag;
            definition.observation->filtered_supports = definition.filtered_supports;
            return handler_program_entry_t<test_event>::null();
        }
    };

    struct program_observation
    {
        std::size_t event_compile_calls = 0;
        std::size_t onpay_compile_calls = 0;
        definition_id<support_view> resolved_support;
        bool first_event_entry_set = false;
        bool second_event_entry_set = false;
        bool onpay_entry_set = false;
    };

    struct event_program_instruction
    {
        using context_type = test_event;

        definition_id<support_view> resolved_support;

        bool execute(card_table&, execution_context&, random_fn&) const noexcept
        {
            return resolved_support.is_valid();
        }
    };

    struct onpay_program_instruction
    {
        using context_type = onpay_context<cost_of_switch>;

        bool execute(card_table&, execution_context&, random_fn&) const noexcept
        {
            return true;
        }
    };

    struct context_free_program_instruction
    {
        using context_type = void;

        bool execute(card_table&, execution_context&, random_fn&) const noexcept
        {
            return true;
        }
    };

    struct programmed_card_source
    {
        using definition_category = card_definition;

        struct definition_type
        {
            program_observation* observation;
            definition_id<support_view> support;
            program_entry<test_event> first_entry;
            program_entry<test_event> second_entry;
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

        definition_type compile(definition_compile_context& context) const
        {
            ++observation->event_compile_calls;
            const auto support = context.resolve_id<support_view>("ProgrammedSupport");
            observation->resolved_support = support;

            const auto first_entry = context.add_program<test_event>(std::tuple{
                event_program_instruction{ support },
                context_free_program_instruction{}
            });
            const auto second_entry = context.add_program<test_event>(
                std::vector{ event_program_instruction{ support } }
            );
            return {
                .observation = observation,
                .support = support,
                .first_entry = first_entry,
                .second_entry = second_entry
            };
        }

        static handler_program_entry_t<test_event> handle(
            const definition_type& definition,
            const hand_card_view&,
            test_event&,
            const card_table&,
            random_fn&
        )
        {
            definition.observation->first_event_entry_set = bool{ definition.first_entry };
            definition.observation->second_event_entry_set = bool{ definition.second_entry };
            return handler_program_entry_t<test_event>::null();
        }
    };

    struct programmed_support_source
    {
        using definition_category = support_view;

        struct definition_type
        {
            program_observation* observation;
            program_entry<onpay_context<cost_of_switch>> onpay_entry;
        };

        program_observation* observation;

        constexpr std::string_view name() const noexcept
        {
            return "ProgrammedSupport";
        }

        definition_type compile(definition_compile_context& context) const
        {
            ++observation->onpay_compile_calls;
            using context_type = onpay_context<cost_of_switch>;
            using instruction_type = any_instruction_for<context_type>;
            const auto entry = context.add_program<context_type>(std::vector{
                instruction_type{ onpay_program_instruction{} },
                instruction_type{ onpay_program_instruction{} }
            });
            return { .observation = observation, .onpay_entry = entry };
        }

        static handler_program_entry_t<test_event> handle(
            const definition_type& definition,
            const support_view&,
            test_event&,
            const card_table&,
            random_fn&
        )
        {
            definition.observation->onpay_entry_set = bool{ definition.onpay_entry };
            return handler_program_entry_t<test_event>::null();
        }
    };

    struct undeclared_dependency_source
    {
        using definition_category = card_definition;

        struct definition_type{};

        constexpr std::string_view name() const noexcept
        {
            return "UndeclaredDependency";
        }

        definition_type compile(definition_compile_context& context) const
        {
            (void)context.resolve_id<support_view>("MissingDeclaration");
            return {};
        }
    };

    struct selectable_handler_source
    {
        using definition_category = support_view;

        struct definition_type{};

        std::string_view source_name;
        bool enabled;

        constexpr std::string_view name() const noexcept
        {
            return source_name;
        }

        constexpr definition_type compile(definition_compile_context&) const noexcept
        {
            return {};
        }

        template<class TView, class TEvent>
        constexpr bool can_handle() const noexcept
        {
            return enabled;
        }

        static handler_program_entry_t<test_event> handle(
            const definition_type&,
            const support_view&,
            test_event&,
            const card_table&,
            random_fn&
        )
        {
            return handler_program_entry_t<test_event>::null();
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

    definition_source_library source_library;
    REQUIRE(source_library.add(card, alpha, beta));
    const auto program = std::tuple{ context_free_program_instruction{} };
    const auto [library, id_map] = source_library.compile(program, program);
    const auto card_id = id_map.get_id<card_definition>(card.name());

    card_table table{ library };
    const auto card_entity = table[player_id{ 0 }].add_hand_card(card_id, {});
    zero_random random_source;
    random_fn random{ random_source };
    test_event event;
    const hand_card_view card_view = card_entity;
    CHECK_FALSE(card_view.definition().handle<test_event>(card_view, event, table, random));

    REQUIRE(observation.handled);
    CHECK(observation.alpha_support.value() == id_map.get_id<support_view>("AlphaSupport").value());
    CHECK(observation.beta_support.value() == id_map.get_id<support_view>("BetaSupport").value());
    CHECK(observation.chosen_tag.value() == id_map.get_tag_id("chosen").value());

    REQUIRE(observation.filtered_supports.size() == 1);
    CHECK(
        observation.filtered_supports.front().value()
        == id_map.get_id<support_view>("AlphaSupport").value()
    );
}

TEST_CASE("definition compile context rejects undeclared dependency queries", "[source_view]")
{
    const undeclared_dependency_source source;
    definition_source_library source_library;
    REQUIRE(source_library.add(source));
    const auto program = std::tuple{ context_free_program_instruction{} };
    REQUIRE_THROWS_AS(source_library.compile(program, program), std::invalid_argument);
}

TEST_CASE("compiled definitions expose only enabled source handlers", "[source_view]")
{
    const selectable_handler_source enabled{ "Enabled", true };
    const selectable_handler_source disabled{ "Disabled", false };

    definition_source_library source_library;
    REQUIRE(source_library.add(enabled, disabled));
    const auto program = std::tuple{ context_free_program_instruction{} };
    const auto [library, id_map] = source_library.compile(program, program);

    CHECK(library[id_map.get_id<support_view>(enabled.name())].can_handle<test_event, support_view>());
    CHECK_FALSE(
        library[id_map.get_id<support_view>(enabled.name())].can_handle<damage_effect, support_view>()
    );
    CHECK_FALSE(
        library[id_map.get_id<support_view>(disabled.name())].can_handle<test_event, support_view>()
    );
}

TEST_CASE("definition compile context accepts heterogeneous tuples and homogeneous ranges", "[source_view]")
{
    program_observation observation;
    const programmed_card_source card{ &observation };
    const programmed_support_source support{ &observation };

    definition_source_library source_library;
    REQUIRE(source_library.add(card, support));
    const auto program = std::tuple{ context_free_program_instruction{} };
    const auto [library, id_map] = source_library.compile(program, program);

    CHECK(observation.event_compile_calls == 1);
    CHECK(observation.onpay_compile_calls == 1);
    CHECK(
        observation.resolved_support.value()
        == id_map.get_id<support_view>(support.name()).value()
    );
    card_table table{ library };
    const auto card_entity = table[player_id{ 0 }].add_hand_card(
        id_map.get_id<card_definition>(card.name()),
        {}
    );
    const auto support_entity = table[player_id{ 0 }].add(
        id_map.get_id<support_view>(support.name()),
        { .count = 1 }
    );

    zero_random random_source;
    random_fn random{ random_source };
    test_event event;
    const hand_card_view card_view = card_entity;
    const support_view support_view_value = support_entity;
    CHECK_FALSE(card_view.definition().handle<test_event>(card_view, event, table, random));
    CHECK_FALSE(support_view_value.definition().handle<test_event>(
        support_view_value,
        event,
        table,
        random
    ));

    CHECK(observation.first_event_entry_set);
    CHECK(observation.second_event_entry_set);
    CHECK(observation.onpay_entry_set);
}
