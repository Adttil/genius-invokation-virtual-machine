#ifndef GIVM_EXECUTOR_COMMANDS_END_GAME_HPP
#define GIVM_EXECUTOR_COMMANDS_END_GAME_HPP

#include "../executor.hpp"
#include "../../definition/commands.hpp"

namespace givm::detail
{
    inline execution_state end_game_execute(
        const definition_library& library, unrestricted_table&,
        execution_context& context, random_fn&
    )
    {
        return context.end_game(context.instruction_data<1, end_game>(library).result);
    }

    inline void compile(program_writer& writer, const end_game& command, compile_mode)
    {
        writer.write<execute_fn>(&end_game_execute);
        writer.write(command);
    }
}

#endif
