#include <cstddef>
#include <cstdint>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/table.hpp>
#include <givm/executor.hpp>

#include "table/test_definition_library.hpp"

using namespace givm;

namespace
{
    std::vector<definition_id<card_definition>> hand_definitions(player_view player)
    {
        std::vector<definition_id<card_definition>> result;
        for(const auto card : player.hand_cards())
        {
            result.push_back(card.definition_id());
        }
        return result;
    }

    std::vector<definition_id<card_definition>> deck_definitions(player_view player)
    {
        std::vector<definition_id<card_definition>> result;
        for(std::size_t index = 0; index < player.deck_card_count(); ++index)
        {
            result.push_back(player.deck_card_definition(index));
        }
        return result;
    }
}

TEST_CASE("table views track execution changes while copies own their state", "[table][public-interface]")
{
    const test::named_definition_source<card_definition> alpha{ "Alpha" };
    const test::named_definition_source<card_definition> beta{ "Beta" };
    const test::named_definition_source<card_definition> gamma{ "Gamma" };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{ draw_cards{ .count = 2 }, start_round{} },
        std::tuple{ end_game{ game_result::both_loss } },
        alpha, beta, gamma
    );
    const auto alpha_id = id_map.get_id<card_definition>(alpha.name());
    const auto beta_id = id_map.get_id<card_definition>(beta.name());
    const auto gamma_id = id_map.get_id<card_definition>(gamma.name());
    card_table table{ game_parameters{ .hand_limit = 2 } };
    table.load_deck(player_id{ 0 }, linked_deck{ .cards = { alpha_id, beta_id, gamma_id } });
    const auto player = table[player_id{ 0 }];
    auto copy = table;

    auto random = []() -> std::uint32_t { return 0; };
    executor executor;
    executor.enter_entry(library);
    REQUIRE(executor.run(library, table, random) == execution_state::finished);
    CHECK(player.hand_card_count() == 2);
    CHECK(player.deck_card_count() == 1);
    CHECK(hand_definitions(player) == std::vector{ gamma_id, beta_id });
    CHECK(deck_definitions(player) == std::vector{ alpha_id });
    CHECK(table.state().round_number == 1);
    CHECK(copy[player_id{ 0 }].hand_card_count() == 0);
    CHECK(deck_definitions(copy[player_id{ 0 }]) == std::vector{ alpha_id, beta_id, gamma_id });
    CHECK(copy.state().round_number == 0);
    CHECK(copy.parameters().hand_limit == 2);

    table.clean_up();
    CHECK(hand_definitions(table[player_id{ 0 }]) == std::vector{ gamma_id, beta_id });
    CHECK(deck_definitions(table[player_id{ 0 }]) == std::vector{ alpha_id });
    CHECK(deck_definitions(copy[player_id{ 0 }]) == std::vector{ alpha_id, beta_id, gamma_id });
}
