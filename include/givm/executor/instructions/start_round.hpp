#ifndef GIVM_EXECUTOR_INSTRUCTIONS_START_ROUND_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_START_ROUND_HPP

#include "../executor.hpp"
#include "../../definition/commands.hpp"

namespace givm::detail
{
    namespace start_round_command
    {
        inline execution_state finish(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn&
        )
        {
            const auto& command = context.instruction_data<1, start_round>(library);
            if(table.state().round_number > command.max_rounds) [[unlikely]]
            {
                return context.end_game(game_result::both_loss);
            }
            for(auto player : table.players())
            {
                player.state().dice = {};
            }
            return context.advance(instruction_extent<1, start_round>);
        }

        inline execution_state execute(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            ++table.state().round_number;
            return finish(library, table, context, random);
        }

        inline execution_state observe(
            const definition_library&, unrestricted_table& table,
            execution_context& context, random_fn&
        )
        {
            ++table.state().round_number;
            return context.yield_next(execution_state::round_started);
        }
    }

    inline void compile(program_writer& writer, const start_round& command, compile_mode mode)
    {
        if(mode == compile_mode::observed)
        {
            writer.write<execute_fn>(&start_round_command::observe);
            writer.write<execute_fn>(&start_round_command::finish);
        }
        else
        {
            writer.write<execute_fn>(&start_round_command::execute);
        }
        writer.write(command);
    }
}

#endif
