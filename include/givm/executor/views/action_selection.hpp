#ifndef GIVM_EXECUTOR_VIEWS_ACTION_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_ACTION_SELECTION_HPP

#include <cstdint>
#include <span>
#include <tuple>
#include <utility>

#include "../executor.hpp"
#include "../commands/begin_action.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    enum class switch_payment_check_result : std::uint8_t
    {
        valid,
        requirement_mismatch,
        insufficient_dice
    };

    template<>
    class execution_view<execution_state::action_selection>
    {
    public:
        constexpr std::span<const cost_of_switch> switch_costs() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<
                cost_of_switch[],
                onpay_item<cost_of_switch>[],
                stack_count_t,
                detail::action_selection
            >());
        }

        const cost_of_switch& calculate_switch_cost(
            const definition_library& library, const table& card_table,
            character_id target
        ) const
        {
            return detail::calculate_switch_cost(
                library, switch_cost_index(target), card_table, *stack_
            );
        }

        constexpr switch_payment_check_result check_switch_payment(
            const table& card_table, character_id target, const dice_counts& paid_dice
        ) const noexcept
        {
            const auto index = switch_cost_index(target);
            const auto available_costs = switch_costs();
            if(not payment_matches(available_costs[index].requirement.dice_requirement, paid_dice))
            {
                return switch_payment_check_result::requirement_mismatch;
            }
            const auto player = card_table.state().active_player;
            if(not card_table[player].state().dice.contains(paid_dice))
            {
                return switch_payment_check_result::insufficient_dice;
            }
            return switch_payment_check_result::valid;
        }

        constexpr void switch_active_character(character_id target, const dice_counts& paid_dice) const noexcept
        {
            // Assign the complete variant through its trivial assignment operator.
            get<0>(stack_->top<detail::action_selection>()) = detail::action_selection{
                detail::switch_selection{ .switch_cost_index = switch_cost_index(target), .paid_dice = paid_dice }
            };
        }

        void switch_active_character(
            const definition_library& library, const table& card_table,
            character_id target, const dice_counts& paid_dice
        ) const
        {
            const auto index = switch_cost_index(target);
            detail::calculate_switch_cost(library, index, card_table, *stack_);
            // Assign the complete variant through its trivial assignment operator.
            get<0>(stack_->top<detail::action_selection>()) = detail::action_selection{
                detail::switch_selection{ .switch_cost_index = index, .paid_dice = paid_dice }
            };
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

        constexpr stack_count_t switch_cost_index(character_id target) const noexcept
        {
            const auto&& [indices, costs, onpay_items, onpay_cursor, selection] =
                std::as_const(*stack_).top<
                    stack_count_t[],
                    cost_of_switch[],
                    onpay_item<cost_of_switch>[],
                    stack_count_t,
                    detail::action_selection
                >();
            GIVM_ASSERT(target.index < indices.size());
            const auto index = indices[target.index];
            GIVM_ASSERT(index < costs.size());
            GIVM_ASSERT(costs[index].target == target);
            return index;
        }

    };
}

#include "../../macro_undef.hpp"

#endif
