#ifndef GIVM_EXECUTOR_COMMANDS_ADD_SUPPORT_HPP
#define GIVM_EXECUTOR_COMMANDS_ADD_SUPPORT_HPP

#include <algorithm>
#include <utility>

#include "../executor.hpp"
#include "../../definition.hpp"

namespace givm::detail
{
    constexpr support_state clamp_support_state(support_state state, support_state limit) noexcept
    {
        return { std::min(state.count, limit.count), std::min(state.round_usages, limit.round_usages) };
    }

    template<bool Fixed>
    inline execution_state apply_support_addition(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        add_support_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, add_support>(library);
            input = {
                .player = command.player == relative_player::self
                    ? table.state().self_player : other_player(table.state().self_player),
                .definition = command.definition, .state = command.state
            };
            context.advance(instruction_extent<1, add_support>);
        }
        else
        {
            input = get<0>(context.stack().top<add_support_input>());
            context.stack().pop<add_support_input>();
            context.enter_next();
        }

        const auto player = std::as_const(table)[input.player];
        auto remaining_capacity = player.state().support_limit;
        if(remaining_capacity == 0)
            return continue_execution;
        for([[maybe_unused]] const auto existing : player.supports())
        {
            if(--remaining_capacity == 0)
                return continue_execution;
        }
        input.state = clamp_support_state(input.state, library[input.definition].query(support_state_limit{}));
        table[input.player].add(input.definition, input.state);
        return continue_execution;
    }

    inline void compile(program_writer& writer, const givm::add_support& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ apply_support_addition<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ apply_support_addition<false> });
    }
}

#endif
