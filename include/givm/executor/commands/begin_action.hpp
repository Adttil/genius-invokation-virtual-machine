#ifndef GIVM_EXECUTOR_COMMANDS_BEGIN_ACTION_HPP
#define GIVM_EXECUTOR_COMMANDS_BEGIN_ACTION_HPP

#include "../executor.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
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
        struct switch_selection
        {
            stack_count_t switch_cost_index = 0;
            dice_counts paid_dice;
        };

        struct round_end_selection {};

        using action_selection = std::variant<round_end_selection, switch_selection>;
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
    };
}

namespace givm::detail
{
    using switch_handler_id = handler_id<cost_of_switch>;

    // Offsets count execute_fn entries from the start of this command. They never
    // become runtime state or require a second dispatch after fetching the instruction.
    inline constexpr std::size_t prepare_action_phase_offset = 0;
    inline constexpr std::size_t action_phase_broadcast_offset = 1;
    inline constexpr std::size_t before_action_offset = 2;
    inline constexpr std::size_t before_action_with_switch_offset = 3;
    inline constexpr std::size_t before_action_broadcast_offset = 4;
    inline constexpr std::size_t execute_action_selection_offset = 5;
    inline constexpr std::size_t switch_onpay_offset = 6;
    inline constexpr std::size_t after_fixed_switch_onpay_offset = 7;
    inline constexpr std::size_t switch_payment_broadcast_offset = 8;
    inline constexpr std::size_t switch_action_offset = 9;
    inline constexpr std::size_t switch_action_apply_offset = 10;

    template<bool Observed>
    inline constexpr std::size_t switch_action_broadcast_offset = 10 + Observed;
    template<bool Observed>
    inline constexpr std::size_t first_round_end_broadcast_offset = 11 + Observed;
    template<bool Observed>
    inline constexpr std::size_t second_round_end_broadcast_offset = 12 + Observed;

