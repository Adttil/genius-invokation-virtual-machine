#include <bitset>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/executor.hpp>

#include "../table/test_definition_library.hpp"

namespace
{
    struct counting_random
    {
        std::size_t calls = 0;

        std::uint32_t operator()() noexcept
        {
            ++calls;
            return 0;
        }
    };

    std::vector<givm::definition_id<givm::card_definition>> hand_definitions(givm::player_view player)
    {
        std::vector<givm::definition_id<givm::card_definition>> result;
        for(const auto card : player.hand_cards()) result.push_back(card.definition_id());
        return result;
    }
}

TEST_CASE("card selection checks leave submitted replacements and the table unchanged", "[execution-view][selection][cards]")
{
    using check_result = givm::initial_card_selection_check_result;
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const givm::test::named_definition_source<givm::card_definition> alpha{ "Alpha" };
    const givm::test::named_definition_source<givm::card_definition> beta{ "Beta" };
    const givm::test::named_definition_source<givm::card_definition> gamma{ "Gamma" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        mode,
        std::tuple{
            givm::draw_cards{ .count = 2 },
            givm::draw_cards{ .count = 2, .player = givm::relative_player::other },
            givm::replace_cards_both{},
            givm::end_game{ givm::game_result::both_loss }
        },
        std::tuple{}, alpha, beta, gamma
    );
    const auto a = ids.get_id<givm::card_definition>(alpha.name());
    const auto b = ids.get_id<givm::card_definition>(beta.name());
    const auto c = ids.get_id<givm::card_definition>(gamma.name());
    givm::table table;
    for(const givm::player_id player : { givm::player_id{ 0 }, givm::player_id{ 1 } })
        table.load_deck(player, { .cards = { a, b, c } });
    givm::executor execution;
    execution.enter_entry(library);
    counting_random random;
    REQUIRE(execution.step(library, table, random) == givm::execution_state::initial_card_selection);
    const auto prepared_random_count = random.calls;
    const auto initial = execution.view_in<givm::execution_state::initial_card_selection>();
    initial.select(givm::player_id{ 1 }, std::bitset<givm::selection_capacity>{ 0b01 });
    CHECK(initial.check_selection(table, givm::player_id{ 0 }, {}) == check_result::valid);
    CHECK(initial.check_selection(table, givm::player_id{ 0 }, std::bitset<givm::selection_capacity>{ 0b10 })
        == check_result::valid);
    CHECK(initial.check_selection(table, givm::player_id{ 2 }, {}) == check_result::invalid_player);
    CHECK(initial.check_selection(table, givm::player_id{ std::numeric_limits<std::size_t>::max() }, {})
        == check_result::invalid_player);
    CHECK(initial.check_selection(table, givm::player_id{ 1 }, std::bitset<givm::selection_capacity>{ 0b100 })
        == check_result::invalid_card_position);
    std::bitset<givm::selection_capacity> last_bit;
    last_bit.set(givm::selection_capacity - 1);
    CHECK(initial.check_selection(table, givm::player_id{ 1 }, last_bit) == check_result::invalid_card_position);
    CHECK(initial.check_selection(table, givm::player_id{ 2 }, last_bit) == check_result::invalid_player);
    for(const givm::player_id player : { givm::player_id{ 0 }, givm::player_id{ 1 } })
    {
        CHECK(hand_definitions(table[player]) == std::vector{ c, b });
        CHECK(table[player].deck_card_count() == 1);
        CHECK(table[player].deck_card_definition(0) == a);
    }
    CHECK(random.calls == prepared_random_count);

    REQUIRE(execution.step(library, table, random) == givm::execution_state::card_selection);
    CHECK(hand_definitions(table[givm::player_id{ 1 }]) == std::vector{ b, a });
    const auto remaining = execution.view_in<givm::execution_state::card_selection>();
    REQUIRE(remaining.player() == givm::player_id{ 0 });
    const std::bitset<givm::selection_capacity> submitted{ 0b10 };
    remaining.select(submitted);
    CHECK(remaining.check_selection(table, {}));
    CHECK(remaining.check_selection(table, std::bitset<givm::selection_capacity>{ 0b01 }));
    CHECK_FALSE(remaining.check_selection(table, std::bitset<givm::selection_capacity>{ 0b100 }));
    CHECK_FALSE(remaining.check_selection(table, last_bit));
    CHECK(remaining.selected() == submitted);
    CHECK(remaining.player() == givm::player_id{ 0 });
    CHECK(hand_definitions(table[givm::player_id{ 0 }]) == std::vector{ c, b });
    CHECK(table[givm::player_id{ 0 }].deck_card_count() == 1);
    CHECK(random.calls == prepared_random_count);
    REQUIRE(execution.step(library, table, random) == givm::execution_state::finished);
    CHECK(hand_definitions(table[givm::player_id{ 0 }]) == std::vector{ c, a });
    CHECK(random.calls == prepared_random_count);
}

