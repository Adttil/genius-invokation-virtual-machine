#include "../test_source_library.hpp"

#include <cstdint>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

namespace givm_test::executor::executor
{
namespace
{
    struct zero_random
    {
        std::uint32_t operator()() const noexcept { return 0; }
    };
}

TEST_CASE("executor repeats the round program and reports round boundaries", "[executor]")
{
    const bool observed = GENERATE(false, true);
    const bool empty_round = GENERATE(false, true);
    auto sources = givm_test::make_source_library();
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources,
        std::tuple{}, empty_round ? std::vector<givm::any_command>{}
            : std::vector<givm::any_command>{ givm::start_round{} },
        observed ? givm::compile_mode::observed : givm::compile_mode::normal
    ));
    givm::table table{ givm::table_state{ .max_rounds = 2 } };
    givm::executor target;
    target.start(library, table);
    zero_random random;

    if(observed) for(std::uint32_t round = 1; round <= 3; ++round)
    {
        REQUIRE(target.step(library, table, random) == givm::execution_state::round_started);
        CHECK(table.state().round_number == round);
    }
    REQUIRE(target.step(library, table, random) == givm::execution_state::finished);
    CHECK(table.state().round_number == 3);
    CHECK(target.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
}

TEST_CASE("terminal results survive copies and entering another game replaces the result", "[executor]")
{
    const bool observed = GENERATE(false, true);
    const auto result = GENERATE(givm::game_result::player_0_win, givm::game_result::player_1_win, givm::game_result::both_loss);
    auto sources = givm_test::make_source_library();
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources,
        std::tuple{
            givm::end_game{ .result = result },
            givm::start_round{},
            givm::end_game{ .result = givm::game_result::both_loss }
        },
        std::tuple{}, observed ? givm::compile_mode::observed : givm::compile_mode::normal
    ));
    const auto second = givm_test::require_success(compile(sources, givm_test::basic_sources,
        std::tuple{ givm::end_game{ .result = givm::game_result::player_1_win } }, std::tuple{}, observed ? givm::compile_mode::observed : givm::compile_mode::normal
    ));
    givm::table table;
    givm::executor target;
    target.start(library, table);
    zero_random random;

    REQUIRE(target.step(library, table, random)
        == givm::execution_state::finished);
    CHECK(target.view_in<givm::execution_state::finished>().result() == result);
    CHECK(table.state().round_number == 0);
    auto copy = target;
    target.start(second.library, table);
    REQUIRE(target.step(second.library, table, random)
        == givm::execution_state::finished);
    CHECK(target.view_in<givm::execution_state::finished>().result() == givm::game_result::player_1_win);
    CHECK(copy.view_in<givm::execution_state::finished>().result() == result);
}

TEST_CASE("executor uses the explicitly supplied library with an independent table", "[executor][definition-library]")
{
    const bool observed = GENERATE(false, true);
    auto sources = givm_test::make_source_library();
    const auto first = givm_test::require_success(compile(sources, givm_test::basic_sources,
        std::tuple{ givm::end_game{ .result = givm::game_result::player_0_win } }, std::tuple{}, observed ? givm::compile_mode::observed : givm::compile_mode::normal
    ));
    const auto second = givm_test::require_success(compile(sources, givm_test::basic_sources,
        std::tuple{ givm::end_game{ .result = givm::game_result::player_1_win } }, std::tuple{}, observed ? givm::compile_mode::observed : givm::compile_mode::normal
    ));
    givm::table table;
    givm::executor target;
    zero_random random;

    target.start(first.library, table);
    REQUIRE(target.step(first.library, table, random)
        == givm::execution_state::finished);
    CHECK(target.view_in<givm::execution_state::finished>().result() == givm::game_result::player_0_win);
    target.start(second.library, table);
    REQUIRE(target.step(second.library, table, random)
        == givm::execution_state::finished);
    CHECK(target.view_in<givm::execution_state::finished>().result() == givm::game_result::player_1_win);
}
}
