#ifndef GIVM_DEFINITION_COMMANDS_HPP
#define GIVM_DEFINITION_COMMANDS_HPP

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <variant>

#include "../table.hpp"
#include "../utils/type_list.hpp"
#include "events.hpp"
#include "../enums/damage_flags.hpp"
#include "../enums/damage_type.hpp"
#include "../enums/element_application_cause.hpp"
#include "../enums/game_result.hpp"

namespace givm
{
    struct insert_deck_card
    {
        using input_type = void;

        player_id player;
        definition_id<card_definition> definition;
        std::int32_t position = -1;
    };

    struct enter_character
    {
        using input_type = void;

        player_id player;
        definition_id<character_view> definition;
    };

    struct shuffle_deck
    {
        using input_type = void;

        player_id player;
    };

    struct set_active_character
    {
        using input_type = void;

        character_id target;
    };

    struct set_active_character_from_input
    {
        using input_type = active_character_changed;
    };

    struct select_active_character_both
    {
        using input_type = void;

    };

    enum class relative_player : std::uint8_t
    {
        current,
        other
    };

    struct draw_cards
    {
        using input_type = void;

        std::uint32_t count;
        relative_player player = relative_player::current;
    };

    struct replace_cards
    {
        using input_type = void;

        player_id player;
    };

    struct replace_cards_both
    {
        using input_type = void;

    };

    struct start_round
    {
        using input_type = void;

        std::uint32_t max_rounds = 14;
    };

    struct begin_action
    {
        using input_type = void;

    };

    struct end_round
    {
        using input_type = void;
    };

    struct end_game
    {
        using input_type = void;

        game_result result;
    };

    struct start_dice_roll_phase
    {
        using input_type = void;

        std::uint32_t count = 8;
        std::array<std::uint32_t, 2> reroll_count{ 1, 1 };

    };

    struct start_battle
    {
        using input_type = void;
    };

    struct reduce_combat_status_count
    {
        using input_type = combat_status_count_reduction;
    };

    struct deal_damage
    {
        using input_type = void;

        damage_source_id source;
        character_id target;
        std::uint32_t value;
        std::uint16_t multiplier_numerator = 1;
        std::uint16_t multiplier_denominator = 1;
        damage_type type;
        damage_flags flags;
    };

    struct apply_element
    {
        using input_type = void;

        element_application_source_id source;
        character_id target;
        element element;
        element_application_cause cause = element_application_cause::effect;
    };

    struct set_element_aura
    {
        using input_type = void;

        character_id target;
        element_aura aura;
    };

    struct test_command
    {
        using input_type = void;
    };
}

namespace givm::detail
{
    using command_types = type_list<
        insert_deck_card,
        enter_character,
        shuffle_deck,
        set_active_character,
        set_active_character_from_input,
        select_active_character_both,
        draw_cards,
        replace_cards,
        replace_cards_both,
        start_round,
        begin_action,
        end_round,
        end_game,
        start_dice_roll_phase,
        start_battle,
        reduce_combat_status_count,
        deal_damage,
        apply_element,
        set_element_aura,
        test_command>;

}

namespace givm
{
    using any_command = detail::command_types::apply<std::variant>;
}

#endif
