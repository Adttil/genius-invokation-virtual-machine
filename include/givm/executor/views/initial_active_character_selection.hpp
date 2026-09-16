#ifndef GIVM_EXECUTOR_VIEWS_INITIAL_ACTIVE_CHARACTER_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_INITIAL_ACTIVE_CHARACTER_SELECTION_HPP

#include <cstdint>

#include "../executor.hpp"

namespace givm
{
    enum class initial_active_character_selection_check_result : std::uint8_t
    {
        valid,
        invalid_player,
        invalid_character
    };

    template<>
    class execution_view<execution_state::initial_active_character_selection>
    {
    public:
        constexpr initial_active_character_selection_check_result check_selection(
            const table& card_table, character_id character
        ) const noexcept
        {
            if(character.player_id.index >= 2)
            {
                return initial_active_character_selection_check_result::invalid_player;
            }
            const auto characters = card_table[character.player_id].characters<false>();
            if(character.index >= characters.size() || not characters[character.index].is_valid())
            {
                return initial_active_character_selection_check_result::invalid_character;
            }
            return initial_active_character_selection_check_result::valid;
        }

        constexpr void select(character_id character) const noexcept
        {
            get<0>(stack_->top<character_id>()) = character;
        }

    private:
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;
    };
}

#endif
