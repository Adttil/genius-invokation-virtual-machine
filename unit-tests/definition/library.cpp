#include <cstdint>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition.hpp>
#include <givm/executor.hpp>
#include <givm/table.hpp>

using namespace givm;

TEST_CASE("initialization and round programs accept tuple-like and range forms", "[definition][library]")
{
    const auto initialization = std::tuple{
        start_round{ .max_rounds = 2 },
        draw_cards{ .count = 0 }
    };
    const std::vector round{ end_game{ game_result::player_0_win } };

    definition_source_library sources;
    const auto [library, id_map] = sources.compile(initialization, round);
    auto random = []() -> std::uint32_t { return 0; };
    card_table table;
    executor executor;
    executor.enter_entry(library);
    REQUIRE(executor.run(library, table, random) == execution_state::finished);
    CHECK(table.state().round_number == 1);
    CHECK(executor.view_in<execution_state::finished>().result() == game_result::player_0_win);

    const definition_selection selection{};
    const auto [selected_library, selected_id_map] = sources.compile(selection, initialization, round);
    card_table selected_table;
    executor.enter_entry(selected_library);
    REQUIRE(executor.run(selected_library, selected_table, random) == execution_state::finished);
    CHECK(selected_table.state().round_number == 1);
    CHECK(executor.view_in<execution_state::finished>().result() == game_result::player_0_win);
}
