#include "../test_source_library.hpp"

#include <cstdint>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition.hpp>
#include <givm/executor.hpp>
#include <givm/table.hpp>

TEST_CASE("initialization and round programs accept tuple-like and range forms", "[executor][library]")
{
    const auto initialization = std::tuple{
        givm::start_round{ .max_rounds = 2 },
        givm::draw_cards{ .count = 0 }
    };
    const std::vector round{ givm::end_game{ givm::game_result::player_0_win } };

    auto sources = givm_test::make_source_library();
    const auto [library, id_map] = compile(sources, initialization, round, givm::compile_mode::normal);
    auto random = []() -> std::uint32_t { return 0; };
    givm::table table;
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(table.state().round_number == 1);
    CHECK(executor.view_in<givm::execution_state::finished>().result() == givm::game_result::player_0_win);

    const givm::definition_selection selection{};
    const auto [selected_library, selected_id_map] = compile(sources, selection, initialization, round, givm::compile_mode::normal);
    givm::table selected_table;
    executor.enter_entry(selected_library);
    REQUIRE(executor.step(selected_library, selected_table, random) == givm::execution_state::finished);
    CHECK(selected_table.state().round_number == 1);
    CHECK(executor.view_in<givm::execution_state::finished>().result() == givm::game_result::player_0_win);
}
