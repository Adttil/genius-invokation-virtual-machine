#ifndef GIVM_EXECUTOR_COMMANDS_SET_ACTIVE_CHARACTER_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_ACTIVE_CHARACTER_HPP

#include <utility>

#include "../executor.hpp"
#include "../broadcast.hpp"
#include "../../definition/events.hpp"
#include "../instruction.hpp"
#include "../../definition/commands.hpp"
#include "../../utils/debug.hpp"

#include "../../macro_define.hpp"

namespace givm::detail
{
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

    inline execution_state apply_active_character_change(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        const auto& event = get<0>(context.stack().top<active_character_changed, execution_position>());
        table[event.current.player_id].state().active_character = event.current;
        table[event.current.player_id].state().can_plunge = true;
        context.enter_next();
        return broadcast_active_character_change(library, table, context, random);
    }

    template<bool Observed>
    inline execution_state prepare_active_character_change(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        const auto& command = context.instruction_data<1, givm::set_active_character>(library);
        GIVM_ASSERT(static_cast<bool>(table[command.target]));

        auto& state = table[command.target.player_id].state();
        if(state.active_character && library.is_control_immune(std::as_const(table)[*state.active_character]))
            return context.advance(instruction_extent<1, givm::set_active_character>
                + (Observed ? 2 : 1) * sizeof(execute_fn));
        const active_character_changed event{ .current = command.target };
        if constexpr(Observed)
        {
            if(state.active_character != command.target)
            {
                prepare_broadcast(library, event, table, context.stack(),
                    context.position() + instruction_extent<1, givm::set_active_character> + sizeof(execute_fn));
                context.advance(instruction_extent<1, givm::set_active_character>);
                return context.yield(execution_state::active_character_changed);
            }
        }
        if(state.active_character != command.target) state.can_plunge = true;
        state.active_character = command.target;
        prepare_broadcast(library, event, table, context.stack(),
            context.position() + instruction_extent<1, givm::set_active_character>
                + (Observed ? sizeof(execute_fn) : 0));
        // The observation continuation applies the change after the pause.
        // Skip it when no pause was required.
        context.advance(instruction_extent<1, givm::set_active_character>
            + (Observed ? sizeof(execute_fn) : 0));
        return broadcast_active_character_change(library, table, context, random);
    }

    template<bool Observed>
    inline execution_state prepare_active_character_change_from_input(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        const auto event = get<0>(context.stack().top<active_character_changed>());
        context.stack().pop<active_character_changed>();
        GIVM_ASSERT(static_cast<bool>(table[event.current]));
        auto& state = table[event.current.player_id].state();
        if(state.active_character && library.is_control_immune(std::as_const(table)[*state.active_character]))
            return context.advance((Observed ? 3 : 2) * sizeof(execute_fn));
        if constexpr(Observed)
        {
            if(state.active_character != event.current)
            {
                prepare_broadcast(library, event, table, context.stack(),
                    context.position() + 2 * sizeof(execute_fn));
                context.enter_next();
                return context.yield(execution_state::active_character_changed);
            }
        }
        if(state.active_character != event.current) state.can_plunge = true;
        state.active_character = event.current;
        prepare_broadcast(library, event, table, context.stack(),
            context.position() + (Observed ? 2 : 1) * sizeof(execute_fn));
        context.advance((Observed ? 2 : 1) * sizeof(execute_fn));
        return broadcast_active_character_change(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::set_active_character& command, compile_mode mode)
    {
        if(command.target.index == std::numeric_limits<size_t>::max())
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
        if(mode == compile_mode::observed)
        {
            writer.write(execute_fn{ &apply_active_character_change });
        }
        writer.write(execute_fn{ &broadcast_active_character_change });
    }
}

#include "../../macro_undef.hpp"
#endif
