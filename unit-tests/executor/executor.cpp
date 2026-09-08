#include <concepts>
#include <cstdint>
#include <tuple>

#include <catch2/catch_test_macros.hpp>

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

        bool execute(card_table&, execution_context& context, random_fn&) const
        {
            *value = *value * 10 + digit;
            return context.enter_next();
        }
    };

    struct yield_once
    {
        using context_type = void;

        int* execution_count;

        bool execute(card_table&, execution_context& context, random_fn&) const
        {
            ++*execution_count;
            if(context.current_stage() == stage_t{})
            {
                ++context.current_stage();
                return context.yield();
            }
            return context.enter_next();
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

    struct finish_game
    {
        using context_type = void;

        game_result result;

        bool execute(card_table&, execution_context& context, random_fn&) const noexcept
        {
            return context.end_game(result);
        }
    };

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
        execution_context& context,
        random_fn& random
    )
    {
        instruction.execute(table, context, random);
    };
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

    const auto& instruction = library.instruction(target.position());
    STATIC_REQUIRE(observed_instruction<decltype(instruction)>);
    STATIC_REQUIRE(publicly_executable_instruction<decltype(instruction)>);
    REQUIRE(instruction.is<append_digit>());
    CHECK(instruction.as<append_digit>().digit == 1);
    CHECK(instruction.type_index() == instruction_type_index<append_digit>);

    while(target.execute_next(table, random))
    {
    }

    CHECK(value == 123);
    CHECK(library.instruction(target.position()).is<stop_execution>());
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

    const auto initial_position = target.position();
    REQUIRE_FALSE(target.execute_next(table, random));
    CHECK(execution_count == 1);
    CHECK(target.position() == initial_position);
    CHECK(library.instruction(target.position()).is<yield_once>());

    REQUIRE(target.execute_next(table, random));
    CHECK(execution_count == 2);
    CHECK(library.instruction(target.position()).is<stop_execution>());
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

    REQUIRE(target.execute_next(table, random));
    REQUIRE(target.execute_next(table, random));
    REQUIRE(target.execute_next(table, random));
    CHECK(value == 123);

    REQUIRE(library.instruction(target.position()).is<append_digit>());
    REQUIRE(target.execute_next(table, random));
    REQUIRE(target.execute_next(table, random));
    CHECK(value == 12323);
}

TEST_CASE("clear removes the stack and enter_entry resets execution", "[executor][fixed-program]")
{
    definition_source_library sources;
    const auto [library, id_map] = sources.compile(
        std::tuple{ finish_game{ .result = game_result::player_1_win } },
        std::tuple{ stop_execution{} }
    );
    card_table table{ library };
    executor target;
    target.enter_entry(library);
    fixed_random random;

    REQUIRE_FALSE(target.stack().empty());
    auto&& [initial_stage] = target.stack().top<stage_t>();
    CHECK(initial_stage == stage_t{});
    CHECK_FALSE(target.execute_next(table, random));
    REQUIRE(target.status() == game_result::player_1_win);
    REQUIRE_FALSE(target.stack().empty());
    const auto terminal_position = target.position();

    CHECK_FALSE(target.execute_next(table, random));
    CHECK(target.position() == terminal_position);
    CHECK(target.status() == game_result::player_1_win);

    target.clear();
    CHECK(target.stack().empty());
    CHECK(target.position() == terminal_position);
    CHECK(target.status() == game_result::player_1_win);

    target.enter_entry(library);
    REQUIRE_FALSE(target.stack().empty());
    auto&& [restarted_stage] = target.stack().top<stage_t>();
    CHECK(restarted_stage == stage_t{});
    CHECK(target.position() == library.entry());
    CHECK(target.status() == game_result::no_result);
    CHECK(library.instruction(target.position()).is<finish_game>());
}
