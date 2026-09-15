#ifndef GIVM_EXECUTOR_INSTRUCTIONS_START_BATTLE_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_START_BATTLE_HPP

#include "../executor.hpp"
#include "../broadcast.hpp"
#include "../../definition/events.hpp"
#include "../../definition/commands.hpp"

namespace givm::detail
{
    namespace start_battle_command
    {
        inline execution_state broadcast(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            if(not continue_broadcast<battle_started>(library, table, context, random))
            {
                return continue_execution;
            }
            pop_broadcast<battle_started>(context);
            return context.enter_next();
        }

        inline execution_state prepare(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            if(table.state().round_number != 1)
            {
                return context.advance(2 * sizeof(execute_fn));
            }
            prepare_broadcast(library, battle_started{}, table, context.stack());
            context.enter_next();
            return broadcast(library, table, context, random);
        }
    }

    inline void compile(program_writer& writer, const start_battle&, compile_mode)
    {
        writer.write<execute_fn>(&start_battle_command::prepare);
        writer.write<execute_fn>(&start_battle_command::broadcast);
    }
}

#endif
