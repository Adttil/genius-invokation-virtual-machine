#ifndef GIVM_EXECUTOR_COMMANDS_START_BATTLE_HPP
#define GIVM_EXECUTOR_COMMANDS_START_BATTLE_HPP

#include "../program_writer.hpp"

#include <vector>

#include <givm/executor/executor.hpp>
#include "../broadcast.hpp"
#include <givm/definition.hpp>

namespace givm::detail
{
    inline execution_state prepare_battle_start(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(table.state().round_number != 1)
        {
            return context.enter_next();
        }
        append_event_record(context, battle_started{});
        return context.enter_next();
    }

    inline void compile(program_writer& writer, const start_battle&, compile_mode)
    {
        writer.write<execute_fn>(&prepare_battle_start);
    }
}

namespace givm::detail
{
    inline std::vector<start_battle::error_type> check(const start_battle&, const definition_compile_context&, program_kind)
    {
        return {};
    }
}

#endif
