#ifndef GIVM_EXECUTOR_VIEWS_DICE_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_DICE_SELECTION_HPP

#include <cstdint>
#include <utility>

#include "../executor.hpp"
#include "../command_input_error.hpp"
#include "dice_reroll_selection.hpp"

namespace givm::detail
{
    struct dice_reroll_phase
    {
        std::uint32_t dice_count = 0;
        std::uint32_t player0_random_count = 0;
        dice_reroll_lane first;
        dice_reroll_lane second;
    };

    struct dice_selector
    {
        player_id player;
        dice_counts selected;
    };
}

namespace givm
{
    enum class dice_selection_validation : std::uint8_t
    {
        valid,
        invalid_player,
        no_rerolls_remaining,
        insufficient_dice
    };

    inline std::string error_string(dice_selection_validation value)
    {
        switch(value)
        {
        case dice_selection_validation::valid: return "valid";
        case dice_selection_validation::invalid_player: return "invalid player";
        case dice_selection_validation::no_rerolls_remaining: return "no rerolls remaining";
        case dice_selection_validation::insufficient_dice: return "insufficient dice";
        }
        return "unknown selection validation";
    }

    template<execution_state State>
    requires (State == execution_state::dice_selection_both || State == execution_state::dice_selection0 || State == execution_state::dice_selection1)
    class execution_view<State, role::full>
    {
    public:
        static constexpr execution_state state = State;
        static constexpr givm::role access_role = role::full;
        static constexpr bool accepts_input = true;

        constexpr player_id player() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<State>(version_);
#endif
            if constexpr(State != execution_state::dice_selection_both)
                return player_id{ (static_cast<unsigned>(State) & 1u) };
            else
                return get<0>(std::as_const(executor_->context_.stack()).top<detail::dice_selector>()).player;
        }

        constexpr dice_counts selected() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<State>(version_);
#endif
            return get<0>(std::as_const(executor_->context_.stack()).top<detail::dice_selector>()).selected;
        }

        constexpr std::uint32_t remaining() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<State>(version_);
#endif
            return remaining(player());
        }

        constexpr std::uint32_t remaining(player_id player) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<State>(version_);
            if(player.index() >= 2)
                throw view_input_error{ "dice_selection.remaining", view_index_out_of_range{ "player", player.index(), 2 } };
#endif
            const auto& phase = get<0>(
                std::as_const(executor_->context_.stack()).top<detail::dice_reroll_phase, detail::dice_selector>()
            );
            return player == player_id{ 0 } ? phase.first.remaining : phase.second.remaining;
        }

        constexpr std::uint32_t dice_count() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<State>(version_);
#endif
            return get<0>(
                std::as_const(executor_->context_.stack()).top<detail::dice_reroll_phase, detail::dice_selector>()
            ).dice_count;
        }

        constexpr bool selection_validate(
            const table& card_table, const dice_counts& selected
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<State>(version_);
#endif
            return card_table[player()].state().dice.contains(selected);
        }

        constexpr dice_selection_validation selection_validate(
            const table& card_table, player_id player, const dice_counts& selected
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<State>(version_);
#endif
            if(player.index() >= 2)
            {
                return dice_selection_validation::invalid_player;
            }
            if(remaining(player) == 0)
            {
                return dice_selection_validation::no_rerolls_remaining;
            }
            if(not card_table[player].state().dice.contains(selected))
            {
                return dice_selection_validation::insufficient_dice;
            }
            return dice_selection_validation::valid;
        }

        template<class TRandom>
        execution_state select(const definition_library& library, table& card_table, TRandom& random,
            const dice_counts& selected) const
        {
            return select(library, card_table, random, player(), selected);
        }

        template<class TRandom>
        execution_state select(
            const definition_library& library, table& card_table, TRandom& random,
            player_id player, const dice_counts& selected
        ) const
        {
#ifndef NDEBUG
            executor_->validate_view<State>(version_);
            const auto result = selection_validate(card_table, player, selected);
            if(result != dice_selection_validation::valid)
                throw view_input_error{ "dice_selection.select", result };
#endif
            get<0>(executor_->context_.stack().top<detail::dice_selector>()) = {
                .player = player,
                .selected = selected
            };
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
    requires ((State == execution_state::dice_selection_both || State == execution_state::dice_selection0 || State == execution_state::dice_selection1) && detail::player_role<Role> && detail::accepts_input<State, Role>)
    class execution_view<State, Role> : private execution_view<State, role::full>
    {
        using base = execution_view<State, role::full>;

    public:
        static constexpr execution_state state = State;
        static constexpr givm::role access_role = Role;
        static constexpr bool accepts_input = true;
        using base::selected;
        using base::dice_count;
        using base::remaining;

        constexpr player_id player() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            this->executor_->template validate_view<State>(this->version_);
#endif
            return player_id{ detail::role_player<Role> };
        }

        constexpr std::uint32_t remaining() const noexcept(detail::view_checks_disabled)
        {
            return base::remaining(player());
        }

        constexpr bool selection_validate(const table& card_table, const dice_counts& selected
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            this->executor_->template validate_view<State>(this->version_);
#endif
            return card_table[player_id{ detail::role_player<Role> }].state().dice.contains(selected);
        }

        template<class TRandom>
        execution_state select(const definition_library& library, table& card_table, TRandom& random,
            const dice_counts& selected) const
        {
            return base::select(library, card_table, random, player_id{ detail::role_player<Role> }, selected);
        }

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
    requires (State == execution_state::dice_selection_both || State == execution_state::dice_selection0 || State == execution_state::dice_selection1)
    class execution_view<State, role::spectator> : private execution_view<State, role::full>
    {
        using base = execution_view<State, role::full>;

    public:
        static constexpr execution_state state = State;
        static constexpr givm::role access_role = role::spectator;
        static constexpr bool accepts_input = false;
        using base::player;
        using base::selected;
        using base::remaining;
        using base::dice_count;
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
    requires ((State == execution_state::dice_selection_both || State == execution_state::dice_selection0 || State == execution_state::dice_selection1) && detail::player_role<Role> && !detail::accepts_input<State, Role>)
    class execution_view<State, Role> : private execution_view<State, role::full>
    {
        using base = execution_view<State, role::full>;

    public:
        static constexpr execution_state state = State;
        static constexpr givm::role access_role = Role;
        static constexpr bool accepts_input = false;
        using base::player;
        using base::remaining;
        using base::dice_count;

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
