#include "../executor_access.hpp"
#include <concepts>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/executor/instructions/start_round.hpp>
#include <givm/executor/instructions/end_game.hpp>

#include <givm/definition/source_library.hpp>
#include <givm/executor/executor.hpp>
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

    struct append_digit
    {
        using context_type = void;

        int* value;
        int digit;

        execution_state execute(card_table&, detail::execution_context& context, random_fn&) const
        {
            *value = *value * 10 + digit;
            return context.enter_next();
        }
    };

    struct yield_once
    {
        using context_type = void;

        int* execution_count;

        execution_state execute(card_table&, detail::execution_context& context, random_fn&) const
        {
            ++*execution_count;
            if(context.current_stage() == detail::stage_t{})
            {
                ++context.current_stage();
                return context.yield(execution_state::action);
            }
            return context.enter_next();
        }
    };

    struct stop_execution
    {
        using context_type = void;

        execution_state execute(card_table&, detail::execution_context& context, random_fn&) const noexcept
        {
            return context.yield(execution_state::action);
        }
    };

    template<class T>
    concept publicly_accessible_stack = requires(T& target) { target.stack(); };

    template<class T>
    concept observed_instruction = requires(const T& instruction)
    {
        { instruction.template is<append_digit>() } -> std::same_as<bool>;
        instruction.template as<append_digit>();
        { instruction.type_index() } -> std::same_as<const void*>;
    };

    template<class T>
    concept publicly_executable_instruction = requires(
        const T& instruction,
        card_table& table,
        detail::execution_context& context,
        random_fn& random
    )
    {
        instruction.execute(table, context, random);
    };
}

TEST_CASE("notification views carry no borrowed execution data", "[executor][execution-view]")
{
    STATIC_REQUIRE(std::is_empty_v<decltype(std::declval<executor&>().view_in<execution_state::round_started>())>);
    STATIC_REQUIRE(std::is_empty_v<decltype(std::declval<executor&>().view_in<execution_state::action_started>())>);
    STATIC_REQUIRE(std::is_empty_v<decltype(std::declval<executor&>().view_in<execution_state::round_end_declared>())>);
    STATIC_REQUIRE(std::is_empty_v<decltype(std::declval<executor&>().view_in<execution_state::round_ending>())>);
    STATIC_REQUIRE(std::is_empty_v<decltype(std::declval<executor&>().view_in<execution_state::initial_active_characters_selected>())>);
}

TEST_CASE("executor runs fixed instructions in sequence", "[executor][fixed-program]")
{
    int value = 0;
    definition_source_library sources;
    const auto [library, id_map] = sources.compile(
        std::tuple{
            append_digit{ .value = &value, .digit = 1 },
            append_digit{ .value = &value, .digit = 2 },
            append_digit{ .value = &value, .digit = 3 }
        },
        std::tuple{ stop_execution{} }
    );
    card_table table{ library };
    executor target;
    target.enter_entry(library);
    fixed_random random;

    const auto& instruction = detail::executor_access::instruction(library, detail::executor_access::position(target));
    STATIC_REQUIRE(observed_instruction<decltype(instruction)>);
    STATIC_REQUIRE(not publicly_executable_instruction<decltype(instruction)>);
    STATIC_REQUIRE(not publicly_executable_instruction<start_round>);
    STATIC_REQUIRE(not publicly_executable_instruction<end_game>);
    STATIC_REQUIRE(not publicly_accessible_stack<executor>);
    REQUIRE(instruction.is<append_digit>());
    CHECK(instruction.as<append_digit>().digit == 1);
    CHECK(instruction.type_index() == detail::instruction_type_index<append_digit>);

    CHECK(target.run(table, random) == execution_state::action);

    CHECK(value == 123);
    CHECK(detail::executor_access::instruction(library, detail::executor_access::position(target)).is<stop_execution>());
}

