#ifndef GIVM_EXECUTOR_COMMANDS_END_ROUND_HPP
#define GIVM_EXECUTOR_COMMANDS_END_ROUND_HPP

#include "../program_writer.hpp"

#include <vector>

#include <givm/executor/executor.hpp>
#include "../broadcast.hpp"
#include <givm/definition.hpp>

namespace givm::detail
{
    inline execution_state broadcast_round_end(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<round_ended>(library, table, context, random))
        {
            return continue_execution;
        }
        pop_broadcast<round_ended>(context);
        return context.advance(response_extent<round_ended>);
    }

    inline execution_state prepare_round_end(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        auto& state = table.state();
        state.active_player = other_player(state.active_player);
        state.first_ended = false;
        prepare_broadcast(library, round_ended{}, table, context.stack(), context.position() + sizeof(execute_fn));
        context.enter_next();
        return broadcast_round_end(library, table, context, random);
    }

    inline execution_state observe_round_end(
        const definition_library&, unrestricted_table&,
        execution_context& context, random_fn&
    )
    {
        return context.yield_next(execution_state::round_ending);
    }

    inline void compile(program_writer& writer, const end_round&, compile_mode mode)
    {
        if(mode == compile_mode::observed)
        {
            writer.write<execute_fn>(&observe_round_end);
        }
        writer.write<execute_fn>(&prepare_round_end);
        compile_broadcast<round_ended>(writer, broadcast_round_end);
    }
}

namespace givm::detail
{
    inline std::vector<end_round::error_type> check(const end_round&, const definition_compile_context&, program_kind)
    {
        return {};
    }
}

#endif
