#ifndef GIVM_EXECUTOR_COMMANDS_START_ROUND_HPP
#define GIVM_EXECUTOR_COMMANDS_START_ROUND_HPP

#include "../program_writer.hpp"

#include <vector>

#include <givm/executor/executor.hpp>
#include "../broadcast.hpp"
#include <givm/definition.hpp>

namespace givm::detail
{
    // Internal compilation inputs; definition sources do not expose these commands.
    struct round_program_begin {};

    struct round_program_repeat
    {
        execution_position round_start;
        execution_position round_entry;
    };

    template<bool Repeat>
    inline execution_state finish_round_advance(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn&)
    {
        if(table.state().round_number > table.state().max_rounds) [[unlikely]]
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

    inline execution_state prepare_round_start(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        append_event_record(context, round_started{});
        return context.enter_next();
    }

    inline void compile(program_writer& writer, const start_round&, compile_mode)
    {
        writer.write(execute_fn{ prepare_round_start });
    }
}

namespace givm::detail
{
    inline std::vector<start_round::error_type> check(const start_round&, const definition_compile_context&, program_kind)
    {
        return {};
    }
}

#endif