TEST_CASE("yield keeps the current fixed instruction and stage", "[executor][fixed-program]")
{
    int execution_count = 0;
    definition_source_library sources;
    const auto [library, id_map] = sources.compile(
        std::tuple{ yield_once{ .execution_count = &execution_count } },
        std::tuple{ stop_execution{} }
    );
    card_table table{ library };
    executor target;
    target.enter_entry(library);
    fixed_random random;

    const auto initial_position = detail::executor_access::position(target);
    REQUIRE_FALSE((detail::executor_access::execute_next(target, table, random) == detail::continue_execution));
    CHECK(execution_count == 1);
    CHECK(detail::executor_access::position(target) == initial_position);
    CHECK(detail::executor_access::instruction(library, detail::executor_access::position(target)).is<yield_once>());

    REQUIRE((detail::executor_access::execute_next(target, table, random) == detail::continue_execution));
    CHECK(execution_count == 2);
    CHECK(detail::executor_access::instruction(library, detail::executor_access::position(target)).is<stop_execution>());
}

TEST_CASE("executor repeats the round segment without exposing its connection", "[executor][fixed-program]")
{
    int value = 0;
    definition_source_library sources;
    const auto [library, id_map] = sources.compile(
        std::tuple{ append_digit{ .value = &value, .digit = 1 } },
        std::tuple{
            append_digit{ .value = &value, .digit = 2 },
            append_digit{ .value = &value, .digit = 3 }
        }
    );
    card_table table{ library };
    executor target;
    target.enter_entry(library);
    fixed_random random;

    REQUIRE((detail::executor_access::execute_next(target, table, random) == detail::continue_execution));
    REQUIRE((detail::executor_access::execute_next(target, table, random) == detail::continue_execution));
    REQUIRE((detail::executor_access::execute_next(target, table, random) == detail::continue_execution));
    CHECK(value == 123);

    REQUIRE(detail::executor_access::instruction(library, detail::executor_access::position(target)).is<append_digit>());
    REQUIRE((detail::executor_access::execute_next(target, table, random) == detail::continue_execution));
    REQUIRE((detail::executor_access::execute_next(target, table, random) == detail::continue_execution));
    CHECK(value == 12323);
}

TEST_CASE("terminal result is a view and entering a game replaces its old stack", "[executor][fixed-program]")
{
    const bool observed = GENERATE(false, true);
    const auto result = GENERATE(game_result::player_0_win, game_result::player_1_win, game_result::both_loss);
    int value = 0;
    definition_source_library sources;
    const auto [library, id_map] = sources.compile(
        std::tuple{
            append_digit{ .value = &value, .digit = 1 },
            end_game{ .result = result },
            append_digit{ .value = &value, .digit = 9 },
            end_game{ .result = game_result::both_loss }
        },
        std::tuple{ stop_execution{} }
    );
    card_table table{ library };
    executor target;
    target.enter_entry(library);
    fixed_random random;
    const auto initial_size = detail::executor_access::stack(target).size();

    REQUIRE((observed ? target.step(table, random) : target.run(table, random)) == execution_state::finished);
    REQUIRE(target.view_in<execution_state::finished>().result() == result);
    CHECK(value == 1);
    REQUIRE(detail::executor_access::stack(target).size() > initial_size);
    const auto [stack_result] = detail::executor_access::stack(target).top<game_result>();
    CHECK(stack_result == result);

    auto copy = target;
    CHECK(copy.view_in<execution_state::finished>().result() == result);
    const auto retained_capacity = detail::executor_access::stack(target).capacity();
    target.enter_entry(library);
    CHECK(detail::executor_access::stack(target).size() == initial_size);
    CHECK(detail::executor_access::stack(target).capacity() == retained_capacity);
    const auto [stage] = detail::executor_access::stack(target).top<detail::stage_t>();
    CHECK(stage == detail::stage_t{});
    REQUIRE(target.run(table, random) == execution_state::finished);
    CHECK(target.view_in<execution_state::finished>().result() == result);
    CHECK(value == 11);
}
