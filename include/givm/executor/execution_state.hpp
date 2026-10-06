#ifndef GIVM_EXECUTOR_EXECUTION_STATE_HPP
#define GIVM_EXECUTOR_EXECUTION_STATE_HPP

#include <cstdint>

namespace givm
{
    enum class execution_state : std::uint8_t
    {
        finished = 1,
        card_selection,
        initial_card_selection,
        initial_active_character_selection,
        remaining_active_character_selection,
        active_character_selection,
        dice_selection,
        action_selection,
        health_reduced,
        deck_cards_discarded,
        active_character_changed,
        initial_active_characters_selected,
        round_started,
        action_started,
        round_end_declared,
        round_ending,
        dice_reroll_selection,
        initialized
    };

    namespace detail
    {
        inline constexpr execution_state continue_execution = static_cast<execution_state>(0);
    }
}

#endif
