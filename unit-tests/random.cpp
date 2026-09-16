#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/givm.hpp>

#include "table/test_definition_library.hpp"

namespace
{
    struct random_tape
    {
        std::vector<std::uint32_t> values;
        std::size_t consumed = 0;

        std::uint32_t operator()()
        {
            const auto value = values.at(consumed);
            ++consumed;
            return value;
        }
    };

    std::vector<givm::definition_id<givm::card_definition>> deck_definitions(givm::player_view player)
    {
        std::vector<givm::definition_id<givm::card_definition>> result;
        for(const auto card : player.deck_cards())
        {
            result.push_back(card.definition_id());
        }
        return result;
    }
}

TEST_CASE("shuffle maps the two halves of a random value to the two card positions", "[random][deck]")
{
    const givm::test::named_definition_source<givm::card_definition> alpha{ "Alpha" };
    const givm::test::named_definition_source<givm::card_definition> beta{ "Beta" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        givm::compile_mode::normal,
        std::tuple{ givm::shuffle_deck{ givm::player_id{ 0 } }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, alpha, beta
    );
    const auto a = ids.get_id<givm::card_definition>(alpha.name());
    const auto b = ids.get_id<givm::card_definition>(beta.name());

    for(const auto [value, changes_order] : std::array{
        std::pair{ 0u, true }, std::pair{ 0x7fffffffu, true },
        std::pair{ 0x80000000u, false }, std::pair{ 0xffffffffu, false }
    })
    {
        INFO(value);
        givm::table table;
        table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .cards = { a, b } });
        givm::executor execution;
        execution.enter_entry(library);
        random_tape random{ { value } };
        REQUIRE(execution.step(library, table, random) == givm::execution_state::finished);
        CHECK(random.consumed == 1);
        CHECK(deck_definitions(table[givm::player_id{ 0 }])
            == (changes_order ? std::vector{ b, a } : std::vector{ a, b }));
    }

    for(const auto cards : { std::vector<givm::definition_id<givm::card_definition>>{}, std::vector{ a } })
    {
        givm::table table;
        table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .cards = cards });
        givm::executor execution;
        execution.enter_entry(library);
        random_tape random;
        REQUIRE(execution.step(library, table, random) == givm::execution_state::finished);
        CHECK(random.consumed == 0);
        CHECK(deck_definitions(table[givm::player_id{ 0 }]) == cards);
    }
}

