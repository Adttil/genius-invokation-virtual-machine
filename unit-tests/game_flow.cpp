#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/givm.hpp>

namespace
{
    struct test_card_definition_source
    {
        using definition_category = givm::card_definition;

        struct definition_type{};

        constexpr std::string_view name() const noexcept
        {
            return "SmokeCard";
        }

        constexpr definition_type compile(givm::definition_compile_context&) const noexcept
        {
            return {};
        }
    };

    struct test_character_definition_source
    {
        using definition_category = givm::character_view;

        struct definition_type{};

        constexpr std::string_view name() const noexcept
        {
            return "SmokeCharacter";
        }

        constexpr definition_type compile(givm::definition_compile_context&) const noexcept
        {
            return {};
        }

        static givm::handler_program_entry_t<givm::character_initialization> handle(
            const definition_type&,
            const givm::character_view&,
            givm::character_initialization& event,
            const givm::table&,
            givm::random_fn&
        )
        {
            event.state = {
                .max_health = 10,
                .max_energy = 3,
                .health = 10,
                .energy = 0
            };
            return givm::handler_program_entry_t<givm::character_initialization>::null();
        }
    };

    struct increasing_random
    {
        std::uint32_t value = 0;

        std::uint32_t operator()() noexcept
        {
            return value++;
        }
    };

    givm::dice_counts select_one_available_dice(const givm::table& table, givm::player_id player)
    {
        givm::dice_counts result;
        const auto& dice = table[player].state().dice;
        for(std::size_t dice_index = 0; dice_index < 8; ++dice_index)
        {
            const auto dice_kind = static_cast<givm::elemental_dice>(dice_index);
            if(dice[dice_kind] != 0)
            {
                ++result[dice_kind];
                break;
            }
        }
        return result;
    }

    template<class TRandom>
    bool perform_first_available_switch(
        const givm::definition_library& library,
        givm::executor& target,
        givm::table& table,
        TRandom& random,
        givm::execution_state& state
    )
    {
        if(state != givm::execution_state::action)
        {
            return false;
        }

        const auto acting_player = table.state().active_player;
        const auto active_before = table[acting_player].state().active_character;
        if(not active_before.has_value())
        {
            return false;
        }

        givm::action_argument argument{
            .paid_dice = select_one_available_dice(table, acting_player)
        };
        if(argument.paid_dice.total() != 1)
        {
            return false;
        }

        const auto current = target.view_in<givm::execution_state::action>();
        if(current.costs().empty())
        {
            return false;
        }
        const auto dice_before = table[acting_player].state().dice.total();
        current.execute_action(0, argument);
        state = target.run(library, table, random);

        const auto active_after = table[acting_player].state().active_character;
        return active_after.has_value()
            && *active_after != *active_before
            && table[acting_player].state().dice.total() == dice_before - 1
            && table.state().active_player == other_player(acting_player)
            && state == givm::execution_state::action;
    }

    template<class TRandom>
    bool submit_round_end(
        const givm::definition_library& library,
        givm::executor& target, givm::table& table, TRandom& random, givm::execution_state& state
    )
    {
        if(state != givm::execution_state::action)
        {
            return false;
        }
        target.view_in<givm::execution_state::action>().declare_round_end();
        state = target.run(library, table, random);
        return true;
    }

}

