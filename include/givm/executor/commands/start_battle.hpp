#ifndef GIVM_EXECUTOR_COMMANDS_START_BATTLE_HPP
#define GIVM_EXECUTOR_COMMANDS_START_BATTLE_HPP

#include <vector>

#include "../executor.hpp"
#include "../broadcast.hpp"
#include "../../definition.hpp"

namespace givm::detail
{
    inline execution_state broadcast_battle_start(
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

    inline execution_state prepare_battle_start(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(table.state().round_number != 1)
        {
            return context.advance(2 * sizeof(execute_fn));
        }
        prepare_broadcast(library, battle_started{}, table, context.stack(), context.position() + sizeof(execute_fn));
        context.enter_next();
        return broadcast_battle_start(library, table, context, random);
    }

    inline void compile(program_writer& writer, const start_battle&, compile_mode)
    {
        writer.write<execute_fn>(&prepare_battle_start);
        writer.write<execute_fn>(&broadcast_battle_start);
    }
}

namespace givm
{
    inline std::vector<start_battle::error_type> check(const start_battle&, const definition_compile_context&, program_kind)
    {
        return {};
    }
}

#endif
