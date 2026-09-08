#include <cstdint>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition/source_library.hpp>
#include <givm/executor/instructions/test_command.hpp>
#include <givm/table.hpp>

using namespace givm;

namespace
{
    struct fixed_random
    {
        std::uint32_t operator()() const noexcept
        {
            return 0;
        }
    };

    struct response_observation
    {
        support_id expected_handler{};
        std::vector<int> execution_order;
        bool fixed_context_observed = false;
    };

    struct observe_fixed_response
    {
        using context_type = test_event;

        response_observation* observation;

        bool execute(card_table&, execution_context& context, random_fn&) const
        {
            auto&& [broadcast, activation] = context.stack().top<
                frame<
                    detail::handler_id<test_event>[],
                    stack_count_t,
                    test_event,
                    detail::handler_id<test_event>,
                    stage_t
                >,
                frame<execution_context::return_info, stage_t>
            >();
            auto&& [handlers, cursor, event, current_handler, broadcast_stage] = broadcast;
            auto&& [return_info, activation_stage] = activation;
            (void)event;
            (void)broadcast_stage;
            (void)return_info;

            observation->fixed_context_observed =
                handlers.size() == 2
                && cursor == 1
                && std::holds_alternative<support_id>(current_handler)
                && std::get<support_id>(current_handler) == observation->expected_handler
                && activation_stage == stage_t{};
            observation->execution_order.push_back(1);
            return context.enter_next();
        }
    };

    struct observe_second_response
    {
        using context_type = test_event;

        response_observation* observation;

        bool execute(card_table&, execution_context& context, random_fn&) const
        {
            observation->execution_order.push_back(2);
            return context.enter_next();
        }
    };

    struct fixed_response_source
    {
        using definition_category = support_view;

        struct definition_type
        {
            response_observation* observation;
            program_entry<test_event> entry;
        };

        response_observation* observation;

        constexpr std::string_view name() const noexcept
        {
            return "fixed-response";
        }

        definition_type compile(definition_compile_context& context) const
        {
            return {
                .observation = observation,
                .entry = context.add_program<test_event>(
                    std::tuple{ observe_fixed_response{ observation } }
                )
            };
        }

        static program_entry<test_event> handle(
            const definition_type& data,
            const support_view&,
            test_event&,
            const card_table&,
            random_fn&
        )
        {
            return data.entry;
        }
    };

    struct second_response_source
    {
        using definition_category = support_view;

        struct definition_type
        {
            response_observation* observation;
            program_entry<test_event> entry;
        };

        response_observation* observation;

        constexpr std::string_view name() const noexcept
        {
            return "second-response";
        }

        definition_type compile(definition_compile_context& context) const
        {
            return {
                .observation = observation,
                .entry = context.add_program<test_event>(
                    std::tuple{ observe_second_response{ observation } }
                )
            };
        }

        static program_entry<test_event> handle(
            const definition_type& data,
            const support_view&,
            test_event&,
            const card_table&,
            random_fn&
        )
        {
            return data.entry;
        }
    };

    struct stop_execution
    {
        using context_type = void;

        bool execute(card_table&, execution_context& context, random_fn&) const noexcept
        {
            return context.yield();
        }
    };

    struct terminal_response_source
    {
        using definition_category = support_view;

        struct definition_type{};

        constexpr std::string_view name() const noexcept
        {
            return "terminal-response";
        }

        constexpr definition_type compile(definition_compile_context&) const noexcept
        {
            return {};
        }

        static program_entry<test_event> handle(
            const definition_type&,
            const support_view&,
            test_event&,
            const card_table&,
            random_fn&
        )
        {
            return program_entry<test_event>::player_1_win();
        }
    };
}

TEST_CASE("ordinary broadcasts resume across fixed responses", "[broadcast][fixed-program]")
{
    response_observation observation;
    const fixed_response_source fixed_source{ &observation };
    const second_response_source second_source{ &observation };

    definition_source_library sources;
    REQUIRE(sources.add(fixed_source, second_source));
    const auto [library, id_map] = sources.compile(
        std::tuple{ test_command{} },
        std::tuple{ stop_execution{} }
    );
    card_table table{ library };

    const auto fixed_entity = table[player_id{ 0 }].add(
        id_map.get_id<support_view>(fixed_source.name()),
        { .count = 1 }
    );
    observation.expected_handler = fixed_entity.id();
    table[player_id{ 0 }].add(
        id_map.get_id<support_view>(second_source.name()),
        { .count = 1 }
    );

    executor target;
    target.enter_entry(table.definition_library());
    fixed_random random;
    while(target.execute_next(table, random))
    {
        REQUIRE(target.status() == game_result::no_result);
    }

    CHECK(observation.fixed_context_observed);
    CHECK(observation.execution_order == std::vector{ 1, 2 });
}

TEST_CASE("a fixed response may terminate the game without discarding its stack", "[broadcast][fixed-program]")
{
    const terminal_response_source source;
    definition_source_library sources;
    REQUIRE(sources.add(source));
    const auto [library, id_map] = sources.compile(
        std::tuple{ test_command{} },
        std::tuple{ stop_execution{} }
    );
    card_table table{ library };
    table[player_id{ 0 }].add(
        id_map.get_id<support_view>(source.name()),
        { .count = 1 }
    );

    executor target;
    target.enter_entry(table.definition_library());
    fixed_random random;
    while(
        target.status() == game_result::no_result
        && target.execute_next(table, random)
    )
    {
    }
    REQUIRE(target.status() == game_result::player_1_win);
    const auto terminal_stack_size = target.stack().size();
    REQUIRE(terminal_stack_size != 0);

    CHECK_FALSE(target.execute_next(table, random));
    CHECK(target.status() == game_result::player_1_win);
    CHECK(target.stack().size() == terminal_stack_size);
}