TEST_CASE("card selections cover their highest bit when the hand reaches or exceeds the mask capacity", "[execution-view][selection][cards]")
{
    const auto hand_count = GENERATE(givm::selection_capacity, givm::selection_capacity + 1);
    const givm::test::named_definition_source<givm::card_definition> alpha{ "Alpha" };
    const givm::test::named_definition_source<givm::card_definition> beta{ "Beta" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        givm::compile_mode::normal,
        std::tuple{
            givm::draw_cards{ .count = static_cast<std::uint32_t>(hand_count) },
            givm::replace_cards{ .player = givm::player_id{ 0 } },
            givm::end_game{ givm::game_result::both_loss }
        }, std::tuple{}, alpha, beta
    );
    const auto a = ids.get_id<givm::card_definition>(alpha.name());
    const auto b = ids.get_id<givm::card_definition>(beta.name());
    givm::linked_deck deck;
    deck.cards.assign(hand_count + 1, a);
    deck.cards.front() = b;
    givm::table table{ givm::game_parameters{ .hand_limit = static_cast<std::uint32_t>(hand_count) } };
    table.load_deck(givm::player_id{ 0 }, deck);
    givm::executor execution;
    execution.enter_entry(library);
    counting_random random;
    REQUIRE(execution.step(library, table, random) == givm::execution_state::card_selection);
    REQUIRE(table[givm::player_id{ 0 }].hand_card_count() == hand_count);
    const auto input = execution.view_in<givm::execution_state::card_selection>();
    std::bitset<givm::selection_capacity> highest_bit;
    highest_bit.set(givm::selection_capacity - 1);
    CHECK(input.check_selection(table, highest_bit));
    input.select(highest_bit);
    REQUIRE(execution.step(library, table, random) == givm::execution_state::finished);
    std::vector expected(hand_count, a);
    expected.back() = b;
    CHECK(hand_definitions(table[givm::player_id{ 0 }]) == expected);
    CHECK(table[givm::player_id{ 0 }].deck_card_count() == 1);
    CHECK(table[givm::player_id{ 0 }].deck_card_definition(0) == a);
    CHECK(random.calls == 1);
}

