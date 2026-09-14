#include <cstdint>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

using namespace givm;

namespace
{
    struct zero_random
    {
        std::uint32_t operator()() const noexcept { return 0; }
    };
}

TEST_CASE("executor repeats the round program and reports round boundaries", "[executor]")
{
    definition_source_library sources;
    const auto [library, ids] = sources.compile(
        std::tuple{ start_round{ .max_rounds = 2 } },
        std::tuple{ start_round{ .max_rounds = 2 } }
    );
    card_table table;
    executor target;
    target.enter_entry(library);
    zero_random random;

    for(std::uint32_t round = 1; round <= 3; ++round)
    {
        REQUIRE(target.step(library, table, random) == execution_state::round_started);
        CHECK(table.state().round_number == round);
    }
    REQUIRE(target.step(library, table, random) == execution_state::finished);
    CHECK(target.view_in<execution_state::finished>().result() == game_result::both_loss);
}

TEST_CASE("terminal results survive copies and entering another game replaces the result", "[executor]")
{
    const bool observed = GENERATE(false, true);
    const auto result = GENERATE(game_result::player_0_win, game_result::player_1_win, game_result::both_loss);
    definition_source_library sources;
    const auto [library, ids] = sources.compile(
        std::tuple{
            end_game{ .result = result },
            start_round{},
            end_game{ .result = game_result::both_loss }
        },
        std::tuple{}
    );
    const auto second = sources.compile(
        std::tuple{ end_game{ .result = game_result::player_1_win } }, std::tuple{}
    );
    card_table table;
    executor target;
    target.enter_entry(library);
    zero_random random;

    REQUIRE((observed ? target.step(library, table, random) : target.run(library, table, random))
        == execution_state::finished);
    CHECK(target.view_in<execution_state::finished>().result() == result);
    CHECK(table.state().round_number == 0);
    auto copy = target;
    target.enter_entry(second.library);
    REQUIRE((observed ? target.step(second.library, table, random) : target.run(second.library, table, random))
        == execution_state::finished);
    CHECK(target.view_in<execution_state::finished>().result() == game_result::player_1_win);
    CHECK(copy.view_in<execution_state::finished>().result() == result);
}

TEST_CASE("executor uses the explicitly supplied library with an independent table", "[executor][definition-library]")
{
    const bool observed = GENERATE(false, true);
    definition_source_library sources;
    const auto first = sources.compile(
        std::tuple{ end_game{ .result = game_result::player_0_win } }, std::tuple{}
    );
    const auto second = sources.compile(
        std::tuple{ end_game{ .result = game_result::player_1_win } }, std::tuple{}
    );
    card_table table;
    executor target;
    zero_random random;

    target.enter_entry(first.library);
    REQUIRE((observed ? target.step(first.library, table, random) : target.run(first.library, table, random))
        == execution_state::finished);
    CHECK(target.view_in<execution_state::finished>().result() == game_result::player_0_win);
    target.enter_entry(second.library);
    REQUIRE((observed ? target.step(second.library, table, random) : target.run(second.library, table, random))
        == execution_state::finished);
    CHECK(target.view_in<execution_state::finished>().result() == game_result::player_1_win);
}
