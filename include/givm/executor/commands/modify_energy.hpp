#ifndef GIVM_EXECUTOR_COMMANDS_MODIFY_ENERGY_HPP
#define GIVM_EXECUTOR_COMMANDS_MODIFY_ENERGY_HPP

#include <cstdint>

#include "../executor.hpp"
#include "../character_target.hpp"
#include "../../definition/commands.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<bool Fixed>
    inline execution_state execute_energy_modification(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn&)
    {
        energy_modification input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, modify_energy>(library);
            context.advance(instruction_extent<1, modify_energy>);
            const auto target = resolve_character_target<false>(table, command.target);
            if(not target) return continue_execution;
            input = { *target, command.delta };
        }
        else
        {
            input = get<0>(context.stack().top<energy_modification>());
            context.stack().pop<energy_modification>();
            context.enter_next();
        }
        GIVM_ASSERT(table[input.target].is_valid());
        auto& state = table[input.target].state();
        const auto previous = static_cast<std::int64_t>(state.energy);
        if(input.delta <= -previous)
            state.energy = 0;
        else if(input.delta >= static_cast<std::int64_t>(state.max_energy) - previous)
            state.energy = state.max_energy;
        else
            state.energy = static_cast<std::uint32_t>(previous + input.delta);
        return continue_execution;
    }

    inline void compile(program_writer& writer, const givm::modify_energy& command, compile_mode)
    {
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
            writer.write(execute_fn{ execute_energy_modification<false> });
        else
        {
            GIVM_ASSERT(command.target.selection == character_selection::character);
            [[assume(command.target.selection == character_selection::character)]];
            writer.write(execute_fn{ execute_energy_modification<true> });
            writer.write(command);
        }
    }
}

#include "../../macro_undef.hpp"
#endif
