#ifndef GIVM_EXECUTOR_INSTRUCTIONS_END_GAME_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_END_GAME_HPP

#include "../executor.hpp"
#include "../../definition/commands.hpp"

namespace givm::detail
{
    namespace end_game_command
    {
        inline execution_state execute(
            const definition_library& library, unrestricted_table&,
            execution_context& context, random_fn&
        )
        {
            return context.end_game(context.instruction_data<1, end_game>(library).result);
        }
    }

    inline void compile(program_writer& writer, const end_game& command, compile_mode)
    {
        writer.write<execute_fn>(&end_game_command::execute);
        writer.write(command);
    }
}

#endif
