#ifndef GIVM_EXECUTOR_INSTRUCTIONS_BEGIN_ACTION_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_BEGIN_ACTION_HPP

#include "../executor.hpp"

#include <cstdint>
#include <utility>
#include <variant>

#include "../broadcast.hpp"
#include "../events.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    enum class action_request_kind : std::uint8_t
    {
        none,
        calculate_cost,
        do_action_with_cost,
        do_action
    };

    enum class action_kind : std::uint8_t
    {
        switch_active,
        declare_round_end
    };

    struct action_request
    {
        action_request_kind request_kind = action_request_kind::none;
        action_kind action_kind = action_kind::switch_active;
        stack_count_t action_index = 0;
    };

    enum class action_target_kind : std::uint8_t
    {
        none,
        character,
        support,
        summon
    };

    struct action_target
    {
        action_target_kind kind = action_target_kind::none;
        character_id character{};
        support_id support{};
        summon_id summon{};
    };

    struct action_argument
    {
        dice_counts paid_dice;
        action_target target;
    };

    struct begin_action
    {
        using context_type = void;

        enum class stage_type : stage_t
        {
            prepare_action_phase,
            action_phase_broadcast,
            before_action,
            before_action_with_switch,
            before_action_broadcast,
            prepare_input,
            wait_input,
            switch_onpay,
            after_fixed_switch_onpay,
            switch_payment_broadcast,
            switch_action,
            switch_action_broadcast,
            first_round_end_broadcast,
            second_round_end_broadcast
        };

        bool execute(card_table& table, execution_context& context, random_fn& random) const
        {
            const auto stage = static_cast<stage_type>(context.current_stage());
            switch(stage)
            {
            case stage_type::prepare_action_phase:
                return prepare_action_phase(table, context);
            case stage_type::action_phase_broadcast:
                return broadcast_action_phase(table, context, random);
            case stage_type::before_action:
            case stage_type::before_action_with_switch:
                return execute_before_action(table, context);
            case stage_type::before_action_broadcast:
                return broadcast_before_action(table, context, random);
            case stage_type::prepare_input:
                return prepare_input_frame(table, context);
            case stage_type::wait_input:
                return wait_input(table, context, random);
            case stage_type::switch_onpay:
                return continue_switch_onpay(table, context);
            case stage_type::after_fixed_switch_onpay:
                context.stack().pop<
                    detail::handler_id<cost_of_switch>,
                    cost_effect_argument<cost_of_switch>,
                    stage_t
                >();
                return continue_switch_onpay(table, context);
            case stage_type::switch_payment_broadcast:
                return broadcast_switch_payment(table, context, random);
            case stage_type::switch_action:
                return execute_switch_action(table, context);
            case stage_type::switch_action_broadcast:
                return broadcast_switch_action(table, context, random);
            case stage_type::first_round_end_broadcast:
            case stage_type::second_round_end_broadcast:
                return broadcast_round_end_declaration(table, context, random, stage);
            }

            GIVM_ASSERT(false);
            return false;
        }

    private:
        using switch_handler_id = detail::handler_id<cost_of_switch>;

        static auto action_frame(execution_context& context) noexcept
        {
            return context.stack().top<
                switch_handler_id[],
                cost_of_switch[],
                onpay_item<cost_of_switch>[],
                stack_count_t,
                action_argument,
                action_request,
                stage_t
            >();
        }

        static void pop_action_frame(execution_context& context) noexcept
        {
            context.stack().pop<
                switch_handler_id[],
                cost_of_switch[],
                onpay_item<cost_of_switch>[],
                stack_count_t,
                action_argument,
                action_request,
                stage_t
            >();
        }

        static cost_of_switch default_switch_cost(character_id target) noexcept
        {
            action_cost_requirement requirement;
            requirement.dice_requirement.any = 1;
            requirement.speed = action_speed::combat;
            return {
                .target = target,
                .requirement = requirement
            };
        }

        bool prepare_action_phase(card_table& table, execution_context& context) const
        {
            detail::prepare_broadcast(action_phase_started{}, table, context.stack());
            context.current_stage() = static_cast<stage_t>(stage_type::action_phase_broadcast);
            return true;
        }

        bool broadcast_action_phase(
            card_table& table,
            execution_context& context,
            random_fn& random
        ) const
        {
            if(not detail::continue_broadcast<action_phase_started>(table, context, random))
            {
                return true;
            }

            detail::pop_broadcast<action_phase_started>(context);
            context.current_stage() = static_cast<stage_t>(stage_type::before_action);
            return true;
        }

        static void calculate_switch_cost(
            stack_count_t action_index,
            card_table& table,
            execution_context& context,
            random_fn& random
        )
        {
            auto&& [
                handlers,
                costs,
                onpay_items,
                onpay_cursor,
                argument,
                request,
                stage
            ] = action_frame(context);
            (void)onpay_cursor;
            (void)argument;
            (void)request;
            (void)stage;

            GIVM_ASSERT(action_index < costs.size());
            auto& cost = costs[action_index];
            cost = default_switch_cost(cost.target);

            const auto handler_count = static_cast<stack_count_t>(handlers.size());
            const auto row_begin = action_index * handler_count;
            for(stack_count_t column = 0; column < handler_count; ++column)
            {
                auto& onpay = onpay_items[row_begin + column];
                onpay = {};
                cost.effect_argument = {};

                const auto entry = std::visit([&](auto handler_id)
                {
                    return detail::try_handle(
                        std::as_const(table)[handler_id],
                        cost,
                        table,
                        random
                    );
                }, handlers[column]);
                onpay = {
                    .entry = entry,
                    .argument = cost.effect_argument
                };
            }
        }

        bool execute_before_action(card_table& table, execution_context& context) const
        {
            auto& state = table.state();
            if(
                static_cast<stage_type>(context.current_stage()) == stage_type::before_action_with_switch
                && not state.first_ended
            )
            {
                state.active_player = other_player(state.active_player);
            }

            detail::prepare_broadcast(before_action{}, table, context.stack());
            context.current_stage() = static_cast<stage_t>(stage_type::before_action_broadcast);
            return true;
        }

        bool broadcast_before_action(card_table& table, execution_context& context, random_fn& random) const
        {
            if(not detail::continue_broadcast<before_action>(table, context, random))
            {
                return true;
            }

            detail::pop_broadcast<before_action>(context);
            context.current_stage() = static_cast<stage_t>(stage_type::prepare_input);
            return true;
        }

        bool prepare_input_frame(card_table& table, execution_context& context) const
        {
            const auto player = table[table.state().active_player];
            const auto active_character = player.state().active_character;
            GIVM_ASSERT(active_character.has_value());

            const auto is_switch_target = [active = *active_character](const auto& character)
            {
                return character.id() != active && character.state().health != 0;
            };

            stack_count_t switch_count = 0;
            for(auto character : player.characters())
            {
                if(is_switch_target(character))
                {
                    ++switch_count;
                }
            }

            const auto cost_handlers = detail::collect_all_broadcast_targets<cost_of_switch>(table);
            const auto handler_count = static_cast<stack_count_t>(cost_handlers.size());
            const auto matrix_size = switch_count * handler_count;
            auto&& [
                handlers,
                costs,
                onpay_items,
                onpay_cursor,
                argument,
                request,
                stage
            ] = context.stack().push(
                dynamic_array<switch_handler_id>(cost_handlers),
                dynamic_array<cost_of_switch>(switch_count),
                dynamic_array<onpay_item<cost_of_switch>>(matrix_size),
                stack_count_t{},
                action_argument{},
                action_request{},
                static_cast<stage_t>(stage_type::wait_input)
            );
            (void)handlers;
            (void)onpay_cursor;
            (void)argument;
            (void)request;
            (void)stage;

            for(auto& item : onpay_items)
            {
                item = {};
            }

            stack_count_t index = 0;
            for(auto character : player.characters())
            {
                if(not is_switch_target(character))
                {
                    continue;
                }
                costs[index++] = default_switch_cost(character.id());
            }

            return context.yield();
        }

        bool wait_input(card_table& table, execution_context& context, random_fn& random) const
        {
            auto&& [
                handlers,
                costs,
                onpay_items,
                onpay_cursor,
                argument,
                request,
                stage
            ] = action_frame(context);
            (void)handlers;
            (void)onpay_items;
            (void)onpay_cursor;
            (void)argument;
            (void)stage;

            if(request.request_kind == action_request_kind::none)
            {
                return context.yield();
            }

            if(request.action_kind == action_kind::declare_round_end)
            {
                pop_action_frame(context);

                auto& state = table.state();
                const bool is_first = not state.first_ended;
                if(is_first)
                {
                    state.first_ended = true;
                }

                detail::prepare_broadcast(round_end_declared{}, table, context.stack());
                context.current_stage() = static_cast<stage_t>(
                    is_first
                        ? stage_type::first_round_end_broadcast
                        : stage_type::second_round_end_broadcast
                );
                return true;
            }

            GIVM_ASSERT(request.action_kind == action_kind::switch_active);
            GIVM_ASSERT(request.action_index < costs.size());

            const auto action_index = request.action_index;
            if(request.request_kind == action_request_kind::calculate_cost)
            {
                calculate_switch_cost(action_index, table, context, random);
                request = {};
                return context.yield();
            }

            if(request.request_kind == action_request_kind::do_action)
            {
                calculate_switch_cost(action_index, table, context, random);
            }
            else
            {
                GIVM_ASSERT(request.request_kind == action_request_kind::do_action_with_cost);
            }

            onpay_cursor = 0;
            stage = static_cast<stage_t>(stage_type::switch_onpay);
            return continue_switch_onpay(table, context);
        }

        bool continue_switch_onpay(card_table& table, execution_context& context) const
        {
            auto&& [
                handlers,
                costs,
                onpay_items,
                onpay_cursor,
                argument,
                request,
                stage
            ] = action_frame(context);
            (void)costs;
            (void)argument;
            (void)stage;

            GIVM_ASSERT(request.action_index < costs.size());
            const auto handler_count = static_cast<stack_count_t>(handlers.size());
            const auto row_begin = request.action_index * handler_count;
            while(onpay_cursor < handler_count)
            {
                const auto column = onpay_cursor++;
                const auto onpay = onpay_items[row_begin + column];

                if(onpay.entry)
                {
                    const auto handler = handlers[column];
                    context.stack().push(
                        handler,
                        onpay.argument,
                        static_cast<stage_t>(stage_type::after_fixed_switch_onpay)
                    );
                    return context.enter(onpay.entry);
                }
            }

            return pay_switch_cost(table, context);
        }

        bool pay_switch_cost(card_table& table, execution_context& context) const
        {
            auto&& [
                handlers,
                costs,
                onpay_items,
                onpay_cursor,
                argument,
                request,
                stage
            ] = action_frame(context);
            (void)handlers;
            (void)costs;
            (void)onpay_items;
            (void)onpay_cursor;
            (void)request;

            const auto paid_dice = argument.paid_dice;
            const auto player = table.state().active_player;
            auto& player_dice = table[player].state().dice;
            GIVM_ASSERT(player_dice.contains(paid_dice));
            player_dice -= paid_dice;
            stage = static_cast<stage_t>(stage_type::switch_action);

            if(paid_dice.total() == 0)
            {
                return execute_switch_action(table, context);
            }

            detail::prepare_broadcast(
                dice_removed{
                    .player = player,
                    .dice = paid_dice
                },
                table,
                context.stack()
            );
            context.current_stage() = static_cast<stage_t>(stage_type::switch_payment_broadcast);
            return true;
        }

        bool broadcast_switch_payment(
            card_table& table,
            execution_context& context,
            random_fn& random
        ) const
        {
            if(not detail::continue_broadcast<dice_removed>(table, context, random))
            {
                return true;
            }

            detail::pop_broadcast<dice_removed>(context);
            return execute_switch_action(table, context);
        }

        bool execute_switch_action(card_table& table, execution_context& context) const
        {
            auto&& [
                handlers,
                costs,
                onpay_items,
                onpay_cursor,
                argument,
                request,
                stage
            ] = action_frame(context);
            (void)handlers;
            (void)onpay_items;
            (void)onpay_cursor;
            (void)argument;
            (void)stage;

            GIVM_ASSERT(request.action_index < costs.size());
            const auto target = costs[request.action_index].target;
            GIVM_ASSERT(static_cast<bool>(table[target]));
            table[target.player_id].state().active_character = target;

            detail::prepare_broadcast(active_character_changed{ .current = target }, table, context.stack());
            context.current_stage() = static_cast<stage_t>(stage_type::switch_action_broadcast);
            return true;
        }

        bool broadcast_switch_action(
            card_table& table,
            execution_context& context,
            random_fn& random
        ) const
        {
            if(not detail::continue_broadcast<active_character_changed>(table, context, random))
            {
                return true;
            }

            detail::pop_broadcast<active_character_changed>(context);
            return finish_switch_action(context);
        }

        bool finish_switch_action(execution_context& context) const
        {
            auto&& [
                handlers,
                costs,
                onpay_items,
                onpay_cursor,
                argument,
                request,
                stage
            ] = action_frame(context);
            (void)handlers;
            (void)onpay_items;
            (void)onpay_cursor;
            (void)argument;
            (void)stage;

            GIVM_ASSERT(request.action_index < costs.size());
            const auto speed = costs[request.action_index].requirement.speed;
            pop_action_frame(context);

            const auto next_stage = speed == action_speed::combat
                ? stage_type::before_action_with_switch
                : stage_type::before_action;
            context.current_stage() = static_cast<stage_t>(next_stage);
            return true;
        }

        bool broadcast_round_end_declaration(
            card_table& table,
            execution_context& context,
            random_fn& random,
            stage_type stage
        ) const
        {
            if(not detail::continue_broadcast<round_end_declared>(table, context, random))
            {
                return true;
            }

            detail::pop_broadcast<round_end_declared>(context);
            if(stage == stage_type::second_round_end_broadcast)
            {
                return context.enter_next();
            }

            table.state().active_player = other_player(table.state().active_player);
            context.current_stage() = static_cast<stage_t>(stage_type::before_action);
            return true;
        }
    };
}

#include "../../macro_undef.hpp"
#endif
