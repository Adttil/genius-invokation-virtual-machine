#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <tuple>

#include <catch2/catch_test_macros.hpp>

#include <givm/givm.hpp>

using namespace givm;

namespace
{
    struct test_card_definition_source
    {
        using definition_category = card_definition;

        struct definition_type{};

        constexpr std::string_view name() const noexcept
        {
            return "SmokeCard";
        }

        constexpr definition_type compile(definition_compile_context&) const noexcept
        {
            return {};
        }
    };

    struct test_character_definition_source
    {
        using definition_category = character_view;

        struct definition_type{};

        constexpr std::string_view name() const noexcept
        {
            return "SmokeCharacter";
        }

        constexpr definition_type compile(definition_compile_context&) const noexcept
        {
            return {};
        }

        static handler_program_entry_t<character_initialization> handle(
            const definition_type&,
            const character_view&,
            character_initialization& event,
            const card_table&,
            random_fn&
        )
        {
            event.state = {
                .max_health = 10,
                .max_energy = 3,
                .health = 10,
                .energy = 0
            };
            return handler_program_entry_t<character_initialization>::null();
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

    template<class TRandom>
    void run_until_blocked(
        executor& target,
        card_table& table,
        TRandom& random
    )
    {
        while(
            target.status() == game_result::no_result
            && target.execute_next(table, random)
        )
        {
        }
    }

    bool submit_selector(
        executor& target,
        player_id player,
        std::bitset<selection_capacity> selected
    )
    {
        if(target.stack().empty())
        {
            return false;
        }

        auto&& [input, stage] = target.stack().top<selector, stage_t>();
        (void)stage;
        input.player = player;
        input.selected = selected;
        return true;
    }

    bool submit_active_character_selection(executor& target, character_id selected)
    {
        if(target.stack().empty())
        {
            return false;
        }

        auto&& [input, stage] = target.stack().top<character_id, stage_t>();
        (void)stage;
        input = selected;
        return true;
    }

    dice_counts select_one_available_dice(const card_table& table, player_id player)
    {
        dice_counts result;
        const auto& dice = table[player].state().dice;
        for(size_t dice_index = 0; dice_index < 8; ++dice_index)
        {
            const auto dice_kind = static_cast<elemental_dice>(dice_index);
            if(dice[dice_kind] != 0)
            {
                ++result[dice_kind];
                break;
            }
        }
        return result;
    }

    bool waiting_for_action(const executor& target, const card_table& table)
    {
        return target.status() == game_result::no_result
            && table.definition_library().instruction(target.position()).is<begin_action>();
    }

    bool submit_action(
        executor& target,
        const card_table& table,
        action_request request,
        action_argument argument = {}
    )
    {
        if(not waiting_for_action(target, table))
        {
            return false;
        }

        auto&& [stored_argument, stored_request, stage] =
            target.stack().top<action_argument, action_request, stage_t>();
        (void)stage;
        stored_argument = argument;
        stored_request = request;
        return true;
    }

    template<class TRandom>
    bool perform_first_available_switch(executor& target, card_table& table, TRandom& random)
    {
        if(not waiting_for_action(target, table))
        {
            return false;
        }

        const auto acting_player = table.state().active_player;
        const auto active_before = table[acting_player].state().active_character;
        if(not active_before.has_value())
        {
            return false;
        }

        action_argument argument{
            .paid_dice = select_one_available_dice(table, acting_player)
        };
        if(argument.paid_dice.total() != 1)
        {
            return false;
        }

        const auto dice_before = table[acting_player].state().dice.total();
        if(not submit_action(
            target,
            table,
            action_request{
                .request_kind = action_request_kind::do_action,
                .action_kind = action_kind::switch_active,
                .action_index = 0
            },
            argument
        ))
        {
            return false;
        }

        run_until_blocked(target, table, random);

        const auto active_after = table[acting_player].state().active_character;
        return active_after.has_value()
            && *active_after != *active_before
            && table[acting_player].state().dice.total() == dice_before - 1
            && table.state().active_player == other_player(acting_player)
            && waiting_for_action(target, table);
    }

    template<class TRandom>
    bool submit_round_end(executor& target, card_table& table, TRandom& random)
    {
        if(not submit_action(
            target,
            table,
            action_request{
                .request_kind = action_request_kind::do_action,
                .action_kind = action_kind::declare_round_end
            }
        ))
        {
            return false;
        }

        run_until_blocked(target, table, random);
        return true;
    }

    bool game_ended(const executor& target)
    {
        return target.status() != game_result::no_result;
    }
}

TEST_CASE("minimal game reaches the max-round result", "[game-flow]")
{
    const test_card_definition_source card_source;
    const test_character_definition_source character_source;

    definition_source_library source_library;
    REQUIRE(source_library.add(card_source, character_source));

    constexpr std::uint32_t max_rounds = 2;
    const auto initialization = std::tuple{
        shuffle_deck{ .player = player_id{ 0 } },
        shuffle_deck{ .player = player_id{ 1 } },
        initialize_characters{ .player = player_id{ 0 } },
        initialize_characters{ .player = player_id{ 1 } },
        draw_cards{ .count = 5, .player = relative_player::current },
        draw_cards{ .count = 5, .player = relative_player::other },
        replace_cards_both{},
        select_active_character_both{}
    };
    const auto round = std::tuple{
        start_round{ .max_rounds = max_rounds },
        start_dice_roll_phase{ .count = 8, .reroll_count = { 1, 1 } },
        start_battle{},
        begin_action{},
        end_round{},
        draw_cards{ .count = 2, .player = relative_player::current },
        draw_cards{ .count = 2, .player = relative_player::other }
    };
    const auto [library, id_map] = source_library.compile(initialization, round);
    std::array<std::string_view, 10> card_names;
    card_names.fill(card_source.name());
    std::array<std::string_view, 3> character_names;
    character_names.fill(character_source.name());
    const auto deck = link_deck(id_map, card_names, character_names);

    card_table table{
        library,
        game_parameters{
            .hand_limit = 10
        }
    };
    table.load_deck(player_id{ 0 }, deck);
    table.load_deck(player_id{ 1 }, deck);
    executor target;
    target.enter_entry(table.definition_library());
    increasing_random random;

    table.state().active_player = player_id{ 0 };

    run_until_blocked(target, table, random);
    REQUIRE(table[player_id{ 0 }].hand_card_count() == 5);
    REQUIRE(table[player_id{ 1 }].hand_card_count() == 5);

    REQUIRE(submit_selector(target, player_id{ 0 }, std::bitset<selection_capacity>{ 0b11 }));
    run_until_blocked(target, table, random);
    REQUIRE(submit_selector(target, player_id{ 1 }, {}));
    run_until_blocked(target, table, random);

    REQUIRE(table[player_id{ 0 }].hand_card_count() == 5);
    REQUIRE(table[player_id{ 0 }].deck_card_count() == 5);
    REQUIRE(table[player_id{ 1 }].hand_card_count() == 5);
    REQUIRE(table[player_id{ 1 }].deck_card_count() == 5);

    REQUIRE(table.definition_library().instruction(target.position()).is<select_active_character_both>());
    REQUIRE(submit_active_character_selection(
        target,
        character_id{ .player_id = player_id{ 0 }, .index = 0 }
    ));
    run_until_blocked(target, table, random);

    REQUIRE(table.definition_library().instruction(target.position()).is<select_active_character_both>());
    REQUIRE(submit_active_character_selection(
        target,
        character_id{ .player_id = player_id{ 1 }, .index = 0 }
    ));
    run_until_blocked(target, table, random);

    REQUIRE(table[player_id{ 0 }].state().active_character.has_value());
    REQUIRE(table[player_id{ 1 }].state().active_character.has_value());

    for(std::uint32_t round = 1; round <= max_rounds; ++round)
    {
        CAPTURE(round);

        REQUIRE(table.definition_library().instruction(target.position()).is<start_dice_roll_phase>());
        REQUIRE(table.state().round_number == round);
        REQUIRE(table[player_id{ 0 }].state().dice.total() == 8);
        REQUIRE(table[player_id{ 1 }].state().dice.total() == 8);

        REQUIRE(submit_selector(target, player_id{ 0 }, {}));
        run_until_blocked(target, table, random);
        REQUIRE(table.definition_library().instruction(target.position()).is<start_dice_roll_phase>());

        REQUIRE(submit_selector(target, player_id{ 1 }, {}));
        run_until_blocked(target, table, random);
        REQUIRE(waiting_for_action(target, table));
        REQUIRE(table.state().active_player == player_id{ 0 });

        REQUIRE(perform_first_available_switch(target, table, random));
        REQUIRE(perform_first_available_switch(target, table, random));
        REQUIRE(table.state().active_player == player_id{ 0 });

        REQUIRE(submit_round_end(target, table, random));
        REQUIRE(not game_ended(target));
        REQUIRE(table.state().first_ended);
        REQUIRE(table.state().active_player == player_id{ 1 });

        REQUIRE(submit_round_end(target, table, random));
        REQUIRE(table[player_id{ 0 }].hand_card_count() == 5 + round * 2);
        REQUIRE(table[player_id{ 1 }].hand_card_count() == 5 + round * 2);
        if(round < max_rounds)
        {
            REQUIRE(not game_ended(target));
        }
    }

    REQUIRE(game_ended(target));
    REQUIRE(table.state().round_number == max_rounds);
    REQUIRE(target.status() == game_result::both_loss);
}
