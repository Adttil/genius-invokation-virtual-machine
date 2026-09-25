#ifndef GIVM_EXECUTOR_COMMANDS_SET_ACTIVE_CHARACTER_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_ACTIVE_CHARACTER_HPP

#include <optional>
#include <utility>
#include <vector>

#include "../executor.hpp"
#include "../character_target.hpp"
#include "../broadcast.hpp"
#include "../../definition/events.hpp"
#include "../instruction.hpp"
#include "../../definition/commands.hpp"
#include "../../utils/debug.hpp"

#include "../../macro_define.hpp"

namespace givm::detail
{
    inline std::optional<execution_state> continue_switch_prepared_removal(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        for(;;)
        {
            if(not continue_broadcast<attachment_removed>(library, table, context, random)) return continue_execution;
            pop_broadcast<attachment_removed>(context);
            const auto frame = context.stack().top<attachment_id[], stack_count_t, active_character_changed, execution_position>();
            const auto& attachments = get<0>(frame);
            auto& cursor = get<1>(frame);
            if(cursor != attachments.size())
            {
                const auto event = attachment_removed{ attachments[cursor++] };
                prepare_broadcast(library, event, table, context.stack(), context.position());
                continue;
            }
            const auto event = get<2>(frame);
            const auto position = get<3>(frame);
            context.stack().pop<attachment_id[], stack_count_t, active_character_changed, execution_position>();
            prepare_broadcast(library, event, table, context.stack(), position);
            context.jump(position);
            return std::nullopt;
        }
    }

    inline std::optional<execution_state> prepare_active_character_switch(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random,
        active_character_changed event, execution_position removal_resume, execution_position broadcast_resume)
    {
        auto& state = table[event.current.player_id].state();
        std::vector<attachment_id> removed;
        if(state.active_character != event.current)
        {
            if(state.active_character)
            {
                for(const auto attachment : table[*state.active_character].attachments())
                {
                    if(library[attachment.definition_id()].template can_handle<prepared_skill_effect, attachment_view>())
                    {
                        removed.push_back(attachment.id());
                        attachment.erase();
                    }
                }
            }
            state.can_plunge = true;
        }
        // All original preparations leave before any removal response runs.
        state.active_character = event.current;
        if(removed.empty())
        {
            prepare_broadcast(library, event, table, context.stack(), broadcast_resume);
            context.jump(broadcast_resume);
            return std::nullopt;
        }
        const auto first = removed.front();
        context.stack().push(dynamic_array<attachment_id>(removed), stack_count_t{ 1 }, event, broadcast_resume);
        prepare_broadcast(library, attachment_removed{ first }, table, context.stack(), removal_resume);
        context.jump(removal_resume);
        return continue_switch_prepared_removal(library, table, context, random);
    }

    inline execution_state broadcast_active_character_change(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<active_character_changed>(library, table, context, random))
        {
            return continue_execution;
        }
        pop_broadcast<active_character_changed>(context);
        return context.enter_next();
    }

    inline execution_state resume_active_character_prepared_removal(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(const auto state = continue_switch_prepared_removal(library, table, context, random)) return *state;
        return broadcast_active_character_change(library, table, context, random);
    }

    inline execution_state apply_active_character_change(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto frame = context.stack().top<active_character_changed, response_return>();
        const auto event = get<0>(frame);
        const auto position = get<1>(frame).position;
        context.stack().pop<active_character_changed, response_return>();
        if(const auto state = prepare_active_character_switch(library, table, context, random,
            event, position - 2 * sizeof(execute_fn), position)) return *state;
        return broadcast_active_character_change(library, table, context, random);
    }

    template<bool Observed>
    inline execution_state prepare_active_character_change(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto& command = context.instruction_data<1, givm::set_active_character>(library);
        const auto removal_resume = context.position() + instruction_extent<1, givm::set_active_character>;
        const auto broadcast_resume = removal_resume + (Observed ? 2 : 1) * sizeof(execute_fn);
        const auto target = resolve_character_target<true>(table, command.target);
        if(not target) return context.jump(broadcast_resume + sizeof(execute_fn));
        const auto& state = table[target->player_id].state();
        if(state.active_character == *target)
            return context.jump(broadcast_resume + sizeof(execute_fn));
        if(state.active_character && library.is_control_immune(std::as_const(table)[*state.active_character]))
            return context.jump(broadcast_resume + sizeof(execute_fn));
        const active_character_changed event{ .current = *target };
        if constexpr(Observed)
        {
            context.stack().push(event, response_return{ table.state().self_player, broadcast_resume });
            context.jump(removal_resume + sizeof(execute_fn));
            return context.yield(execution_state::active_character_changed);
        }
        if(const auto result = prepare_active_character_switch(library, table, context, random,
            event, removal_resume, broadcast_resume)) return *result;
        return broadcast_active_character_change(library, table, context, random);
    }

    template<bool Observed>
    inline execution_state prepare_active_character_change_from_input(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto event = get<0>(context.stack().top<active_character_changed>());
        context.stack().pop<active_character_changed>();
        GIVM_ASSERT(static_cast<bool>(table[event.current]));
        const auto removal_resume = context.position() + sizeof(execute_fn);
        const auto broadcast_resume = removal_resume + (Observed ? 2 : 1) * sizeof(execute_fn);
        const auto& state = table[event.current.player_id].state();
        if(state.active_character == event.current)
            return context.jump(broadcast_resume + sizeof(execute_fn));
        if(state.active_character && library.is_control_immune(std::as_const(table)[*state.active_character]))
            return context.jump(broadcast_resume + sizeof(execute_fn));
        if constexpr(Observed)
        {
            context.stack().push(event, response_return{ table.state().self_player, broadcast_resume });
            context.jump(removal_resume + sizeof(execute_fn));
            return context.yield(execution_state::active_character_changed);
        }
        if(const auto result = prepare_active_character_switch(library, table, context, random,
            event, removal_resume, broadcast_resume)) return *result;
        return broadcast_active_character_change(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::set_active_character& command, compile_mode mode)
    {
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
        {
            writer.write(mode == compile_mode::observed
                ? execute_fn{ &prepare_active_character_change_from_input<true> }
                : execute_fn{ &prepare_active_character_change_from_input<false> });
        }
        else
        {
            writer.write(mode == compile_mode::observed
                ? execute_fn{ &prepare_active_character_change<true> }
                : execute_fn{ &prepare_active_character_change<false> });
            writer.write(command);
        }
        writer.write(execute_fn{ &resume_active_character_prepared_removal });
        if(mode == compile_mode::observed)
        {
            writer.write(execute_fn{ &apply_active_character_change });
        }
        writer.write(execute_fn{ &broadcast_active_character_change });
    }
}

#include "../../macro_undef.hpp"
#endif
