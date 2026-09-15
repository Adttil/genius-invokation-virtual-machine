#ifndef GIVM_EXECUTOR_VIEWS_ACTION_HPP
#define GIVM_EXECUTOR_VIEWS_ACTION_HPP

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
        constexpr std::span<const cost_of_switch> costs() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<
                cost_of_switch[],
                onpay_item<cost_of_switch>[],
                stack_count_t,
                action_argument,
                detail::action_request
            >());
        }

        template<class TRandom>
        const cost_of_switch& calculate_cost(
            const definition_library& library, const table& card_table,
            stack_count_t index, TRandom& random_source
        ) const
        {
            random_fn random{ random_source };
            return detail::begin_action_command::calculate_switch_cost(library, index, card_table, *stack_, random);
        }

        constexpr payment_check_result check_payment(
            const table& card_table, stack_count_t index, const dice_counts& paid_dice
        ) const noexcept
        {
            const auto available_costs = costs();
            GIVM_ASSERT(index < available_costs.size());
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

        constexpr void execute_action(stack_count_t index, const action_argument& argument) const noexcept
        {
            submit_action(index, argument, detail::action_request_kind::do_action);
        }

        constexpr void execute_action_with_cost(stack_count_t index, const action_argument& argument) const noexcept
        {
            submit_action(index, argument, detail::action_request_kind::do_action_with_cost);
        }

        constexpr void declare_round_end() const noexcept
        {
            get<0>(stack_->top<detail::action_request>()) = {
                .request_kind = detail::action_request_kind::do_action,
                .action_kind = detail::action_kind::declare_round_end
            };
        }

    private:
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;

        constexpr void submit_action(
            stack_count_t index,
            const action_argument& argument,
            detail::action_request_kind kind
        ) const noexcept
        {
            auto&& [stored_argument, request] =
                stack_->top<action_argument, detail::action_request>();
            stored_argument = argument;
            request = {
                .request_kind = kind,
                .action_kind = detail::action_kind::switch_active,
                .action_index = index
            };
        }
    };
}

#include "../../macro_undef.hpp"

#endif
