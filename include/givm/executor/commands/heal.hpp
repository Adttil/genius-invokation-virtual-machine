#ifndef GIVM_EXECUTOR_COMMANDS_HEAL_HPP
#define GIVM_EXECUTOR_COMMANDS_HEAL_HPP

#include <algorithm>
#include <limits>

#include "../executor.hpp"
#include "../broadcast.hpp"
#include "../instruction.hpp"
#include "../../definition/commands.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state broadcast_healing_completed(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<healed>(library, table, context, random)) return continue_execution;
        pop_broadcast<healed>(context);
        return context.enter_next();
    }

    inline execution_state apply_healing(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<healing>(library, table, context, random)) return continue_execution;
        const auto event = get<0>(context.stack().top<healing, execution_position>());
        pop_broadcast<healing>(context);
        auto& state = table[event.target].state();
        GIVM_ASSERT(state.health <= state.max_health);
        [[assume(state.health <= state.max_health)]];
        const auto value = std::min(event.value, state.max_health - state.health);
        state.health += value;
        context.enter_next();
        prepare_broadcast(library, healed{ event.source, event.target, value }, table, context.stack(), context.position());
        return broadcast_healing_completed(library, table, context, random);
    }

    template<bool Fixed>
    inline execution_state prepare_healing(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto event = [&]() -> healing
        {
            if constexpr(Fixed)
            {
                const auto command = context.instruction_data<1, heal>(library);
                context.advance(instruction_extent<1, heal>);
                return { command.source, command.target, command.value };
            }
            else
            {
                const auto input = get<0>(context.stack().top<healing>());
                context.stack().pop<healing>();
                context.enter_next();
                return input;
            }
        }();
        const bool valid = static_cast<bool>(table[event.target]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        prepare_broadcast(library, event, table, context.stack(), context.position());
        return apply_healing(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::heal& command, compile_mode)
    {
        if(command.target.index == std::numeric_limits<size_t>::max())
            writer.write(execute_fn{ prepare_healing<false> });
        else
        {
            writer.write(execute_fn{ prepare_healing<true> });
            writer.write(command);
        }
        writer.write(execute_fn{ apply_healing });
        writer.write(execute_fn{ broadcast_healing_completed });
    }
}

#include "../../macro_undef.hpp"
#endif
