#ifndef GIVM_EXECUTOR_VIEWS_DICE_REROLL_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_DICE_REROLL_SELECTION_HPP

#include <cstdint>
#include <utility>

#include "../executor.hpp"
#include "../command_input_error.hpp"

namespace givm::detail
{
    struct dice_reroll_lane
    {
        std::uint32_t remaining = 0;
        std::uint32_t cursor = 0;
    };

    struct single_player_dice_reroll
    {
        player_id player;
        dice_reroll_lane lane;
    };
}

namespace givm
{
    template<execution_state State>
    requires (State == execution_state::dice_reroll_selection0 || State == execution_state::dice_reroll_selection1)
    class execution_view<State, role::full>
    {
    public:
        static constexpr execution_state state = State;
        static constexpr givm::role access_role = role::full;
        static constexpr bool accepts_input = true;

        static constexpr player_id player() noexcept
        {
            return player_id{ (static_cast<unsigned>(State) & 1u) };
        }

        constexpr bool selection_validate(const table& card_table, const dice_counts& selected) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<State>(version_);
#endif
            return card_table[player()].state().dice.contains(selected);
        }

        template<class TRandom>
        execution_state select(
            const definition_library& library, table& card_table, TRandom& random,
            const dice_counts& selected
        ) const
        {
#ifndef NDEBUG
            executor_->validate_view<State>(version_);
            if(not selection_validate(card_table, selected))
                throw view_input_error{ "dice_reroll_selection.select", insufficient_dice_argument{ player(), selected, card_table[player()].state().dice } };
#endif
            get<0>(executor_->context_.stack().top<dice_counts>()) = selected;
            return executor_->advance(library, card_table, random);
        }

    private:
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
    };

    template<execution_state State, role Role>
    requires ((State == execution_state::dice_reroll_selection0 || State == execution_state::dice_reroll_selection1) && Role != role::full && detail::accepts_input<State, Role>)
    class execution_view<State, Role> : private execution_view<State, role::full>
    {
        using base = execution_view<State, role::full>;

    public:
        static constexpr execution_state state = State;
        static constexpr givm::role access_role = Role;
        static constexpr bool accepts_input = true;
        using base::player;
        using base::selection_validate;
        using base::select;

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

    template<execution_state State>
    requires (State == execution_state::dice_reroll_selection0 || State == execution_state::dice_reroll_selection1)
    class execution_view<State, role::spectator> : private execution_view<State, role::full>
    {
        using base = execution_view<State, role::full>;

    public:
        static constexpr execution_state state = State;
        static constexpr givm::role access_role = role::spectator;
        static constexpr bool accepts_input = false;
        using base::player;
        using base::selection_validate;

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

    template<execution_state State, role Role>
    requires ((State == execution_state::dice_reroll_selection0 || State == execution_state::dice_reroll_selection1) && detail::player_role<Role> && !detail::accepts_input<State, Role>)
    class execution_view<State, Role> : private execution_view<State, role::full>
    {
        using base = execution_view<State, role::full>;

    public:
        static constexpr execution_state state = State;
        static constexpr givm::role access_role = Role;
        static constexpr bool accepts_input = false;
        using base::player;

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
