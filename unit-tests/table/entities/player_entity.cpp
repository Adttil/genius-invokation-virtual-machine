#include <array>
#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/table.hpp>

#include "../test_definition_library.hpp"

using namespace givm;

namespace
{
    template<class TPlayer>
    std::vector<size_t> hand_definition_values(const TPlayer& player)
    {
        std::vector<size_t> result;
        for(const auto card : player.hand_cards())
        {
            result.push_back(card.definition().id().value());
        }
        return result;
    }

    template<class TPlayer>
    std::vector<size_t> deck_definition_values(const TPlayer& player)
    {
        std::vector<size_t> result;
        for(size_t index = 0; index < player.deck_card_count(); ++index)
        {
            result.push_back(player.deck_card_definition(index).value());
        }
        return result;
    }
}

TEST_CASE("player_entity preserves logical deck order across insertion and extraction", "[player_entity]")
{
    const test::named_definition_source<card_definition> alpha{ "Alpha" };
    const test::named_definition_source<card_definition> beta{ "Beta" };
    const test::named_definition_source<card_definition> gamma{ "Gamma" };
    const auto [library, id_map] = test::compile_definitions(alpha, beta, gamma);
    const auto alpha_id = id_map.get_id<card_definition>("Alpha");
    const auto beta_id = id_map.get_id<card_definition>("Beta");
    const auto gamma_id = id_map.get_id<card_definition>("Gamma");

    card_table table{ library };
    const auto player = table[player_id{ 0 }];
    player.add_deck_card(alpha_id, {});
    player.add_deck_card(beta_id, {});
    player.insert_deck_card(1, gamma_id, {});

    CHECK(deck_definition_values(player) == std::vector<size_t>{
        alpha_id.value(), gamma_id.value(), beta_id.value()
    });

    const auto top = player.take_top_deck_card();
    CHECK(top.definition_id.value() == beta_id.value());
    CHECK(deck_definition_values(player) == std::vector<size_t>{ alpha_id.value(), gamma_id.value() });

    const std::array<size_t, 2> selected{ 1, 0 };
    const auto taken = player.take_deck_cards(selected);
    REQUIRE(taken.size() == 2);
    CHECK(taken[0].definition_id.value() == gamma_id.value());
    CHECK(taken[1].definition_id.value() == alpha_id.value());
    CHECK(player.deck_card_count() == 0);
}

TEST_CASE("player_entity cleanup preserves deck order", "[player_entity]")
{
    const test::named_definition_source<card_definition> alpha{ "Alpha" };
    const test::named_definition_source<card_definition> beta{ "Beta" };
    const test::named_definition_source<card_definition> gamma{ "Gamma" };
    const test::named_definition_source<card_definition> delta{ "Delta" };
    const auto [library, id_map] = test::compile_definitions(alpha, beta, gamma, delta);
    const auto alpha_id = id_map.get_id<card_definition>("Alpha");
    const auto beta_id = id_map.get_id<card_definition>("Beta");
    const auto gamma_id = id_map.get_id<card_definition>("Gamma");
    const auto delta_id = id_map.get_id<card_definition>("Delta");

    card_table table{ library };
    const auto player = table[player_id{ 0 }];
    player.add_deck_card(alpha_id, {});
    player.add_deck_card(beta_id, {});
    player.add_deck_card(gamma_id, {});
    player.add_deck_card(delta_id, {});
    const deck_card_id erased{ .player_id = player_id{ 0 }, .index = 1 };
    table[erased].erase();
    CHECK_FALSE(table[erased]);

    const std::vector expected{ alpha_id.value(), gamma_id.value(), delta_id.value() };
    CHECK(deck_definition_values(player) == expected);

    table.clean_up();

    const auto cleaned_player = table[player_id{ 0 }];
    CHECK(cleaned_player.deck_card_count() == 3);
    CHECK(deck_definition_values(cleaned_player) == expected);
}

TEST_CASE("player_entity cleanup preserves hand order after erased cards are removed", "[player_entity]")
{
    const test::named_definition_source<card_definition> alpha{ "Alpha" };
    const test::named_definition_source<card_definition> beta{ "Beta" };
    const test::named_definition_source<card_definition> gamma{ "Gamma" };
    const auto [library, id_map] = test::compile_definitions(alpha, beta, gamma);
    const auto alpha_id = id_map.get_id<card_definition>("Alpha");
    const auto beta_id = id_map.get_id<card_definition>("Beta");
    const auto gamma_id = id_map.get_id<card_definition>("Gamma");

    card_table table{ library };
    const auto player = table[player_id{ 0 }];
    player.add_hand_card(alpha_id, {});
    player.add_hand_card(beta_id, {});
    player.add_hand_card(gamma_id, {});
    table[hand_card_id{ .player_id = player_id{ 0 }, .index = 1 }].erase();

    CHECK(player.hand_card_count() == 2);
    CHECK(hand_definition_values(player) == std::vector<size_t>{ alpha_id.value(), gamma_id.value() });

    table.clean_up();

    const auto cleaned_player = table[player_id{ 0 }];
    CHECK(cleaned_player.hand_card_count() == 2);
    CHECK(hand_definition_values(cleaned_player) == std::vector<size_t>{
        alpha_id.value(), gamma_id.value()
    });
}
