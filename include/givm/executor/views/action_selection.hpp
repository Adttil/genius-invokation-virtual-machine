#ifndef GIVM_EXECUTOR_VIEWS_ACTION_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_ACTION_SELECTION_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>
#include <utility>

#include "../executor.hpp"
#include "../commands/begin_action.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    enum class switch_payment_validation : std::uint8_t
    {
        valid,
        requirement_mismatch,
        insufficient_dice
    };

    enum class card_payment_validation : std::uint8_t
    {
        valid,
        requirement_mismatch,
        insufficient_dice
    };

    template<>
    class execution_view<execution_state::action_selection>
    {
    public:
        constexpr std::size_t switch_target_count() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<
                cost_of_switch[],
                onpay_item<cost_of_switch>[],
                stack_count_t,
                detail::action_selection
            >()).size();
        }

        constexpr const cost_of_switch& switch_cost(std::size_t target_index) const noexcept
        {
            return get<0>(std::as_const(*stack_).top<
                cost_of_switch[],
                onpay_item<cost_of_switch>[],
                stack_count_t,
                detail::action_selection
            >())[target_index];
        }

        constexpr character_id switch_target(std::size_t target_index) const noexcept
        {
            return switch_cost(target_index).target;
        }

        const cost_of_switch& calculate_switch_cost(
            const definition_library& library, const table& card_table,
            std::size_t target_index
        ) const
        {
            return detail::calculate_switch_cost(
                library, target_index, card_table, *stack_
            );
        }

        constexpr switch_payment_validation switch_payment_validate(
            const table& card_table, std::size_t target_index, const dice_counts& paid_dice
        ) const noexcept
        {
            const auto& cost = switch_cost(target_index);
            if(not payment_matches(cost.requirement.dice_requirement, paid_dice))
            {
                return switch_payment_validation::requirement_mismatch;
            }
            const auto player = card_table.state().active_player;
            if(not card_table[player].state().dice.contains(paid_dice))
            {
                return switch_payment_validation::insufficient_dice;
            }
            return switch_payment_validation::valid;
        }

        constexpr void switch_active_character(std::size_t target_index, const dice_counts& paid_dice) const noexcept
        {
            // Assign the complete variant through its trivial assignment operator.
            get<0>(stack_->top<detail::action_selection>()) = detail::action_selection{
                detail::switch_selection{ .switch_cost_index = target_index, .paid_dice = paid_dice }
            };
        }

        void switch_active_character(
            const definition_library& library, const table& card_table,
            std::size_t target_index, const dice_counts& paid_dice
        ) const
        {
            detail::calculate_switch_cost(library, target_index, card_table, *stack_);
            // Assign the complete variant through its trivial assignment operator.
            get<0>(stack_->top<detail::action_selection>()) = detail::action_selection{
                detail::switch_selection{ .switch_cost_index = target_index, .paid_dice = paid_dice }
            };
        }

        constexpr std::size_t card_count() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<
                cost_of_card[], onpay_item<cost_of_card>[],
                detail::switch_handler_id[],
                cost_of_switch[], onpay_item<cost_of_switch>[],
                stack_count_t, detail::action_selection
            >()).size();
        }

        constexpr const cost_of_card& card_cost(std::size_t card_index) const noexcept
        {
            return get<0>(std::as_const(*stack_).top<
                cost_of_card[], onpay_item<cost_of_card>[],
                detail::switch_handler_id[],
                cost_of_switch[], onpay_item<cost_of_switch>[],
                stack_count_t, detail::action_selection
            >())[card_index];
        }

        constexpr hand_card_id card_id(std::size_t card_index) const noexcept
        {
            return card_cost(card_index).card;
        }

        const cost_of_card& calculate_card_cost(
            const definition_library& library, const table& card_table, std::size_t card_index
        ) const
        {
            return detail::calculate_card_cost(library, card_index, card_table, *stack_);
        }

        constexpr card_payment_validation card_payment_validate(
            const table& card_table, std::size_t card_index, const dice_counts& paid_dice
        ) const noexcept
        {
            const auto& cost = card_cost(card_index);
            if(not payment_matches(cost.requirement.dice_requirement, paid_dice))
            {
                return card_payment_validation::requirement_mismatch;
            }
            if(not card_table[cost.card.player_id].state().dice.contains(paid_dice))
            {
                return card_payment_validation::insufficient_dice;
            }
            return card_payment_validation::valid;
        }

        target_validation card_targets_validate(
            const definition_library& library, const table& card_table,
            std::size_t card_index, std::span<const card_target_id> targets = {}
        ) const
        {
            const auto id = card_id(card_index);
            const auto entity = card_table[id];
            const auto definition = library[entity.definition_id()];
            std::array<card_target_id, 2> selected_targets{};
            const auto target_count = std::min(targets.size(), selected_targets.size());
            for(std::size_t index = 0; index < target_count; ++index)
            {
                selected_targets[index] = targets[index];
            }
            return definition.query(card_target_validation{
                .card = entity, .table = card_table, .targets = selected_targets, .target_count = target_count
            });
        }

        constexpr void play_card(
            std::size_t card_index, const dice_counts& paid_dice, std::span<const card_target_id> targets = {}
        ) const noexcept
        {
            std::array<card_target_id, 2> selected_targets{};
            const auto target_count = std::min(targets.size(), selected_targets.size());
            for(std::size_t index = 0; index < target_count; ++index)
            {
                selected_targets[index] = targets[index];
            }
            get<0>(stack_->top<detail::action_selection>()) = detail::action_selection{
                detail::card_selection{
                    .card_cost_index = card_index, .targets = selected_targets, .paid_dice = paid_dice
                }
            };
        }

        void play_card(
            const definition_library& library, const table& card_table,
            std::size_t card_index, const dice_counts& paid_dice, std::span<const card_target_id> targets = {}
        ) const
        {
            detail::calculate_card_cost(library, card_index, card_table, *stack_);
            play_card(card_index, paid_dice, targets);
        }

        constexpr void declare_round_end() const noexcept
        {
            get<0>(stack_->top<detail::action_selection>()).emplace<detail::round_end_selection>();
        }

    private:
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;

        static constexpr bool payment_matches(
            const elemental_dice_requirement& requirement, const dice_counts& paid
        ) noexcept
        {
            const std::uint32_t required_total =
                requirement.fixed.total() + requirement.same + requirement.any;
            if(paid.total() != required_total)
            {
                return false;
            }

            std::uint32_t required_omni = requirement.fixed[elemental_dice::omni];
            std::uint32_t largest_remaining_group = 0;
            for(std::uint8_t index = 1; index < 8; ++index)
            {
                const auto dice = static_cast<elemental_dice>(index);
                const std::uint32_t fixed = requirement.fixed[dice];
                const std::uint32_t count = paid[dice];
                if(count < fixed)
                {
                    required_omni += fixed - count;
                }
                else
                {
                    const auto remaining = count - fixed;
                    if(remaining > largest_remaining_group)
                    {
                        largest_remaining_group = remaining;
                    }
                }
            }

            const std::uint32_t omni = paid[elemental_dice::omni];
            return omni >= required_omni
                && largest_remaining_group + (omni - required_omni) >= requirement.same;
        }
    };
}

#include "../../macro_undef.hpp"

#endif
