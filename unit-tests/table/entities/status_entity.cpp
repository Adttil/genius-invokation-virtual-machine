#include <cstdint>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/table.hpp>

#include "../test_definition_library.hpp"

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

TEST_CASE("card status ranges preserve order when head middle and tail are erased", "[status_entity]")
{
    const test::named_definition_source<card_definition> card_source{ "Card" };
    const test::named_definition_source<status_definition> status_source{ "Status" };
    const auto [library, id_map] = test::compile_definitions(card_source, status_source);
    const auto card_id = id_map.get_id<card_definition>("Card");
    const auto status_id = id_map.get_id<status_definition>("Status");

    card_table table{ library };
    const auto player = table[player_id{ 0 }];
    const auto card = player.add_hand_card(card_id, {});
    card.add(status_id, { .count = 10 });
    card.add(status_id, { .count = 20 });
    card.add(status_id, { .count = 30 });

    CHECK(status_counts(card) == std::vector<std::uint32_t>{ 10, 20, 30 });

    auto middle = card.statuses().begin();
    ++middle;
    (*middle).erase();
    CHECK(status_counts(card) == std::vector<std::uint32_t>{ 10, 30 });

    (*card.statuses().begin()).erase();
    CHECK(status_counts(card) == std::vector<std::uint32_t>{ 30 });

    (*card.statuses().begin()).erase();
    CHECK(status_counts(card).empty());
}

TEST_CASE("moving a deck card into hand preserves its attached status chain", "[status_entity]")
{
    const test::named_definition_source<card_definition> card_source{ "Card" };
    const test::named_definition_source<status_definition> status_source{ "Status" };
    const auto [library, id_map] = test::compile_definitions(card_source, status_source);
    const auto card_id = id_map.get_id<card_definition>("Card");
    const auto status_id = id_map.get_id<status_definition>("Status");

    card_table table{ library };
    const auto player = table[player_id{ 0 }];
    const auto deck_card = player.add_deck_card(card_id, {});
    deck_card.add(status_id, { .count = 7 });
    deck_card.add(status_id, { .count = 9 });

    const auto moved_data = player.take_top_deck_card();
    const auto hand_card = player.add_hand_card(moved_data);

    CHECK(player.deck_card_count() == 0);
    CHECK(player.hand_card_count() == 1);
    CHECK(status_counts(hand_card) == std::vector<std::uint32_t>{ 7, 9 });
    for(const auto status : hand_card.statuses())
    {
        CHECK(status.card().id() == hand_card.id());
    }
}
