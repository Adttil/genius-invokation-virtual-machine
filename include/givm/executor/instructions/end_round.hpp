#ifndef GIVM_EXECUTOR_INSTRUCTIONS_END_ROUND_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_END_ROUND_HPP

#include "../executor.hpp"
#include "../broadcast.hpp"
#include "../../definition/events.hpp"
#include "../../definition/commands.hpp"

namespace givm::detail
{
    namespace end_round_command
    {
        inline execution_state broadcast(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            if(not continue_broadcast<round_ended>(library, table, context, random))
            {
                return continue_execution;
            }
            pop_broadcast<round_ended>(context);
            return context.enter_next();
        }

        inline execution_state prepare(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            auto& state = table.state();
            state.active_player = other_player(state.active_player);
            state.first_ended = false;
            prepare_broadcast(library, round_ended{}, table, context.stack());
            context.enter_next();
            return broadcast(library, table, context, random);
        }

        inline execution_state observe(
            const definition_library&, unrestricted_table&,
            execution_context& context, random_fn&
        )
        {
            return context.yield_next(execution_state::round_ending);
        }
    }

    inline void compile(program_writer& writer, const end_round&, compile_mode mode)
    {
        if(mode == compile_mode::observed)
        {
            writer.write<execute_fn>(&end_round_command::observe);
        }
        writer.write<execute_fn>(&end_round_command::prepare);
        writer.write<execute_fn>(&end_round_command::broadcast);
    }
}

#endif
