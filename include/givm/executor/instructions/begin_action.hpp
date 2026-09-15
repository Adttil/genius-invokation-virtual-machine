#ifndef GIVM_EXECUTOR_INSTRUCTIONS_BEGIN_ACTION_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_BEGIN_ACTION_HPP

#include "../executor.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <variant>

#include "../broadcast.hpp"
#include "../../definition/events.hpp"
#include "../../definition/commands.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    namespace detail
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
    }

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
}

namespace givm::detail
{
    namespace begin_action_command
    {
        using switch_handler_id = handler_id<cost_of_switch>;

        // Local labels describe this command's pointer sequence only. They never
        // become runtime state or require a second dispatch after fetching the instruction.
        inline constexpr std::size_t prepare_action_phase_slot = 0;
        inline constexpr std::size_t action_phase_broadcast_slot = 1;
        inline constexpr std::size_t before_action_slot = 2;
        inline constexpr std::size_t before_action_with_switch_slot = 3;
        inline constexpr std::size_t before_action_broadcast_slot = 4;
        inline constexpr std::size_t wait_input_slot = 5;
        inline constexpr std::size_t switch_onpay_slot = 6;
        inline constexpr std::size_t after_fixed_switch_onpay_slot = 7;
        inline constexpr std::size_t switch_payment_broadcast_slot = 8;
        inline constexpr std::size_t switch_action_slot = 9;
        inline constexpr std::size_t switch_action_apply_slot = 10;

        template<bool Observed>
        inline constexpr std::size_t switch_action_broadcast_slot = 10 + Observed;
        template<bool Observed>
        inline constexpr std::size_t first_round_end_broadcast_slot = 11 + Observed;
        template<bool Observed>
        inline constexpr std::size_t second_round_end_broadcast_slot = 12 + Observed;

        template<std::size_t From, std::size_t To>
        execution_state go_to(execution_context& context) noexcept
        {
            return context.jump(context.position() - From * sizeof(execute_fn) + To * sizeof(execute_fn));
        }

        inline auto action_frame(execution_context& context) noexcept
        {
            return context.stack().top<
                switch_handler_id[],
                cost_of_switch[],
                onpay_item<cost_of_switch>[],
                stack_count_t,
                action_argument,
                action_request
            >();
        }

        inline void pop_action_frame(execution_context& context) noexcept
        {
            context.stack().pop<
                switch_handler_id[],
                cost_of_switch[],
                onpay_item<cost_of_switch>[],
                stack_count_t,
                action_argument,
                action_request
            >();
        }

        inline cost_of_switch default_switch_cost(character_id target) noexcept
        {
            action_cost_requirement requirement;
            requirement.dice_requirement.any = 1;
            requirement.speed = action_speed::combat;
            return {
                .target = target,
                .requirement = requirement
            };
        }

        inline void calculate_switch_cost(
            const definition_library& library,
            stack_count_t action_index,
            unrestricted_table& table,
            execution_context& context,
            random_fn& random
        )
        {
            auto&& [handlers, costs, onpay_items, onpay_cursor, argument, request] = action_frame(context);
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
                const auto entry = std::visit([&](auto handler)
                {
                    return try_handle(library, std::as_const(table)[handler], cost, table, random);
                }, handlers[column]);
                onpay = { .entry = entry, .argument = cost.effect_argument };
            }
        }

        inline execution_state prepare_action_phase(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn&
        )
        {
            prepare_broadcast(library, action_phase_started{}, table, context.stack());
            return context.enter_next();
        }

        template<bool Observed>
        execution_state broadcast_action_phase(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            if(not continue_broadcast<action_phase_started>(library, table, context, random))
            {
                return continue_execution;
            }
            pop_broadcast<action_phase_started>(context);
            context.enter_next();
            if constexpr(Observed)
            {
                return context.yield(execution_state::action_started);
            }
            return continue_execution;
        }

