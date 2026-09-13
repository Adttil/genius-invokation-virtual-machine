#ifndef GIVM_EXECUTOR_VIEWS_DAMAGE_HPP
#define GIVM_EXECUTOR_VIEWS_DAMAGE_HPP

#include <cstdint>
#include <utility>

#include "../executor.hpp"
#include "../events.hpp"

namespace givm
{
    template<>
    class execution_view<execution_state::health_reduced>
    {
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;

        const after_damage& event() const noexcept
        {
            auto&& [event, stage] = std::as_const(*stack_).top<after_damage, detail::stage_t>();
            return event;
        }

    public:
        const damage_source_id& source() const noexcept { return event().source; }
        character_id target() const noexcept { return event().target; }
        std::uint32_t value() const noexcept { return event().value; }
        damage_type type() const noexcept { return event().type; }
        damage_flags flags() const noexcept { return event().flags; }
    };
}

#endif
