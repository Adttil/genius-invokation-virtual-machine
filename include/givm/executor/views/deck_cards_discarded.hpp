#ifndef GIVM_EXECUTOR_VIEWS_DECK_CARDS_DISCARDED_HPP
#define GIVM_EXECUTOR_VIEWS_DECK_CARDS_DISCARDED_HPP

#include <span>

#include "../executor.hpp"

namespace givm
{
    template<>
    class execution_view<execution_state::deck_cards_discarded>
    {
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;

    public:
        constexpr std::span<const deck_card_id> cards() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<deck_card_id[], stack_count_t>());
        }
    };
}

#endif
