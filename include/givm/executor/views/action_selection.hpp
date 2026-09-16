#ifndef GIVM_EXECUTOR_VIEWS_ACTION_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_ACTION_SELECTION_HPP

#include <span>
#include <tuple>
#include <utility>

#include "../executor.hpp"
#include "../payment.hpp"
#include "../instructions/begin_action.hpp"

#include "../../macro_define.hpp"

namespace givm
{
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
            return detail::begin_action_command::calculate_switch_cost(
                library, switch_cost_index(target), card_table, *stack_
            );
        }

        constexpr payment_check_result check_switch_payment(
            const table& card_table, character_id target, const dice_counts& paid_dice
        ) const noexcept
        {
            const auto index = switch_cost_index(target);
            const auto available_costs = switch_costs();
            if(not detail::payment_matches(available_costs[index].requirement.dice_requirement, paid_dice))
            {
                return payment_check_result::requirement_mismatch;
            }
            const auto player = card_table.state().active_player;
            if(not card_table[player].state().dice.contains(paid_dice))
            {
                return payment_check_result::insufficient_dice;
            }
            return payment_check_result::valid;
        }

        constexpr void switch_active_character(character_id target, const dice_counts& paid_dice) const noexcept
        {
            submit_switch(switch_cost_index(target), paid_dice);
        }

        void switch_active_character(
            const definition_library& library, const table& card_table,
            character_id target, const dice_counts& paid_dice
        ) const
        {
            const auto index = switch_cost_index(target);
            detail::begin_action_command::calculate_switch_cost(library, index, card_table, *stack_);
            submit_switch(index, paid_dice);
        }

        constexpr void declare_round_end() const noexcept
        {
            get<0>(stack_->top<detail::action_selection>()).emplace<detail::round_end_selection>();
        }

    private:
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;

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

        constexpr void submit_switch(stack_count_t index, const dice_counts& paid_dice) const noexcept
        {
            // Assign the complete variant through its trivial assignment operator.
            get<0>(stack_->top<detail::action_selection>()) = detail::action_selection{
                detail::switch_selection{ .switch_cost_index = index, .paid_dice = paid_dice }
            };
        }
    };
}

#include "../../macro_undef.hpp"

#endif
