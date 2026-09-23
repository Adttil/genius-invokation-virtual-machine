#ifndef GIVM_EXECUTOR_COMMANDS_HEAL_HPP
#define GIVM_EXECUTOR_COMMANDS_HEAL_HPP

#include <algorithm>
#include <limits>

#include "../executor.hpp"
#include "../character_target.hpp"
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
        const auto event = get<0>(context.stack().top<healing, response_return>());
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
        const auto event = [&]() -> std::optional<healing>
        {
            if constexpr(Fixed)
            {
                const auto command = context.instruction_data<1, heal>(library);
                context.advance(instruction_extent<1, heal>);
                const auto source = resolve_character_target<false>(table, command.source);
                const auto target = resolve_character_target<false>(table, command.target);
                if(not source || not target) return std::nullopt;
                return healing{ *source, *target, command.value };
            }
            else
            {
                const auto input = get<0>(context.stack().top<healing>());
                context.stack().pop<healing>();
                context.enter_next();
                return input;
            }
        }();
        if(not event) return context.advance(2 * sizeof(execute_fn));
        const bool valid = static_cast<bool>(table[event->target]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        prepare_broadcast(library, *event, table, context.stack(), context.position());
        return apply_healing(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::heal& command, compile_mode)
    {
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
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
