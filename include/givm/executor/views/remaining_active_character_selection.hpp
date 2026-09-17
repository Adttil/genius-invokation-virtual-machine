#ifndef GIVM_EXECUTOR_VIEWS_REMAINING_ACTIVE_CHARACTER_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_REMAINING_ACTIVE_CHARACTER_SELECTION_HPP

#include <cstddef>
#include <cstdint>
#include <utility>

#include "../executor.hpp"

namespace givm
{
    enum class remaining_active_character_selection_validation : std::uint8_t
    {
        valid,
        invalid_player,
        wrong_player,
        invalid_character
    };

    template<>
    class execution_view<execution_state::remaining_active_character_selection>
    {
    public:
        constexpr player_id player() const noexcept
        {
            return other_player(first_selected_character().player_id);
        }

        constexpr character_id first_selected_character() const noexcept
        {
            return get<1>(std::as_const(*stack_).top<std::size_t, character_id>());
        }

        constexpr remaining_active_character_selection_validation selection_validate(
            const table& card_table, character_id character
        ) const noexcept
        {
            if(character.player_id.index >= 2)
            {
                return remaining_active_character_selection_validation::invalid_player;
            }
            if(character.player_id != player())
            {
                return remaining_active_character_selection_validation::wrong_player;
            }
            const auto characters = card_table[character.player_id].characters<false>();
            if(character.index >= characters.size() || not characters[character.index].is_valid())
            {
                return remaining_active_character_selection_validation::invalid_character;
            }
            return remaining_active_character_selection_validation::valid;
        }

        constexpr void select(character_id character) const noexcept
        {
            get<0>(stack_->top<std::size_t, character_id>()) = character.index;
        }

    private:
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;
    };
}

#endif
