#ifndef GIVM_EXECUTOR_VIEWS_DICE_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_DICE_SELECTION_HPP

#include <cstdint>
#include <utility>

#include "../executor.hpp"
#include "../commands/start_dice_roll_phase.hpp"

namespace givm
{
    enum class dice_selection_check_result : std::uint8_t
    {
        valid,
        invalid_player,
        no_rerolls_remaining,
        insufficient_dice
    };

    template<>
    class execution_view<execution_state::dice_selection>
    {
    public:
        constexpr player_id player() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<detail::dice_selector>()).player;
        }

        constexpr dice_counts selected() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<detail::dice_selector>()).selected;
        }

        constexpr std::uint32_t remaining() const noexcept
        {
            return remaining(player());
        }

        constexpr std::uint32_t remaining(player_id player) const noexcept
        {
            const auto& phase = get<0>(
                std::as_const(*stack_).top<detail::dice_reroll_phase, detail::dice_selector>()
            );
            return player == player_id{ 0 } ? phase.first.remaining : phase.second.remaining;
        }

        constexpr std::uint32_t dice_count() const noexcept
        {
            return get<0>(
                std::as_const(*stack_).top<detail::dice_reroll_phase, detail::dice_selector>()
            ).dice_count;
        }

        constexpr bool check_selection(
            const table& card_table, const dice_counts& selected
        ) const noexcept
        {
            return card_table[player()].state().dice.contains(selected);
        }

        constexpr dice_selection_check_result check_selection(
            const table& card_table, player_id player, const dice_counts& selected
        ) const noexcept
        {
            if(player.index >= 2)
            {
                return dice_selection_check_result::invalid_player;
            }
            if(remaining(player) == 0)
            {
                return dice_selection_check_result::no_rerolls_remaining;
            }
            if(not card_table[player].state().dice.contains(selected))
            {
                return dice_selection_check_result::insufficient_dice;
            }
            return dice_selection_check_result::valid;
        }

        constexpr void select(const dice_counts& selected) const noexcept
        {
            get<0>(stack_->top<detail::dice_selector>()).selected = selected;
        }

        constexpr void select(player_id player, const dice_counts& selected) const noexcept
        {
            get<0>(stack_->top<detail::dice_selector>()) = {
                .player = player,
                .selected = selected
            };
        }

    private:
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;
    };
}

#endif