TEST_CASE("initial replacements assign random values by player and selected card order", "[random][selection]")
{
    const givm::test::named_definition_source<givm::card_definition> alpha{ "Alpha" };
    const givm::test::named_definition_source<givm::card_definition> beta{ "Beta" };
    const givm::test::named_definition_source<givm::card_definition> gamma{ "Gamma" };
    const givm::test::named_definition_source<givm::card_definition> delta{ "Delta" };
    const givm::test::named_definition_source<givm::card_definition> epsilon{ "Epsilon" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        givm::compile_mode::normal,
        std::tuple{
            givm::draw_cards{ .count = 3 },
            givm::draw_cards{ .count = 3, .player = givm::relative_player::other },
            givm::replace_cards_both{}, givm::end_game{ givm::game_result::both_loss }
        },
        std::tuple{}, alpha, beta, gamma, delta, epsilon
    );
    const auto a = ids.get_id<givm::card_definition>(alpha.name());
    const auto b = ids.get_id<givm::card_definition>(beta.name());
    const auto c = ids.get_id<givm::card_definition>(gamma.name());
    const auto d = ids.get_id<givm::card_definition>(delta.name());
    const auto e = ids.get_id<givm::card_definition>(epsilon.name());
    givm::table initial_table;
    for(const givm::player_id player : { givm::player_id{ 0 }, givm::player_id{ 1 } })
    {
        initial_table.load_deck(player, givm::linked_deck{ .cards = { a, b, c, d, e } });
    }
    givm::executor initial_execution;
    initial_execution.enter_entry(library);
    random_tape random{ { 0u, 0xaaaaaaaau, 0xffffffffu, 0xffffffffu, 0x55555555u, 0u } };
    REQUIRE(initial_execution.step(library, initial_table, random) == givm::execution_state::initial_card_selection);
    REQUIRE(random.consumed == 6);

    for(const givm::player_id first_player : { givm::player_id{ 0 }, givm::player_id{ 1 } })
    {
        auto table = initial_table;
        auto execution = initial_execution;
        random_tape later_random;
        const auto selected = std::bitset<givm::selection_capacity>{ 1u << 2 };
        execution.view_in<givm::execution_state::initial_card_selection>().select(first_player, selected);
        REQUIRE(execution.step(library, table, later_random) == givm::execution_state::card_selection);
        REQUIRE(execution.view_in<givm::execution_state::card_selection>().player() == other_player(first_player));
        execution.view_in<givm::execution_state::card_selection>().select(selected);
        REQUIRE(execution.step(library, table, later_random) == givm::execution_state::finished);
        CHECK(later_random.consumed == 0);
        CHECK(deck_definitions(table[givm::player_id{ 0 }]) == std::vector{ c, a });
        CHECK(deck_definitions(table[givm::player_id{ 1 }]) == std::vector{ a, c });
        for(const givm::player_id player : { givm::player_id{ 0 }, givm::player_id{ 1 } })
        {
            std::vector<givm::definition_id<givm::card_definition>> hand;
            for(const auto card : table[player].hand_cards())
            {
                hand.push_back(card.definition_id());
            }
            CHECK(hand == std::vector{ e, d, b });
        }
    }
}

TEST_CASE("replacements fill a blacklist shortfall in deck order and preserve the remaining cards", "[random][selection][replace_cards]")
{
    const givm::test::named_definition_source<givm::card_definition> alpha{ "Alpha" };
    const givm::test::named_definition_source<givm::card_definition> beta{ "Beta" };
    const givm::test::named_definition_source<givm::card_definition> gamma{ "Gamma" };
    const givm::test::named_definition_source<givm::card_definition> delta{ "Delta" };
    const givm::test::named_definition_source<givm::card_definition> epsilon{ "Epsilon" };
    for(const auto mode : { givm::compile_mode::normal, givm::compile_mode::observed })
    {
        const auto [library, ids] = givm::test::compile_definitions_with_program(
            mode,
            std::tuple{
                givm::draw_cards{ .count = 4 },
                givm::replace_cards{ .player = givm::player_id{ 0 } },
                givm::end_game{ givm::game_result::both_loss }
            },
            std::tuple{}, alpha, beta, gamma, delta, epsilon
        );
        const auto a = ids.get_id<givm::card_definition>(alpha.name());
        const auto b = ids.get_id<givm::card_definition>(beta.name());
        const auto c = ids.get_id<givm::card_definition>(gamma.name());
        const auto d = ids.get_id<givm::card_definition>(delta.name());
        const auto e = ids.get_id<givm::card_definition>(epsilon.name());
        givm::table table;
        table.load_deck(givm::player_id{ 0 }, { .cards = { a, d, c, e, c, b, a } });
        givm::executor execution;
        execution.enter_entry(library);
        random_tape random{ { 0u, 0xffffffffu, 0x80000000u } };
        REQUIRE(execution.step(library, table, random) == givm::execution_state::card_selection);
        CHECK(random.consumed == 0);
        execution.view_in<givm::execution_state::card_selection>().select(
            std::bitset<givm::selection_capacity>{ 0b0111 }
        );
        REQUIRE(execution.step(library, table, random) == givm::execution_state::finished);
        CHECK(random.consumed == 3);

        // A, B and C return at the bottom, top and middle. Only D avoids the
        // blacklist, so the top B and C also return to the hand before D.
        std::vector<givm::definition_id<givm::card_definition>> hand;
        for(const auto card : table[givm::player_id{ 0 }].hand_cards())
            hand.push_back(card.definition_id());
        CHECK(hand == std::vector{ e, b, c, d });
        CHECK(deck_definitions(table[givm::player_id{ 0 }]) == std::vector{ a, a, c });
    }
}

