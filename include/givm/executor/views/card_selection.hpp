#ifndef GIVM_EXECUTOR_VIEWS_CARD_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_CARD_SELECTION_HPP

#include <bitset>
#include <utility>

#include "../executor.hpp"

namespace givm
{
    template<>
    class execution_view<execution_state::card_selection>
    {
    public:
        constexpr player_id player() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<player_id, std::bitset<selection_capacity>>());
        }

        constexpr std::bitset<selection_capacity> selected() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<std::bitset<selection_capacity>>());
        }

        constexpr bool check_selection(
            const table& card_table, std::bitset<selection_capacity> selected
        ) const noexcept
        {
            return (selected >> card_table[player()].hand_card_count()).none();
        }

        constexpr void select(std::bitset<selection_capacity> selected) const noexcept
        {
            get<0>(stack_->top<std::bitset<selection_capacity>>()) = selected;
        }

    private:
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;
    };
}

#endif
