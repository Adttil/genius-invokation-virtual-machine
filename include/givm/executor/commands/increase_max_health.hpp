#ifndef GIVM_EXECUTOR_COMMANDS_INCREASE_MAX_HEALTH_HPP
#define GIVM_EXECUTOR_COMMANDS_INCREASE_MAX_HEALTH_HPP

#include "heal.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<bool Fixed>
    inline execution_state execute_max_health_increase(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto input = [&]() -> healing
        {
            if constexpr(Fixed)
            {
                const auto command = context.instruction_data<1, increase_max_health>(library);
                context.advance(instruction_extent<1, increase_max_health>);
                return { command.source, command.target, command.value };
            }
            else
            {
                const auto event = get<0>(context.stack().top<healing>());
                context.stack().pop<healing>();
                context.enter_next();
                return event;
            }
        }();
        const bool valid = static_cast<bool>(table[input.target]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        auto& state = table[input.target].state();
        GIVM_ASSERT(state.health <= state.max_health);
        [[assume(state.health <= state.max_health)]];
        const auto value = std::min(input.value, std::numeric_limits<std::uint32_t>::max() - state.max_health);
        state.max_health += value;
        state.health += value;
        prepare_broadcast(library, healed{ input.source, input.target, value }, table, context.stack(), context.position());
        return broadcast_healing_completed(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::increase_max_health& command, compile_mode)
    {
        if(command.target.index == std::numeric_limits<size_t>::max())
            writer.write(execute_fn{ execute_max_health_increase<false> });
        else
        {
            writer.write(execute_fn{ execute_max_health_increase<true> });
            writer.write(command);
        }
        writer.write(execute_fn{ broadcast_healing_completed });
    }
}

#include "../../macro_undef.hpp"
#endif