        template<bool Switch, bool Observed>
        execution_state prepare_before_action(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn&
        )
        {
            if constexpr(Switch)
            {
                auto& state = table.state();
                if(not state.first_ended)
                {
                    state.active_player = other_player(state.active_player);
                }
            }
            prepare_broadcast(library, before_action{}, table, context.stack());
            constexpr auto from = Switch ? before_action_with_switch_slot : before_action_slot;
            go_to<from, before_action_broadcast_slot>(context);
            if constexpr(Switch && Observed)
            {
                return context.yield(execution_state::action_started);
            }
            return continue_execution;
        }

        inline execution_state prepare_input_frame(
            const definition_library& library, unrestricted_table& table, execution_context& context
        )
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

            const auto cost_handlers = collect_all_broadcast_targets<cost_of_switch>(library, table);
            const auto handler_count = static_cast<stack_count_t>(cost_handlers.size());
            const auto matrix_size = switch_count * handler_count;
            auto&& [handlers, costs, onpay_items, onpay_cursor, argument, request] = context.stack().push(
                dynamic_array<switch_handler_id>(cost_handlers),
                dynamic_array<cost_of_switch>(switch_count),
                dynamic_array<onpay_item<cost_of_switch>>(matrix_size),
                stack_count_t{},
                action_argument{},
                action_request{}
            );

