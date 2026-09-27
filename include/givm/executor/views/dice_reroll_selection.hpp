#ifndef GIVM_EXECUTOR_VIEWS_DICE_REROLL_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_DICE_REROLL_SELECTION_HPP

#include <utility>

#include "../executor.hpp"
#include "../commands/reroll_dice.hpp"

namespace givm
{
    template<>
    class execution_view<execution_state::dice_reroll_selection>
    {
    public:
        constexpr player_id player() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<detail::single_player_dice_reroll, dice_counts>()).player;
        }

        constexpr bool selection_validate(const table& card_table, const dice_counts& selected) const noexcept
        {
            return card_table[player()].state().dice.contains(selected);
        }

        constexpr void select(const dice_counts& selected) const noexcept
        {
            get<0>(stack_->top<dice_counts>()) = selected;
        }

    private:
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;
    };
}

#endif
