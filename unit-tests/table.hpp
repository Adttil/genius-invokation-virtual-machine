#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/table.hpp>
#include <givm/executor.hpp>

#include "table/test_definition_library.hpp"

namespace givm_test::root::table
{
constexpr std::array<std::size_t, 2> draw_positions_2{ 0, 1 };

namespace
{
    void check_view_tables(const givm::table& table)
    {
        const auto check = [&](const auto entity)
        {
            CHECK(&entity.table() == &table);
            CHECK(&table[entity.id()].table() == &table);
        };
        for(const auto player : table.players())
        {
            check(player);
            for(const auto card : player.hand_cards())
            {
                check(card);
                for(const auto status : card.statuses()) check(status);
            }
            for(const auto card : player.deck_cards())
            {
                check(card);
                for(const auto status : card.statuses()) check(status);
            }
            for(const auto support : player.supports()) check(support);
            for(const auto summon : player.summons()) check(summon);
            for(const auto status : player.combat_statuses()) check(status);
            for(const auto character : player.characters())
            {
                check(character);
                for(const auto skill : character.skills()) check(skill);
                for(const auto attachment : character.attachments()) check(attachment);
            }
        }
    }

    std::vector<givm::definition_id<givm::card_definition>> hand_definitions(givm::player_view player)
    {
        std::vector<givm::definition_id<givm::card_definition>> result;
        for(const auto card : player.hand_cards())
        {
            result.push_back(card.definition_id());
        }
        return result;
    }

    std::vector<givm::definition_id<givm::card_definition>> deck_definitions(givm::player_view player)
    {
        std::vector<givm::definition_id<givm::card_definition>> result;
        for(std::size_t index = 0; index < player.deck_card_count(); ++index)
        {
            result.push_back(player.deck_card_definition(index));
        }
        return result;
    }
}

TEST_CASE("table views track execution changes while copies own their state", "[table][public-interface]")
{
    const givm::test::named_definition_source<givm::card_definition> alpha{ "Alpha" };
    const givm::test::named_definition_source<givm::card_definition> beta{ "Beta" };
    const givm::test::named_definition_source<givm::card_definition> gamma{ "Gamma" };
    const auto character = givm::test::with_passive_skill(
        givm::test::named_definition_source<givm::character_view>{ "Character" });
    const auto [library, id_map] = givm::test::compile_definitions_with_program(
        givm::compile_mode::normal,
        std::tuple{
            givm::draw_cards{ .position = 0, .count = 2 },
            givm::draw_cards{ .player = givm::relative_player::opponent, .position = 0, .count = 2 },
            givm::start_round{}
        },
        std::tuple{ givm::end_game{ givm::game_result::both_loss } },
        alpha, beta, gamma, character
    );
    const auto alpha_id = id_map.get_id<givm::card_definition>(alpha.name());
    const auto beta_id = id_map.get_id<givm::card_definition>(beta.name());
    const auto gamma_id = id_map.get_id<givm::card_definition>(gamma.name());
    const auto character_id = id_map.get_id<givm::character_view>(character.name());
    givm::table table{ { .max_rounds = 3, .self_player = givm::player_id{ 0 } }, { .hand_limit = 2 }, { .hand_limit = 1 } };
    const givm::linked_deck deck{
        .cards = { alpha_id, beta_id, gamma_id }, .characters = { character_id }
    };
    load_deck(table, library, deck, deck);
    const auto player = table[givm::player_id{ 0 }];
    auto copy = table;
    check_view_tables(table);
    check_view_tables(copy);

    auto random = []() -> std::uint32_t { return 0; };
    givm_test::executor_driver executor;
    executor.start(library, table);
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::finished);
    CHECK(player.hand_card_count() == 2);
    CHECK(player.deck_card_count() == 1);
    CHECK(hand_definitions(player) == std::vector{ gamma_id, beta_id });
    CHECK(deck_definitions(player) == std::vector{ alpha_id });
    CHECK(table.state().round_number == 1);
    CHECK(copy[givm::player_id{ 0 }].hand_card_count() == 0);
    CHECK(deck_definitions(copy[givm::player_id{ 0 }]) == std::vector{ alpha_id, beta_id, gamma_id });
    CHECK(copy.state().round_number == 0);
    CHECK(copy[givm::player_id{ 0 }].state().hand_limit == 2);
    CHECK(copy[givm::player_id{ 1 }].state().hand_limit == 1);
    CHECK(table[givm::player_id{ 1 }].hand_card_count() == 1);
    CHECK(table[givm::player_id{ 1 }].deck_card_count() == 1);
    CHECK(table.state().max_rounds == 3);
    check_view_tables(table);
    check_view_tables(copy);

    table.clean_up();
    CHECK(hand_definitions(table[givm::player_id{ 0 }]) == std::vector{ gamma_id, beta_id });
    CHECK(deck_definitions(table[givm::player_id{ 0 }]) == std::vector{ alpha_id });
    CHECK(deck_definitions(copy[givm::player_id{ 0 }]) == std::vector{ alpha_id, beta_id, gamma_id });
    check_view_tables(table);
    check_view_tables(copy);
}
}
