#ifndef GIVM_EXECUTOR_VIEWS_DAMAGE_HPP
#define GIVM_EXECUTOR_VIEWS_DAMAGE_HPP

#include <cstdint>
#include <utility>

#include "../executor.hpp"
#include "../../definition_common.hpp"

namespace givm
{
    template<>
    class execution_view<execution_state::health_reduced>
    {
        friend class executor;
        constexpr explicit execution_view(executor& owner
#ifndef NDEBUG
            , std::size_t version
#endif
        ) noexcept : executor_{ &owner }
#ifndef NDEBUG
            , version_{ version }
#endif
        {}
        executor* executor_;
#ifndef NDEBUG
        std::size_t version_;
#endif

        const after_damage& event() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::health_reduced>(version_);
#endif
            auto&& [event] = std::as_const(executor_->context_.stack()).top<after_damage>();
            return event;
        }

    public:
        const damage_source_id& source() const noexcept(detail::view_checks_disabled) { return event().source; }
        character_id target() const noexcept(detail::view_checks_disabled) { return event().target; }
        std::uint32_t value() const noexcept(detail::view_checks_disabled) { return event().value; }
        damage_type type() const noexcept(detail::view_checks_disabled) { return event().type; }
        damage_flags flags() const noexcept(detail::view_checks_disabled) { return event().flags; }
        elemental_reaction reaction() const noexcept(detail::view_checks_disabled) { return event().reaction; }
        tag_id replacement_reaction() const noexcept(detail::view_checks_disabled) { return event().replacement_reaction; }

        template<class TRandom>
        execution_state resume(const definition_library& library, table& card_table, TRandom& random) const
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::health_reduced>(version_);
#endif
            return executor_->advance(library, card_table, random);
        }
    };
}

#endif
