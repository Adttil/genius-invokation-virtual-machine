#ifndef GIVM_EXECUTOR_EXECUTION_STATE_HPP
#define GIVM_EXECUTOR_EXECUTION_STATE_HPP

#include <cstdint>

#include "../enums/role.hpp"

namespace givm
{
    enum class execution_state : std::uint8_t
    {
        finished = 0x03,
        initialized = 0x07,

        // Input states: bit 6 is set; bits 0-1 identify player 0, player 1, or both (2).
        card_selection0 = 0x40,
        card_selection1 = 0x41,
        remaining_active_character_selection0 = 0x44,
        remaining_active_character_selection1 = 0x45,
        active_character_selection0 = 0x48,
        active_character_selection1 = 0x49,
        dice_reroll_selection0 = 0x4c,
        dice_reroll_selection1 = 0x4d,
        dice_selection0 = 0x50,
        dice_selection1 = 0x51,
        dice_selection_both = 0x52,
        action_selection0 = 0x54,
        action_selection1 = 0x55,
        initial_card_selection = 0x5a,
        initial_active_character_selection = 0x5e,

        // Observed states: bit 7 is set; bits 0-1 are 3 (no input player).
        health_reduced = 0x83,
        deck_cards_discarded = 0x87,
        active_character_changed = 0x8b,
        initial_active_characters_selected = 0x8f,
        round_started = 0x93,
        action_started = 0x97,
        round_end_declared = 0x9b,
        round_ending = 0x9f
    };

    namespace detail
    {
        template<role Role>
        inline constexpr bool player_role = Role == role::player0 || Role == role::player1;

        template<role Role>
        inline constexpr unsigned role_player = Role == role::player0 ? 0u : 1u;

        template<execution_state State, role Role>
        inline constexpr bool accepts_input = (static_cast<unsigned>(State) & 0x40u) != 0
            && (Role == role::full || (player_role<Role>
                && ((static_cast<unsigned>(State) & 2u) != 0
                    || (static_cast<unsigned>(State) & 1u) == role_player<Role>)));

        inline constexpr execution_state continue_execution = static_cast<execution_state>(0);
    }
}

#endif