TEST_CASE("minimal game reaches the max-round result", "[game-flow]")
{
    const test_card_definition_source card_source;
    const test_character_definition_source character_source;

    givm::definition_source_library source_library;
    REQUIRE(source_library.add(card_source, character_source));

    constexpr std::uint32_t max_rounds = 2;
    const auto initialization = std::tuple{
        givm::shuffle_deck{ .player = givm::player_id{ 0 } },
        givm::shuffle_deck{ .player = givm::player_id{ 1 } },
        givm::initialize_characters{ .player = givm::player_id{ 0 } },
        givm::initialize_characters{ .player = givm::player_id{ 1 } },
        givm::draw_cards{ .count = 5, .player = givm::relative_player::current },
        givm::draw_cards{ .count = 5, .player = givm::relative_player::other },
        givm::replace_cards_both{},
        givm::select_active_character_both{}
    };
    const auto round = std::tuple{
        givm::start_round{ .max_rounds = max_rounds },
        givm::start_dice_roll_phase{ .count = 8, .reroll_count = { 1, 1 } },
        givm::start_battle{},
        givm::begin_action{},
        givm::end_round{},
        givm::draw_cards{ .count = 2, .player = givm::relative_player::current },
        givm::draw_cards{ .count = 2, .player = givm::relative_player::other }
    };
    const auto [library, id_map] = source_library.compile(initialization, round);
    std::array<std::string_view, 10> card_names;
    card_names.fill(card_source.name());
    std::array<std::string_view, 3> character_names;
    character_names.fill(character_source.name());
    const auto deck = link_deck(id_map, card_names, character_names);

    givm::table table{
        givm::game_parameters{
            .hand_limit = 10
        }
    };
    table.load_deck(givm::player_id{ 0 }, deck);
    table.load_deck(givm::player_id{ 1 }, deck);
    givm::executor target;
    target.enter_entry(library);
    increasing_random random;


    auto state = target.run(library, table, random);
    REQUIRE(state == givm::execution_state::initial_card_selection);
    REQUIRE(table[givm::player_id{ 0 }].hand_card_count() == 5);
    REQUIRE(table[givm::player_id{ 1 }].hand_card_count() == 5);

    const auto first_selection_player = GENERATE(givm::player_id{ 0 }, givm::player_id{ 1 });
    const auto second_selection_player = other_player(first_selection_player);
    target.view_in<givm::execution_state::initial_card_selection>().select(
        first_selection_player, std::bitset<givm::selection_capacity>{ 0b11 }
    );
    state = target.run(library, table, random);
    REQUIRE(state == givm::execution_state::card_selection);
    REQUIRE(target.view_in<givm::execution_state::card_selection>().player() == second_selection_player);
    target.view_in<givm::execution_state::card_selection>().select({});
    state = target.run(library, table, random);

    REQUIRE(table[givm::player_id{ 0 }].hand_card_count() == 5);
    REQUIRE(table[givm::player_id{ 0 }].deck_card_count() == 5);
    REQUIRE(table[givm::player_id{ 1 }].hand_card_count() == 5);
    REQUIRE(table[givm::player_id{ 1 }].deck_card_count() == 5);

    REQUIRE(state == givm::execution_state::initial_active_character_selection);
    target.view_in<givm::execution_state::initial_active_character_selection>().select(
        givm::character_id{ .player_id = first_selection_player, .index = 0 }
    );
    state = target.run(library, table, random);

    REQUIRE(state == givm::execution_state::remaining_active_character_selection);
    const auto remaining = target.view_in<givm::execution_state::remaining_active_character_selection>();
    REQUIRE(remaining.player() == second_selection_player);
    CHECK(remaining.selected() == givm::character_id{ .player_id = first_selection_player, .index = 0 });
    remaining.select(0);
    state = target.run(library, table, random);

    REQUIRE(table[givm::player_id{ 0 }].state().active_character.has_value());
    REQUIRE(table[givm::player_id{ 1 }].state().active_character.has_value());

    for(std::uint32_t round = 1; round <= max_rounds; ++round)
    {
        CAPTURE(round);

        REQUIRE(state == givm::execution_state::dice_selection);
        REQUIRE(table.state().round_number == round);
        REQUIRE(table[givm::player_id{ 0 }].state().dice.total() == 8);
        REQUIRE(table[givm::player_id{ 1 }].state().dice.total() == 8);

        const auto prepared_random_count = random.value;
        REQUIRE(target.view_in<givm::execution_state::dice_selection>().remaining(first_selection_player) == 1);
        target.view_in<givm::execution_state::dice_selection>().select(
            first_selection_player, std::bitset<givm::selection_capacity>{ 1 }
        );
        state = target.run(library, table, random);
        REQUIRE(state == givm::execution_state::dice_selection);

        REQUIRE(target.view_in<givm::execution_state::dice_selection>().player() == second_selection_player);
        REQUIRE(target.view_in<givm::execution_state::dice_selection>().remaining(first_selection_player) == 0);
        CHECK(random.value == prepared_random_count);
        target.view_in<givm::execution_state::dice_selection>().select({});
        state = target.run(library, table, random);
        CHECK(random.value == prepared_random_count);
        REQUIRE(state == givm::execution_state::action);
        REQUIRE(table.state().active_player == givm::player_id{ 0 });

        REQUIRE(perform_first_available_switch(library, target, table, random, state));
        REQUIRE(perform_first_available_switch(library, target, table, random, state));
        REQUIRE(table.state().active_player == givm::player_id{ 0 });

        REQUIRE(submit_round_end(library, target, table, random, state));
        REQUIRE(state != givm::execution_state::finished);
        REQUIRE(table.state().first_ended);
        REQUIRE(table.state().active_player == givm::player_id{ 1 });

        REQUIRE(submit_round_end(library, target, table, random, state));
        REQUIRE(table[givm::player_id{ 0 }].hand_card_count() == 5 + round * 2);
        REQUIRE(table[givm::player_id{ 1 }].hand_card_count() == 5 + round * 2);
        if(round < max_rounds)
        {
            REQUIRE(state != givm::execution_state::finished);
        }
    }

    REQUIRE(state == givm::execution_state::finished);
    REQUIRE(table.state().round_number == max_rounds + 1);
    REQUIRE(target.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
}

TEST_CASE("step skips replacements and observes simultaneous initial active choices", "[game-flow][execution-view]")
{
    const test_card_definition_source card_source;
    const test_character_definition_source character_source;
    givm::definition_source_library sources;
    REQUIRE(sources.add(card_source, character_source));
    const auto [library, id_map] = sources.compile(
        std::tuple{
            givm::initialize_characters{ .player = givm::player_id{ 0 } },
            givm::initialize_characters{ .player = givm::player_id{ 1 } },
            givm::draw_cards{ .count = 5, .player = givm::relative_player::current },
            givm::draw_cards{ .count = 5, .player = givm::relative_player::other },
            givm::replace_cards_both{},
            givm::select_active_character_both{},
            givm::begin_action{}
        },
        std::tuple{}
    );
    std::array<std::string_view, 10> cards;
    cards.fill(card_source.name());
    std::array<std::string_view, 3> characters;
    characters.fill(character_source.name());
    const auto deck = link_deck(id_map, cards, characters);
    givm::table table{ givm::game_parameters{ .hand_limit = 10 } };
    table.load_deck(givm::player_id{ 0 }, deck);
    table.load_deck(givm::player_id{ 1 }, deck);
    givm::executor target;
    target.enter_entry(library);
    increasing_random random;

    REQUIRE(target.step(library, table, random) == givm::execution_state::initial_card_selection);
    const auto prepared_random_count = random.value;
    const std::bitset<givm::selection_capacity> replaced{ 0b11 };
    target.view_in<givm::execution_state::initial_card_selection>().select(givm::player_id{ 1 }, replaced);
    REQUIRE(target.step(library, table, random) == givm::execution_state::card_selection);
    CHECK(table[givm::player_id{ 1 }].hand_card_count() == 5);
    CHECK(table[givm::player_id{ 1 }].deck_card_count() == 5);
    CHECK(random.value == prepared_random_count);

    REQUIRE(target.view_in<givm::execution_state::card_selection>().player() == givm::player_id{ 0 });
    target.view_in<givm::execution_state::card_selection>().select({});
    REQUIRE(target.step(library, table, random) == givm::execution_state::initial_active_character_selection);
    CHECK(random.value == prepared_random_count);

    const givm::character_id player1_choice{ .player_id = givm::player_id{ 1 }, .index = 1 };
    const givm::character_id player0_choice{ .player_id = givm::player_id{ 0 }, .index = 2 };
    target.view_in<givm::execution_state::initial_active_character_selection>().select(player1_choice);
    CHECK_FALSE(table[givm::player_id{ 0 }].state().active_character.has_value());
    CHECK_FALSE(table[givm::player_id{ 1 }].state().active_character.has_value());
    REQUIRE(target.step(library, table, random) == givm::execution_state::remaining_active_character_selection);
    CHECK_FALSE(table[givm::player_id{ 0 }].state().active_character.has_value());
    CHECK_FALSE(table[givm::player_id{ 1 }].state().active_character.has_value());
    const auto remaining = target.view_in<givm::execution_state::remaining_active_character_selection>();
    CHECK(remaining.selected() == player1_choice);
    CHECK(remaining.player() == givm::player_id{ 0 });
    remaining.select(player0_choice.index);
    CHECK(remaining.selected() == player1_choice);
    CHECK(remaining.player() == givm::player_id{ 0 });
    CHECK_FALSE(table[givm::player_id{ 0 }].state().active_character.has_value());
    CHECK_FALSE(table[givm::player_id{ 1 }].state().active_character.has_value());
    REQUIRE(target.step(library, table, random) == givm::execution_state::initial_active_characters_selected);

    CHECK(table[givm::player_id{ 0 }].state().active_character == player0_choice);
    CHECK(table[givm::player_id{ 1 }].state().active_character == player1_choice);
    REQUIRE(target.step(library, table, random) == givm::execution_state::action_started);
    REQUIRE(target.step(library, table, random) == givm::execution_state::action);
}
