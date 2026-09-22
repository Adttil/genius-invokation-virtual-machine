#ifndef GIVM_EXECUTOR_COMMANDS_ADD_SUMMON_HPP
#define GIVM_EXECUTOR_COMMANDS_ADD_SUMMON_HPP

#include <algorithm>

#include "../executor.hpp"
#include "../../definition.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    constexpr summon_state clamp_summon_state(summon_state state, summon_state limit) noexcept
    {
        return { std::min(state.value, limit.value), std::min(state.usages, limit.usages) };
    }

    template<bool Fixed>
    execution_state apply_summon_addition(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        summon_addition input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, add_summon>(library);
            input = {
                .player = command.player == relative_player::current
                    ? table.state().active_player : other_player(table.state().active_player),
                .definition = command.definition, .state = command.state
            };
            context.advance(instruction_extent<1, add_summon>);
        }
        else
        {
            input = get<0>(context.stack().top<summon_addition>());
            context.stack().pop<summon_addition>();
            context.enter_next();
        }
        const auto player = std::as_const(table)[input.player];
        auto remaining_capacity = player.state().summon_limit;
        if(remaining_capacity == 0)
            return continue_execution;
        for([[maybe_unused]] const auto existing : player.summons())
        {
            if(--remaining_capacity == 0)
                return continue_execution;
        }
        input.state = clamp_summon_state(input.state, library[input.definition].query(summon_state_limit{}));
        GIVM_ASSERT(input.state.usages != 0);
        [[assume(input.state.usages != 0)]];
        table[input.player].add(input.definition, input.state);
        return continue_execution;
    }

    inline void compile(program_writer& writer, const givm::add_summon& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ apply_summon_addition<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ apply_summon_addition<false> });
    }
}

#include "../../macro_undef.hpp"
#endif