    template<std::size_t From, std::size_t To>
    execution_state jump_to_action_instruction(execution_context& context) noexcept
    {
        return context.jump(context.position() - From * sizeof(execute_fn) + To * sizeof(execute_fn));
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

    inline const cost_of_switch& calculate_switch_cost(
        const definition_library& library,
        stack_count_t cost_index,
        const table& card_table,
        frame_stack& stack
    )
    {
        auto&& [handlers, switch_cost_indices, costs, onpay_items, onpay_cursor, selection] =
            stack.top<
                switch_handler_id[],
                stack_count_t[],
                cost_of_switch[],
                onpay_item<cost_of_switch>[],
                stack_count_t,
                action_selection
            >();
        GIVM_ASSERT(cost_index < costs.size());
        auto& cost = costs[cost_index];
        cost.requirement = default_switch_cost(cost.target).requirement;

        auto zero_random = []() -> std::uint32_t { return 0; };
        random_fn random{ zero_random };
        const auto handler_count = static_cast<stack_count_t>(handlers.size());
        const auto row_begin = cost_index * handler_count;
        for(stack_count_t column = 0; column < handler_count; ++column)
        {
            auto& onpay = onpay_items[row_begin + column];
            cost.effect_argument = {};
            const auto entry = std::visit([&](auto handler)
            {
                const auto entity = card_table[handler];
                if(not entity)
                {
                    return handler_program_entry_t<cost_of_switch>::null();
                }
                return library[entity.definition_id()].template handle<cost_of_switch>(
                    entity, cost, card_table, random
                );
            }, handlers[column]);
            onpay = { .entry = entry, .argument = cost.effect_argument };
        }
        return cost;
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
        constexpr auto from = Switch ? before_action_with_switch_offset : before_action_offset;
        jump_to_action_instruction<from, before_action_broadcast_offset>(context);
        if constexpr(Switch && Observed)
        {
            return context.yield(execution_state::action_started);
        }
        return continue_execution;
    }

    inline execution_state prepare_action_selection(
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
        const auto characters = player.characters<false>();
        const auto character_count = static_cast<stack_count_t>(characters.size());
        auto&& [handlers, switch_cost_indices, costs, onpay_items, onpay_cursor, selection] = context.stack().push(
            dynamic_array<switch_handler_id>(cost_handlers),
            dynamic_array<stack_count_t>(character_count),
            dynamic_array<cost_of_switch>(switch_count),
            dynamic_array<onpay_item<cost_of_switch>>(matrix_size),
            stack_count_t{},
            action_selection{}
        );

        stack_count_t index = 0;
        for(stack_count_t character_index = 0; character_index < character_count; ++character_index)
        {
            const auto character = characters[character_index];
            if(character && is_switch_target(character))
            {
                switch_cost_indices[character_index] = index;
                std::construct_at(&costs[index++], default_switch_cost(character.id()));
            }
            else
            {
                switch_cost_indices[character_index] = switch_count;
            }
        }
        return context.yield(execution_state::action_selection);
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
        return prepare_action_selection(library, table, context);
    }

    template<bool Observed>
    execution_state execute_action_selection(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        auto&& [handlers, switch_cost_indices, costs, onpay_items, onpay_cursor, selection] =
            context.stack().top<
                switch_handler_id[],
                stack_count_t[],
                cost_of_switch[],
                onpay_item<cost_of_switch>[],
                stack_count_t,
                action_selection
            >();
        if(std::holds_alternative<round_end_selection>(selection))
        {
            context.stack().pop<
                switch_handler_id[],
                stack_count_t[],
                cost_of_switch[],
                onpay_item<cost_of_switch>[],
                stack_count_t,
                action_selection
            >();
            auto& state = table.state();
            const bool is_first = not state.first_ended;
            if(is_first)
            {
                state.first_ended = true;
            }
            prepare_broadcast(library, round_end_declared{}, table, context.stack());
            if(is_first)
            {
                jump_to_action_instruction<execute_action_selection_offset, first_round_end_broadcast_offset<Observed>>(context);
            }
            else
            {
                jump_to_action_instruction<execute_action_selection_offset, second_round_end_broadcast_offset<Observed>>(context);
            }
            if constexpr(Observed)
            {
                return context.yield(execution_state::round_end_declared);
            }
            return continue_execution;
        }

#ifndef NDEBUG
        const auto* selected = std::get_if<switch_selection>(&selection);
        GIVM_ASSERT(selected != nullptr);
        GIVM_ASSERT(selected->switch_cost_index < costs.size());
#endif

        onpay_cursor = 0;
        return context.enter_next();
    }

    inline execution_state continue_switch_onpay(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        auto&& [handlers, switch_cost_indices, costs, onpay_items, onpay_cursor, selection] =
            context.stack().top<
                switch_handler_id[],
                stack_count_t[],
                cost_of_switch[],
                onpay_item<cost_of_switch>[],
                stack_count_t,
                action_selection
            >();
        const auto* selected = std::get_if<switch_selection>(&selection);
        GIVM_ASSERT(selected != nullptr);
        GIVM_ASSERT(selected->switch_cost_index < costs.size());
        const auto handler_count = static_cast<stack_count_t>(handlers.size());
        const auto row_begin = selected->switch_cost_index * handler_count;
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

        const auto paid_dice = selected->paid_dice;
        if(paid_dice.total() == 0)
        {
            return jump_to_action_instruction<switch_onpay_offset, switch_action_offset>(context);
        }

        const auto player = table.state().active_player;
        auto& player_dice = table[player].state().dice;
        GIVM_ASSERT(player_dice.contains(paid_dice));
        player_dice -= paid_dice;

        prepare_broadcast(library, dice_removed{ .player = player, .dice = paid_dice }, table, context.stack());
        return jump_to_action_instruction<switch_onpay_offset, switch_payment_broadcast_offset>(context);
    }

    inline execution_state after_fixed_switch_onpay(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        context.stack().pop<switch_handler_id, cost_effect_argument<cost_of_switch>>();
        jump_to_action_instruction<after_fixed_switch_onpay_offset, switch_onpay_offset>(context);
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
        auto&& [handlers, switch_cost_indices, costs, onpay_items, onpay_cursor, selection] =
            context.stack().top<
                switch_handler_id[],
                stack_count_t[],
                cost_of_switch[],
                onpay_item<cost_of_switch>[],
                stack_count_t,
                action_selection
            >();
        const auto* selected = std::get_if<switch_selection>(&selection);
        GIVM_ASSERT(selected != nullptr);
        GIVM_ASSERT(selected->switch_cost_index < costs.size());
        const auto target = costs[selected->switch_cost_index].target;
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

        auto&& [handlers, switch_cost_indices, costs, onpay_items, onpay_cursor, selection] =
            context.stack().top<
                switch_handler_id[],
                stack_count_t[],
                cost_of_switch[],
                onpay_item<cost_of_switch>[],
                stack_count_t,
                action_selection
            >();
        const auto* selected = std::get_if<switch_selection>(&selection);
        GIVM_ASSERT(selected != nullptr);
        GIVM_ASSERT(selected->switch_cost_index < costs.size());
        const auto speed = costs[selected->switch_cost_index].requirement.speed;
        context.stack().pop<
            switch_handler_id[],
            stack_count_t[],
            cost_of_switch[],
            onpay_item<cost_of_switch>[],
            stack_count_t,
            action_selection
        >();
        if(speed == action_speed::combat)
        {
            return jump_to_action_instruction<switch_action_broadcast_offset<Observed>, before_action_with_switch_offset>(context);
        }
        return jump_to_action_instruction<switch_action_broadcast_offset<Observed>, before_action_offset>(context);
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
        jump_to_action_instruction<first_round_end_broadcast_offset<Observed>, before_action_offset>(context);
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
    void compile_begin_action(program_writer& writer)
    {
        writer.write<execute_fn>(&prepare_action_phase);
        writer.write<execute_fn>(&broadcast_action_phase<Observed>);
        writer.write<execute_fn>(&prepare_before_action<false, Observed>);
        writer.write<execute_fn>(&prepare_before_action<true, Observed>);
        writer.write<execute_fn>(&broadcast_before_action);
        writer.write<execute_fn>(&execute_action_selection<Observed>);
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

    inline void compile(program_writer& writer, const begin_action&, compile_mode mode)
    {
        if(mode == compile_mode::observed)
        {
            compile_begin_action<true>(writer);
        }
        else
        {
            compile_begin_action<false>(writer);
        }
    }
}

#include "../../macro_undef.hpp"
#endif
