#ifndef GIVM_EXECUTOR_COMMANDS_MODIFY_SUMMON_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_MODIFY_SUMMON_STATE_HPP

#include <cstdint>

#include "set_summon_state.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<bool Fixed>
    execution_state execute_summon_state_modification(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        summon_state_modification input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, modify_summon_state>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            input = { require_summon(table, player, command.definition), command.value, command.usages };
            context.advance(instruction_extent<1, modify_summon_state>);
        }
        else
        {
            input = get<0>(context.stack().top<summon_state_modification>());
            context.stack().pop<summon_state_modification>();
            context.enter_next();
        }

        const bool valid = static_cast<bool>(table[input.summon]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        const auto current = table[input.summon].state();
        const auto limit = library[table[input.summon].definition_id()].query(summon_state_limit{});
        const auto add_saturated = [](std::uint32_t value, std::int64_t delta, std::uint32_t maximum)
        {
            const auto previous = static_cast<std::int64_t>(value);
            if(delta <= -previous)
                return std::uint32_t{};
            if(delta >= static_cast<std::int64_t>(maximum) - previous)
                return maximum;
            return static_cast<std::uint32_t>(previous + delta);
        };
        const summon_state state{
            add_saturated(current.value, input.value, limit.value),
            add_saturated(current.usages, input.usages, limit.usages)
        };
        return change_summon_state(library, table, context, random,
            summon_state_change{ input.summon, state });
    }

    inline void compile(program_writer& writer, const givm::modify_summon_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_summon_state_modification<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_summon_state_modification<false> });
        writer.write(execute_fn{ finish_summon_state_change });
        writer.write(execute_fn{ broadcast_summon_removal });
    }
}

#include "../../macro_undef.hpp"
#endif
