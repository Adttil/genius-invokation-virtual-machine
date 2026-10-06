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

    public:
        character_id character() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::active_character_changed>(version_);
#endif
            const auto& event = get<0>(std::as_const(executor_->context_.stack()).top<
                active_character_changed>());
            return event.current;
        }

        template<class TRandom>
        execution_state resume(const definition_library& library, table& card_table, TRandom& random) const
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::active_character_changed>(version_);
#endif
            return executor_->advance(library, card_table, random);
        }
    };
}

#endif
