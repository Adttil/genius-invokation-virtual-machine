#ifndef GIVM_EXECUTOR_VIEWS_DICE_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_DICE_SELECTION_HPP

#include <cstdint>
#include <utility>

#include "../executor.hpp"
#include "../command_input_error.hpp"
#include "../commands/start_dice_roll_phase.hpp"

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

    template<>
    class execution_view<execution_state::dice_selection>
    {
    public:
        constexpr player_id player() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::dice_selection>(version_);
#endif
            return get<0>(std::as_const(executor_->context_.stack()).top<detail::dice_selector>()).player;
        }

        constexpr dice_counts selected() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::dice_selection>(version_);
#endif
            return get<0>(std::as_const(executor_->context_.stack()).top<detail::dice_selector>()).selected;
        }

        constexpr std::uint32_t remaining() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::dice_selection>(version_);
#endif
            return remaining(player());
        }

        constexpr std::uint32_t remaining(player_id player) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::dice_selection>(version_);
            if(player.index >= 2)
                throw view_input_error{ "dice_selection.remaining", view_index_out_of_range{ "player", player.index, 2 } };
#endif
            const auto& phase = get<0>(
                std::as_const(executor_->context_.stack()).top<detail::dice_reroll_phase, detail::dice_selector>()
            );
            return player == player_id{ 0 } ? phase.first.remaining : phase.second.remaining;
        }

        constexpr std::uint32_t dice_count() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::dice_selection>(version_);
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
            executor_->validate_view<execution_state::dice_selection>(version_);
#endif
            return card_table[player()].state().dice.contains(selected);
        }

        constexpr dice_selection_validation selection_validate(
            const table& card_table, player_id player, const dice_counts& selected
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::dice_selection>(version_);
#endif
            if(player.index >= 2)
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
        execution_state select(
            const definition_library& library, table& card_table, TRandom& random,
            const dice_counts& selected
        ) const
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::dice_selection>(version_);
            if(not selection_validate(card_table, selected))
                throw view_input_error{ "dice_selection.select", insufficient_dice_argument{ player(), selected, card_table[player()].state().dice } };
#endif
            get<0>(executor_->context_.stack().top<detail::dice_selector>()).selected = selected;
            return executor_->advance(library, card_table, random);
        }

        template<class TRandom>
        execution_state select(
            const definition_library& library, table& card_table, TRandom& random,
            player_id player, const dice_counts& selected
        ) const
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::dice_selection>(version_);
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
}

#endif
