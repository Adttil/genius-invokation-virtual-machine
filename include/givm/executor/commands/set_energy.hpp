#ifndef GIVM_EXECUTOR_COMMANDS_SET_ENERGY_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_ENERGY_HPP

#include <algorithm>

#include "../executor.hpp"
#include "../character_target.hpp"
#include "../../definition/commands.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<bool Fixed>
    inline execution_state execute_energy_change(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn&)
    {
        set_energy_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, set_energy>(library);
            context.advance(instruction_extent<1, set_energy>);
            const auto target = resolve_character_target<false>(table, command.target);
            if(not target) return continue_execution;
            input = { *target, command.value };
        }
        else
        {
            input = get<0>(context.stack().top<set_energy_input>());
            context.stack().pop<set_energy_input>();
            context.enter_next();
        }
        GIVM_ASSERT(table[input.target].is_valid());
        auto& state = table[input.target].state();
        state.energy = std::min(input.value, state.max_energy);
        return continue_execution;
    }

    inline void compile(program_writer& writer, const givm::set_energy& command, compile_mode)
    {
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
            writer.write(execute_fn{ execute_energy_change<false> });
        else
        {
            GIVM_ASSERT(command.target.selection == character_selection::character);
            [[assume(command.target.selection == character_selection::character)]];
            writer.write(execute_fn{ execute_energy_change<true> });
            writer.write(command);
        }
    }
}

#include "../../macro_undef.hpp"
#endif
