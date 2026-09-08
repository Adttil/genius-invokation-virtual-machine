#include <cstdint>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/table.hpp>

#include "table/test_definition_library.hpp"

using namespace givm;

namespace
{
    template<class TCard>
    std::vector<std::uint32_t> status_counts(const TCard& card)
    {
        std::vector<std::uint32_t> result;
        for(const auto status : card.statuses())
        {
            result.push_back(status.state().count);
        }
        return result;
    }
}

TEST_CASE("card_table cleanup preserves every card status chain", "[table]")
{
    const test::named_definition_source<card_definition> alpha{ "Alpha" };
    const test::named_definition_source<card_definition> beta{ "Beta" };
    const test::named_definition_source<card_definition> gamma{ "Gamma" };
    const test::named_definition_source<status_definition> status_source{ "Status" };
    const auto [library, id_map] = test::compile_definitions(alpha, beta, gamma, status_source);
    const auto alpha_id = id_map.get_id<card_definition>("Alpha");
    const auto beta_id = id_map.get_id<card_definition>("Beta");
    const auto gamma_id = id_map.get_id<card_definition>("Gamma");
    const auto status_id = id_map.get_id<status_definition>("Status");

    card_table table{ library };
    const auto player = table[player_id{ 0 }];
    player.add_hand_card(alpha_id, {});
    player.add_hand_card(beta_id, {});
    player.add_deck_card(gamma_id, {});

    const auto erased_status_owner = table[hand_card_id{ .player_id = player_id{ 0 }, .index = 0 }];
    const auto hand_card = table[hand_card_id{ .player_id = player_id{ 0 }, .index = 1 }];
    const auto deck_card = table[deck_card_id{ .player_id = player_id{ 0 }, .index = 0 }];

    erased_status_owner.add(status_id, { .count = 10 });
    erased_status_owner.add(status_id, { .count = 11 });
    hand_card.add(status_id, { .count = 20 });
    hand_card.add(status_id, { .count = 21 });
    deck_card.add(status_id, { .count = 30 });
    deck_card.add(status_id, { .count = 31 });

    while(erased_status_owner.statuses().begin() != erased_status_owner.statuses().end())
    {
        (*erased_status_owner.statuses().begin()).erase();
    }

    table.clean_up();

    const auto cleaned_player = table[player_id{ 0 }];
    auto hand_cards = cleaned_player.hand_cards();
    auto hand_card_iterator = hand_cards.begin();
    const auto cleaned_erased_status_owner = *hand_card_iterator;
    const auto cleaned_hand_card = *++hand_card_iterator;
    const auto cleaned_deck_card = *cleaned_player.deck_cards().begin();

    CHECK(status_counts(cleaned_erased_status_owner).empty());
    CHECK(status_counts(cleaned_hand_card) == std::vector<std::uint32_t>{ 20, 21 });
    CHECK(status_counts(cleaned_deck_card) == std::vector<std::uint32_t>{ 30, 31 });

    for(const auto status : cleaned_hand_card.statuses())
    {
        CHECK(status.card().id() == cleaned_hand_card.id());
    }
    for(const auto status : cleaned_deck_card.statuses())
    {
        CHECK(status.card().id() == cleaned_deck_card.id());
    }
}
