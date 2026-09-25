#ifndef GIVM_EXECUTOR_COMMANDS_SET_SUMMON_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_SUMMON_STATE_HPP

#include "add_summon.hpp"
#include "remove_summon.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<bool Fixed>
    inline execution_state execute_summon_state_change(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, set_summon_state>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            const auto summon = require_summon(table, player, command.definition);
            table[summon].state() = clamp_summon_state(command.state,
                library[command.definition].query(summon_state_limit{}));
            return context.advance(instruction_extent<1, set_summon_state>);
        }
        else
        {
            const auto changes = get<0>(context.stack().top<set_summon_state_input::change[]>());
            for(const auto& change : changes)
            {
                auto summon = table[change.summon];
                const bool valid = static_cast<bool>(summon);
                GIVM_ASSERT(valid);
                [[assume(valid)]];
                summon.state() = clamp_summon_state(change.state,
                    library[summon.definition_id()].query(summon_state_limit{}));
            }
            context.stack().pop<set_summon_state_input::change[]>();
            return context.enter_next();
        }
    }

    inline void compile(program_writer& writer, const givm::set_summon_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_summon_state_change<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_summon_state_change<false> });
    }
}

#include "../../macro_undef.hpp"
#endif
