#ifndef GIVM_EXECUTOR_COMMANDS_ADD_COMBAT_STATUS_HPP
#define GIVM_EXECUTOR_COMMANDS_ADD_COMBAT_STATUS_HPP

#include <algorithm>

#include "../executor.hpp"
#include "../../definition.hpp"

namespace givm::detail
{
    constexpr combat_status_state clamp_combat_status_state(
        combat_status_state state, combat_status_state limit) noexcept
    {
        return { std::min(state.count, limit.count), std::min(state.round_usages, limit.round_usages) };
    }

    template<bool Fixed>
    execution_state apply_combat_status_addition(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        combat_status_addition input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, add_combat_status>(library);
            input = {
                .player = command.player == relative_player::current
                    ? table.state().active_player : other_player(table.state().active_player),
                .definition = command.definition, .state = command.state
            };
            context.advance(instruction_extent<1, add_combat_status>);
        }
        else
        {
            input = get<0>(context.stack().top<combat_status_addition>());
            context.stack().pop<combat_status_addition>();
            context.enter_next();
        }
        input.state = clamp_combat_status_state(
            input.state, library[input.definition].query(combat_status_state_limit{}));
        table[input.player].add(input.definition, input.state);
        return continue_execution;
    }

    inline void compile(program_writer& writer, const givm::add_combat_status& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ apply_combat_status_addition<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ apply_combat_status_addition<false> });
    }
}

#endif
