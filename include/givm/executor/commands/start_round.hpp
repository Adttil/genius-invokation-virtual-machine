#ifndef GIVM_EXECUTOR_COMMANDS_START_ROUND_HPP
#define GIVM_EXECUTOR_COMMANDS_START_ROUND_HPP

#include "../executor.hpp"
#include "../broadcast.hpp"
#include "../../definition/commands.hpp"
#include "../../definition/events.hpp"

namespace givm::detail
{
    template<bool Repeat>
    inline execution_state finish_round_advance(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn&)
    {
        if(table.state().round_number > table.parameters().max_rounds) [[unlikely]]
            return context.end_game(game_result::both_loss);
        for(auto player : table.players())
            player.state().dice = {};
        if constexpr(Repeat)
            return context.jump(context.instruction_data<1, execution_position>(library));
        else
            return context.enter_next();
    }

    template<bool Repeat>
    inline execution_state advance_round(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        ++table.state().round_number;
        return finish_round_advance<Repeat>(library, table, context, random);
    }

    template<bool Repeat>
    inline execution_state observe_round_advance(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn&)
    {
        ++table.state().round_number;
        if constexpr(Repeat)
            context.jump(context.instruction_data<1, execution_position>(library));
        else
            context.enter_next();
        return context.yield(execution_state::round_started);
    }

    inline void compile(program_writer& writer, const round_program_begin&, compile_mode mode)
    {
        if(mode == compile_mode::observed)
        {
            writer.write(execute_fn{ observe_round_advance<false> });
            writer.write(execute_fn{ finish_round_advance<false> });
        }
        else
            writer.write(execute_fn{ advance_round<false> });
    }

    inline void compile(program_writer& writer, const round_program_repeat& command, compile_mode mode)
    {
        if(mode == compile_mode::observed)
        {
            writer.write(execute_fn{ observe_round_advance<true> });
            writer.write(command.round_start + sizeof(execute_fn));
        }
        else
        {
            writer.write(execute_fn{ advance_round<true> });
            writer.write(command.round_entry);
        }
    }

    inline execution_state broadcast_round_start(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<round_started>(library, table, context, random))
            return continue_execution;
        pop_broadcast<round_started>(context);
        return context.enter_next();
    }

    inline execution_state prepare_round_start(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        prepare_broadcast(library, round_started{}, table, context.stack(), context.position() + sizeof(execute_fn));
        context.enter_next();
        return broadcast_round_start(library, table, context, random);
    }

    inline void compile(program_writer& writer, const start_round&, compile_mode)
    {
        writer.write(execute_fn{ prepare_round_start });
        writer.write(execute_fn{ broadcast_round_start });
    }
}

#endif
