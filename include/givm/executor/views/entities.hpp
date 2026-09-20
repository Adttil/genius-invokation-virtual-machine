#ifndef GIVM_EXECUTOR_VIEWS_ENTITIES_HPP
#define GIVM_EXECUTOR_VIEWS_ENTITIES_HPP

#include <utility>

#include "../executor.hpp"

namespace givm
{
    template<>
    class execution_view<execution_state::active_character_changed>
    {
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;

    public:
        character_id character() const noexcept
        {
            const auto& event = get<0>(std::as_const(*stack_).top<
                active_character_changed, detail::execution_position>());
            return event.current;
        }
    };
}

#endif
