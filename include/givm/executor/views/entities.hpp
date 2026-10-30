#ifndef GIVM_EXECUTOR_VIEWS_ENTITIES_HPP
#define GIVM_EXECUTOR_VIEWS_ENTITIES_HPP

#include <utility>

#include "../executor.hpp"

namespace givm
{
    template<>
    class execution_view<execution_state::active_character_changed, role::full>
    {
        friend class executor;
        template<execution_state, role> friend class execution_view;
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
        static constexpr execution_state state = execution_state::active_character_changed;
        static constexpr givm::role access_role = role::full;
        static constexpr bool accepts_input = false;

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

    template<role Role>
    requires (Role != role::full)
    class execution_view<execution_state::active_character_changed, Role> : private execution_view<execution_state::active_character_changed, role::full>
    {
        using base = execution_view<execution_state::active_character_changed, role::full>;

    public:
        static constexpr execution_state state = execution_state::active_character_changed;
        static constexpr givm::role access_role = Role;
        static constexpr bool accepts_input = false;
        using base::character;

    private:
        friend class executor;
        constexpr explicit execution_view(executor& owner
#ifndef NDEBUG
            , std::size_t version
#endif
        ) noexcept : base{ owner
#ifndef NDEBUG
            , version
#endif
        } {}
    };
}

#endif
