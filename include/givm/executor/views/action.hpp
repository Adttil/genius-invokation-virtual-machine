#ifndef GIVM_EXECUTOR_VIEWS_ACTION_HPP
#define GIVM_EXECUTOR_VIEWS_ACTION_HPP

#include <span>
#include <tuple>
#include <utility>

#include "../executor.hpp"
#include "../instructions/begin_action.hpp"

namespace givm
{
    template<>
    class execution_view<execution_state::action>
    {
    public:
        constexpr std::span<const cost_of_switch> costs() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<
                cost_of_switch[],
                onpay_item<cost_of_switch>[],
                stack_count_t,
                action_argument,
                detail::action_request,
                detail::stage_t
            >());
        }

        constexpr void request_cost(stack_count_t index) const noexcept
        {
            get<0>(stack_->top<detail::action_request, detail::stage_t>()) = {
                .request_kind = detail::action_request_kind::calculate_cost,
                .action_kind = detail::action_kind::switch_active,
                .action_index = index
            };
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
            get<0>(stack_->top<detail::action_request, detail::stage_t>()) = {
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
            auto&& [stored_argument, request, stage] =
                stack_->top<action_argument, detail::action_request, detail::stage_t>();
            (void)stage;
            stored_argument = argument;
            request = {
                .request_kind = kind,
                .action_kind = detail::action_kind::switch_active,
                .action_index = index
            };
        }
    };
}

#endif
