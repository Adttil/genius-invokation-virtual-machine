#ifndef GIVM_EXECUTOR_VIEWS_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_SELECTION_HPP

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <utility>

#include "../executor.hpp"
#include "../instructions/start_dice_roll_phase.hpp"
#include "../selector.hpp"

namespace givm
{
    template<>
    class execution_view<execution_state::initial_card_selection>
    {
    public:
        constexpr void select(player_id player, std::bitset<selection_capacity> selected) const noexcept
        {
            get<0>(stack_->top<detail::selector, detail::stage_t>()) = {
                .player = player,
                .selected = selected
            };
        }

    private:
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;
    };

    template<>
    class execution_view<execution_state::card_selection>
    {
    public:
        constexpr player_id player() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<detail::selector, detail::stage_t>()).player;
        }

        constexpr std::bitset<selection_capacity> selected() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<detail::selector, detail::stage_t>()).selected;
        }

        constexpr void select(std::bitset<selection_capacity> selected) const noexcept
        {
            get<0>(stack_->top<detail::selector, detail::stage_t>()).selected = selected;
        }

    private:
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;
    };

    template<>
    class execution_view<execution_state::initial_active_character_selection>
    {
    public:
        constexpr void select(character_id character) const noexcept
        {
            get<0>(stack_->top<character_id, detail::stage_t>()) = character;
        }

    private:
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;
    };

    template<>
    class execution_view<execution_state::remaining_active_character_selection>
    {
    public:
        constexpr player_id player() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<character_id, detail::stage_t>()).player_id;
        }

        constexpr character_id selected() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<character_id, character_id, detail::stage_t>());
        }

        constexpr void select(std::size_t index) const noexcept
        {
            get<0>(stack_->top<character_id, detail::stage_t>()).index = index;
        }

    private:
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;
    };

    template<>
    class execution_view<execution_state::dice_selection>
    {
    public:
        constexpr player_id player() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<detail::selector, detail::stage_t>()).player;
        }

        constexpr std::bitset<selection_capacity> selected() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<detail::selector, detail::stage_t>()).selected;
        }

        constexpr std::uint32_t remaining() const noexcept
        {
            return remaining(player());
        }

        constexpr std::uint32_t remaining(player_id player) const noexcept
        {
            const auto& phase = get<0>(
                std::as_const(*stack_).top<detail::dice_reroll_phase, detail::selector, detail::stage_t>()
            );
            return player == player_id{ 0 } ? phase.first.remaining : phase.second.remaining;
        }

        constexpr std::uint32_t dice_count() const noexcept
        {
            return get<0>(
                std::as_const(*stack_).top<detail::dice_reroll_phase, detail::selector, detail::stage_t>()
            ).dice_count;
        }

        constexpr void select(std::bitset<selection_capacity> selected) const noexcept
        {
            get<0>(stack_->top<detail::selector, detail::stage_t>()).selected = selected;
        }

        constexpr void select(player_id player, std::bitset<selection_capacity> selected) const noexcept
        {
            get<0>(stack_->top<detail::selector, detail::stage_t>()) = {
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
