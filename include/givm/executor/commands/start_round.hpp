#ifndef GIVM_EXECUTOR_COMMANDS_START_ROUND_HPP
#define GIVM_EXECUTOR_COMMANDS_START_ROUND_HPP

#include "../executor.hpp"
#include "../broadcast.hpp"
#include "../../definition/commands.hpp"

namespace givm::detail
{
    inline execution_state broadcast_round_start(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<round_started>(library, table, context, random))
        {
            return continue_execution;
        }
        pop_broadcast<round_started>(context);
        return context.enter_next();
    }

    inline execution_state finish_round_start(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
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
        const auto resume = context.position() + instruction_extent<1, start_round>;
        prepare_broadcast(library, round_started{}, table, context.stack(), resume);
        context.jump(resume);
        return broadcast_round_start(library, table, context, random);
    }

    inline execution_state start_round_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        ++table.state().round_number;
        return finish_round_start(library, table, context, random);
    }

    inline execution_state observe_round_start(
        const definition_library&, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        ++table.state().round_number;
        return context.yield_next(execution_state::round_started);
    }

    inline void compile(program_writer& writer, const start_round& command, compile_mode mode)
    {
        if(mode == compile_mode::observed)
        {
            writer.write<execute_fn>(&observe_round_start);
            writer.write<execute_fn>(&finish_round_start);
        }
        else
        {
            writer.write<execute_fn>(&start_round_execute);
        }
        writer.write(command);
        writer.write<execute_fn>(&broadcast_round_start);
    }
}

#endif