            for(auto& item : onpay_items)
            {
                item = {};
            }
            stack_count_t index = 0;
            for(auto character : player.characters())
            {
                if(is_switch_target(character))
                {
                    costs[index++] = default_switch_cost(character.id());
                }
            }
            return context.yield(execution_state::action);
        }

        inline execution_state broadcast_before_action(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            if(not continue_broadcast<before_action>(library, table, context, random))
            {
                return continue_execution;
            }
            pop_broadcast<before_action>(context);
            context.enter_next();
            return prepare_input_frame(library, table, context);
        }

        template<bool Observed>
        execution_state wait_input(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            auto&& [handlers, costs, onpay_items, onpay_cursor, argument, request] = action_frame(context);
            if(request.request_kind == action_request_kind::none)
            {
                return context.yield(execution_state::action);
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
                prepare_broadcast(library, round_end_declared{}, table, context.stack());
                if(is_first)
                {
                    go_to<wait_input_slot, first_round_end_broadcast_slot<Observed>>(context);
                }
                else
                {
                    go_to<wait_input_slot, second_round_end_broadcast_slot<Observed>>(context);
                }
                if constexpr(Observed)
                {
                    return context.yield(execution_state::round_end_declared);
                }
                return continue_execution;
            }

            GIVM_ASSERT(request.action_kind == action_kind::switch_active);
            GIVM_ASSERT(request.action_index < costs.size());
            const auto action_index = request.action_index;
            if(request.request_kind == action_request_kind::calculate_cost)
            {
                calculate_switch_cost(library, action_index, table, context, random);
                request = {};
                return context.yield(execution_state::action);
            }
            if(request.request_kind == action_request_kind::do_action)
            {
                calculate_switch_cost(library, action_index, table, context, random);
            }
            else
            {
                GIVM_ASSERT(request.request_kind == action_request_kind::do_action_with_cost);
            }

            onpay_cursor = 0;
            return context.enter_next();
        }

        inline execution_state continue_switch_onpay(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn&
        )
        {
            auto&& [handlers, costs, onpay_items, onpay_cursor, argument, request] = action_frame(context);
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
                    context.stack().push(handler, onpay.argument);
                    context.enter_next();
                    return context.enter(onpay.entry);
                }
            }

            const auto paid_dice = argument.paid_dice;
            const auto player = table.state().active_player;
            auto& player_dice = table[player].state().dice;
            GIVM_ASSERT(player_dice.contains(paid_dice));
            player_dice -= paid_dice;
            if(paid_dice.total() == 0)
            {
                return go_to<switch_onpay_slot, switch_action_slot>(context);
            }

            prepare_broadcast(library, dice_removed{ .player = player, .dice = paid_dice }, table, context.stack());
            return go_to<switch_onpay_slot, switch_payment_broadcast_slot>(context);
        }

        inline execution_state after_fixed_switch_onpay(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            context.stack().pop<switch_handler_id, cost_effect_argument<cost_of_switch>>();
            go_to<after_fixed_switch_onpay_slot, switch_onpay_slot>(context);
            return continue_switch_onpay(library, table, context, random);
        }

        inline execution_state broadcast_switch_payment(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            if(not continue_broadcast<dice_removed>(library, table, context, random))
            {
                return continue_execution;
            }
            pop_broadcast<dice_removed>(context);
            return context.enter_next();
        }

        template<bool Observed>
        execution_state execute_switch_action(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn&
        )
        {
            auto&& [handlers, costs, onpay_items, onpay_cursor, argument, request] = action_frame(context);
            GIVM_ASSERT(request.action_index < costs.size());
            const auto target = costs[request.action_index].target;
            GIVM_ASSERT(static_cast<bool>(table[target]));
            if constexpr(not Observed)
            {
                table[target.player_id].state().active_character = target;
            }
            prepare_broadcast(library, active_character_changed{ .current = target }, table, context.stack());
            context.enter_next();
            if constexpr(Observed)
            {
                return context.yield(execution_state::active_character_changed);
            }
            return continue_execution;
        }

        template<bool Observed>
        execution_state broadcast_switch_action(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            if(not continue_broadcast<active_character_changed>(library, table, context, random))
            {
                return continue_execution;
            }
            pop_broadcast<active_character_changed>(context);

            auto&& [handlers, costs, onpay_items, onpay_cursor, argument, request] = action_frame(context);
            GIVM_ASSERT(request.action_index < costs.size());
            const auto speed = costs[request.action_index].requirement.speed;
            pop_action_frame(context);
            if(speed == action_speed::combat)
            {
                return go_to<switch_action_broadcast_slot<Observed>, before_action_with_switch_slot>(context);
            }
            return go_to<switch_action_broadcast_slot<Observed>, before_action_slot>(context);
        }

        inline execution_state apply_switch_action(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            auto&& [event, handler] = context.stack().top<
                active_character_changed,
                handler_id<active_character_changed>
            >();
            table[event.current.player_id].state().active_character = event.current;
            context.enter_next();
            return broadcast_switch_action<true>(library, table, context, random);
        }

        template<bool Observed>
        execution_state broadcast_first_round_end(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            if(not continue_broadcast<round_end_declared>(library, table, context, random))
            {
                return continue_execution;
            }
            pop_broadcast<round_end_declared>(context);
            table.state().active_player = other_player(table.state().active_player);
            go_to<first_round_end_broadcast_slot<Observed>, before_action_slot>(context);
            if constexpr(Observed)
            {
                return context.yield(execution_state::action_started);
            }
            return continue_execution;
        }

        inline execution_state broadcast_second_round_end(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            if(not continue_broadcast<round_end_declared>(library, table, context, random))
            {
                return continue_execution;
            }
            pop_broadcast<round_end_declared>(context);
            return context.enter_next();
        }

        template<bool Observed>
        void emit(program_writer& writer)
        {
            writer.write<execute_fn>(&prepare_action_phase);
            writer.write<execute_fn>(&broadcast_action_phase<Observed>);
            writer.write<execute_fn>(&prepare_before_action<false, Observed>);
            writer.write<execute_fn>(&prepare_before_action<true, Observed>);
            writer.write<execute_fn>(&broadcast_before_action);
            writer.write<execute_fn>(&wait_input<Observed>);
            writer.write<execute_fn>(&continue_switch_onpay);
            writer.write<execute_fn>(&after_fixed_switch_onpay);
            writer.write<execute_fn>(&broadcast_switch_payment);
            writer.write<execute_fn>(&execute_switch_action<Observed>);
            if constexpr(Observed)
            {
                writer.write<execute_fn>(&apply_switch_action);
            }
            writer.write<execute_fn>(&broadcast_switch_action<Observed>);
            writer.write<execute_fn>(&broadcast_first_round_end<Observed>);
            writer.write<execute_fn>(&broadcast_second_round_end);
        }
    }

    inline void compile(program_writer& writer, const begin_action&, compile_mode mode)
    {
        if(mode == compile_mode::observed)
        {
            begin_action_command::emit<true>(writer);
        }
        else
        {
            begin_action_command::emit<false>(writer);
        }
    }
}

#include "../../macro_undef.hpp"
#endif
