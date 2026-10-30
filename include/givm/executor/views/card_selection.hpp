#ifndef GIVM_EXECUTOR_VIEWS_CARD_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_CARD_SELECTION_HPP

#include <bitset>
#include <utility>

#include "../executor.hpp"

namespace givm
{
    struct invalid_card_positions
    {
        std::bitset<selection_capacity> selected;
        std::size_t card_count;
    };

    inline std::string error_string(const invalid_card_positions& error)
    {
        return "invalid card positions " + error.selected.to_string() + "; hand size " + std::to_string(error.card_count);
    }

    template<execution_state State>
    requires (State == execution_state::card_selection0 || State == execution_state::card_selection1)
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

        constexpr std::bitset<selection_capacity> selected() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<State>(version_);
#endif
            return get<0>(std::as_const(executor_->context_.stack()).top<std::bitset<selection_capacity>>());
        }

        constexpr bool selection_validate(
            const table& card_table, std::bitset<selection_capacity> selected
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<State>(version_);
#endif
            return (selected >> card_table[player()].hand_card_count()).none();
        }

        template<class TRandom>
        execution_state select(
            const definition_library& library, table& card_table, TRandom& random,
            std::bitset<selection_capacity> selected
        ) const
        {
#ifndef NDEBUG
            executor_->validate_view<State>(version_);
            if(not selection_validate(card_table, selected))
                throw view_input_error{ "card_selection.select", invalid_card_positions{ selected, card_table[player()].hand_card_count() } };
#endif
            get<0>(executor_->context_.stack().top<std::bitset<selection_capacity>>()) = selected;
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
    requires ((State == execution_state::card_selection0 || State == execution_state::card_selection1) && Role != role::full && detail::accepts_input<State, Role>)
    class execution_view<State, Role> : private execution_view<State, role::full>
    {
        using base = execution_view<State, role::full>;

    public:
        static constexpr execution_state state = State;
        static constexpr givm::role access_role = Role;
        static constexpr bool accepts_input = true;
        using base::player;
        using base::selection_validate;
        using base::selected;
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
    requires (State == execution_state::card_selection0 || State == execution_state::card_selection1)
    class execution_view<State, role::spectator> : private execution_view<State, role::full>
    {
        using base = execution_view<State, role::full>;

    public:
        static constexpr execution_state state = State;
        static constexpr givm::role access_role = role::spectator;
        static constexpr bool accepts_input = false;
        using base::player;
        using base::selection_validate;
        using base::selected;

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
    requires ((State == execution_state::card_selection0 || State == execution_state::card_selection1) && detail::player_role<Role> && !detail::accepts_input<State, Role>)
    class execution_view<State, Role> : private execution_view<State, role::full>
    {
        using base = execution_view<State, role::full>;

    public:
        static constexpr execution_state state = State;
        static constexpr givm::role access_role = Role;
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
}

#endif