TEST_CASE("initial character checks validate ownership and existence without requiring health", "[execution-view][selection][characters]")
{
    using initial_check_result = givm::initial_active_character_selection_check_result;
    using remaining_check_result = givm::remaining_active_character_selection_check_result;
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const givm::test::named_definition_source<givm::character_view> character{ "Character" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        mode,
        std::tuple{ givm::select_active_character_both{}, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, character
    );
    const auto definition = ids.get_id<givm::character_view>(character.name());
    givm::table table;
    for(const givm::player_id player : { givm::player_id{ 0 }, givm::player_id{ 1 } })
        table.load_deck(player, { .characters = { definition, definition } });
    const givm::character_id first_choice{ givm::player_id{ 1 }, 1 };
    const givm::character_id second_choice{ givm::player_id{ 0 }, 1 };
    REQUIRE(table[first_choice].state().health == 0);
    REQUIRE(table[second_choice].state().health == 0);
    givm::executor execution;
    execution.enter_entry(library);
    counting_random random;
    REQUIRE(execution.step(library, table, random) == givm::execution_state::initial_active_character_selection);
    const auto initial = execution.view_in<givm::execution_state::initial_active_character_selection>();
    initial.select(first_choice);
    CHECK(initial.check_selection(table, first_choice) == initial_check_result::valid);
    CHECK(initial.check_selection(table, second_choice) == initial_check_result::valid);
    CHECK(initial.check_selection(table,
        givm::character_id{ givm::player_id{ 2 }, std::numeric_limits<std::size_t>::max() })
        == initial_check_result::invalid_player);
    CHECK(initial.check_selection(table, givm::character_id{ givm::player_id{ 0 }, 2 })
        == initial_check_result::invalid_character);
    CHECK(initial.check_selection(table,
        givm::character_id{ givm::player_id{ 1 }, std::numeric_limits<std::size_t>::max() })
        == initial_check_result::invalid_character);
    CHECK_FALSE(table[givm::player_id{ 0 }].state().active_character.has_value());
    CHECK_FALSE(table[givm::player_id{ 1 }].state().active_character.has_value());
    CHECK(random.calls == 0);
    REQUIRE(execution.step(library, table, random) == givm::execution_state::remaining_active_character_selection);
    const auto remaining = execution.view_in<givm::execution_state::remaining_active_character_selection>();
    CHECK(remaining.first_selected_character() == first_choice);
    CHECK(remaining.player() == givm::player_id{ 0 });
    remaining.select(second_choice);
    CHECK(remaining.check_selection(table, second_choice) == remaining_check_result::valid);
    CHECK(remaining.check_selection(table, givm::character_id{ givm::player_id{ 0 }, 0 }) == remaining_check_result::valid);
    CHECK(remaining.check_selection(table, first_choice) == remaining_check_result::wrong_player);
    CHECK(remaining.check_selection(table,
        givm::character_id{ givm::player_id{ 2 }, std::numeric_limits<std::size_t>::max() })
        == remaining_check_result::invalid_player);
    CHECK(remaining.check_selection(table, givm::character_id{ givm::player_id{ 0 }, 2 })
        == remaining_check_result::invalid_character);
    CHECK(remaining.check_selection(table, givm::character_id{ givm::player_id{ 1 }, 2 })
        == remaining_check_result::wrong_player);
    CHECK(remaining.first_selected_character() == first_choice);
    CHECK(remaining.player() == givm::player_id{ 0 });
    CHECK_FALSE(table[givm::player_id{ 0 }].state().active_character.has_value());
    CHECK_FALSE(table[givm::player_id{ 1 }].state().active_character.has_value());
    auto state = execution.step(library, table, random);
    if(mode == givm::compile_mode::observed)
    {
        REQUIRE(state == givm::execution_state::initial_active_characters_selected);
        state = execution.step(library, table, random);
    }
    REQUIRE(state == givm::execution_state::finished);
    CHECK(table[givm::player_id{ 0 }].state().active_character == second_choice);
    CHECK(table[givm::player_id{ 1 }].state().active_character == first_choice);
    CHECK(table[first_choice].state().health == 0);
    CHECK(table[second_choice].state().health == 0);
    CHECK(random.calls == 0);
}

TEST_CASE("dice checks validate available counts and rerolls without changing a submitted choice", "[execution-view][selection][dice]")
{
    using check_result = givm::dice_selection_check_result;
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    givm::definition_source_library sources;
    const auto [library, ids] = compile(sources,
        std::tuple{
            givm::start_dice_roll_phase{ .count = 4, .reroll_count = { 1, 2 } },
            givm::end_game{ givm::game_result::both_loss }
        }, std::tuple{}, mode
    );
    givm::table table;
    givm::executor execution;
    execution.enter_entry(library);
    counting_random random;
    REQUIRE(execution.step(library, table, random) == givm::execution_state::dice_selection);
    const auto prepared_random_count = random.calls;
    const auto player0_dice = table[givm::player_id{ 0 }].state().dice;
    const auto player1_dice = table[givm::player_id{ 1 }].state().dice;
    const auto input = execution.view_in<givm::execution_state::dice_selection>();
    givm::dice_counts submitted;
    submitted[givm::elemental_dice::omni] = 2;
    givm::dice_counts alternative;
    alternative[givm::elemental_dice::omni] = 1;
    givm::dice_counts too_many;
    too_many[givm::elemental_dice::omni] = 5;
    givm::dice_counts absent_type;
    absent_type[givm::elemental_dice::cryo] = 1;
    input.select(givm::player_id{ 1 }, submitted);
    CHECK(input.check_selection(table, {}));
    CHECK(input.check_selection(table, alternative));
    CHECK(input.check_selection(table, givm::player_id{ 0 }, alternative) == check_result::valid);
    CHECK_FALSE(input.check_selection(table, too_many));
    CHECK_FALSE(input.check_selection(table, absent_type));
    CHECK(input.check_selection(table, givm::player_id{ 0 }, too_many) == check_result::insufficient_dice);
    CHECK(input.check_selection(table, givm::player_id{ 2 }, too_many) == check_result::invalid_player);
    CHECK(input.check_selection(table, givm::player_id{ std::numeric_limits<std::size_t>::max() }, {})
        == check_result::invalid_player);
    CHECK(input.player() == givm::player_id{ 1 });
    CHECK(input.selected() == submitted);
    CHECK(input.remaining(givm::player_id{ 0 }) == 1);
    CHECK(input.remaining(givm::player_id{ 1 }) == 2);
    CHECK(table[givm::player_id{ 0 }].state().dice == player0_dice);
    CHECK(table[givm::player_id{ 1 }].state().dice == player1_dice);
    CHECK(random.calls == prepared_random_count);
    REQUIRE(execution.step(library, table, random) == givm::execution_state::dice_selection);

    const auto player0_input = execution.view_in<givm::execution_state::dice_selection>();
    REQUIRE(player0_input.player() == givm::player_id{ 0 });
    CHECK(player0_input.remaining(givm::player_id{ 1 }) == 1);
    player0_input.select(alternative);
    REQUIRE(execution.step(library, table, random) == givm::execution_state::dice_selection);
    const auto last_input = execution.view_in<givm::execution_state::dice_selection>();
    REQUIRE(last_input.player() == givm::player_id{ 1 });
    CHECK(last_input.remaining(givm::player_id{ 0 }) == 0);
    CHECK(last_input.check_selection(table, givm::player_id{ 0 }, {}) == check_result::no_rerolls_remaining);
    CHECK(last_input.check_selection(table, givm::player_id{ 0 }, alternative) == check_result::no_rerolls_remaining);
    CHECK(last_input.check_selection(table, givm::player_id{ 0 }, too_many) == check_result::no_rerolls_remaining);
    CHECK(last_input.check_selection(table, {}));
    last_input.select({});
    REQUIRE(execution.step(library, table, random) == givm::execution_state::finished);
    CHECK(random.calls == prepared_random_count);
}