TEST_CASE("rerolls continue each player's random dice sequence across partial selections", "[random][dice]")
{
    givm::definition_source_library sources;
    const auto [library, ids] = compile(sources,
        std::tuple{ givm::start_dice_roll_phase{ .count = 6, .reroll_count = { 3, 1 } },
                    givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, givm::compile_mode::normal
    );
    givm::table table;
    givm::executor execution;
    execution.enter_entry(library);
    // Player 0 receives dice 1,2,3,4,5,6,7,0,1,2, then 3,4,... .
    // The high two bits do not represent a die; player 1 starts its own sequence.
    random_tape random{ std::vector<std::uint32_t>(12, 0u) };
    random.values.push_back(0xc0000000u | 1u | (2u << 3) | (3u << 6) | (4u << 9)
        | (5u << 12) | (6u << 15) | (7u << 18) | (1u << 24) | (2u << 27));
    random.values.push_back(3u | (4u << 3));
    random.values.push_back(0x3fffffffu);
    REQUIRE(execution.step(library, table, random) == givm::execution_state::dice_selection);
    REQUIRE(random.consumed == 15);
    CHECK(table[givm::player_id{ 0 }].state().dice[givm::elemental_dice::omni] == 6);
    CHECK(table[givm::player_id{ 1 }].state().dice[givm::elemental_dice::omni] == 6);

    random_tape later_random;
    givm::dice_counts player1_selection;
    player1_selection[givm::elemental_dice::omni] = 2;
    execution.view_in<givm::execution_state::dice_selection>().select(givm::player_id{ 1 }, player1_selection);
    REQUIRE(execution.step(library, table, later_random) == givm::execution_state::dice_selection);
    CHECK(table[givm::player_id{ 1 }].state().dice[givm::elemental_dice::omni] == 4);
    CHECK(table[givm::player_id{ 1 }].state().dice[givm::elemental_dice::dendro] == 2);

    std::array<givm::dice_counts, 3> player0_selections;
    player0_selections[0][givm::elemental_dice::omni] = 4;
    player0_selections[1][givm::elemental_dice::omni] = 2;
    player0_selections[1][givm::elemental_dice::cryo] = 1;
    player0_selections[1][givm::elemental_dice::hydro] = 1;
    player0_selections[2][givm::elemental_dice::omni] = 1;
    player0_selections[2][givm::elemental_dice::pyro] = 1;
    player0_selections[2][givm::elemental_dice::electro] = 1;
    for(std::size_t index = 0; index < 2; ++index)
    {
        execution.view_in<givm::execution_state::dice_selection>().select(givm::player_id{ 0 }, player0_selections[index]);
        REQUIRE(execution.step(library, table, later_random) == givm::execution_state::dice_selection);
    }
    REQUIRE(execution.view_in<givm::execution_state::dice_selection>().remaining(givm::player_id{ 0 }) == 1);
    execution.view_in<givm::execution_state::dice_selection>().select(givm::player_id{ 0 }, player0_selections[2]);
    REQUIRE(execution.step(library, table, later_random) == givm::execution_state::finished);
    CHECK(later_random.consumed == 0);
    givm::dice_counts expected;
    for(givm::elemental_dice dice : { givm::elemental_dice::cryo, givm::elemental_dice::hydro, givm::elemental_dice::pyro,
                               givm::elemental_dice::anemo, givm::elemental_dice::geo, givm::elemental_dice::dendro })
    {
        ++expected[dice];
    }
    CHECK(table[givm::player_id{ 0 }].state().dice == expected);
}
