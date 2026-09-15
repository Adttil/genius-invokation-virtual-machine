#ifndef GIVM_EXECUTOR_INSTRUCTIONS_SET_ACTIVE_CHARACTER_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_SET_ACTIVE_CHARACTER_HPP

#include "../executor.hpp"
#include "../broadcast.hpp"
#include "../../definition/events.hpp"
#include "../instruction.hpp"
#include "../../definition/commands.hpp"
#include "../../utils/debug.hpp"

#include "../../macro_define.hpp"

namespace givm::detail
{
    namespace set_active_character_command
    {
        inline execution_state broadcast(
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

        inline execution_state apply(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            const auto& [event, handler] = context.stack().top<
                active_character_changed, handler_id<active_character_changed>
            >();
            table[event.current.player_id].state().active_character = event.current;
            context.enter_next();
            return broadcast(library, table, context, random);
        }

        template<bool Observed>
        execution_state prepare(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            const auto& command = context.instruction_data<1, givm::set_active_character>(library);
            GIVM_ASSERT(static_cast<bool>(table[command.target]));

            auto& state = table[command.target.player_id].state();
            const active_character_changed event{ .current = command.target };
            if constexpr(Observed)
            {
                if(state.active_character != command.target)
                {
                    prepare_broadcast(library, event, table, context.stack());
                    context.advance(instruction_extent<1, givm::set_active_character>);
                    return context.yield(execution_state::active_character_changed);
                }
            }
            state.active_character = command.target;
            prepare_broadcast(library, event, table, context.stack());
            // The observation continuation applies the change after the pause.
            // Skip it when no pause was required.
            context.advance(instruction_extent<1, givm::set_active_character>
                + (Observed ? sizeof(execute_fn) : 0));
            return broadcast(library, table, context, random);
        }
    }

    inline void compile(program_writer& writer, const givm::set_active_character& command, compile_mode mode)
    {
        using namespace set_active_character_command;
        writer.write(mode == compile_mode::observed ? execute_fn{ &prepare<true> } : execute_fn{ &prepare<false> });
        writer.write(command);
        if(mode == compile_mode::observed)
        {
            writer.write(execute_fn{ &apply });
        }
        writer.write(execute_fn{ &broadcast });
    }
}

#include "../../macro_undef.hpp"
#endif
