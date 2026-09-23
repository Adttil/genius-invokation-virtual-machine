#ifndef GIVM_EXECUTOR_COMMANDS_MODIFY_COMBAT_STATUS_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_MODIFY_COMBAT_STATUS_STATE_HPP

#include <cstdint>

#include "set_combat_status_state.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<bool Fixed>
    execution_state execute_combat_status_state_modification(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        combat_status_state_modification input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, modify_combat_status_state>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            input = {
                require_combat_status(table, player, command.definition), command.count, command.round_usages
            };
            context.advance(instruction_extent<1, modify_combat_status_state>);
        }
        else
        {
            input = get<0>(context.stack().top<combat_status_state_modification>());
            context.stack().pop<combat_status_state_modification>();
            context.enter_next();
        }

        const bool valid = static_cast<bool>(table[input.status]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        const auto current = table[input.status].state();
        const auto limit = library[table[input.status].definition_id()].query(combat_status_state_limit{});
        const auto add_saturated = [](std::uint32_t value, std::int64_t delta, std::uint32_t maximum)
        {
            const auto previous = static_cast<std::int64_t>(value);
            if(delta <= -previous)
                return std::uint32_t{};
            if(delta >= static_cast<std::int64_t>(maximum) - previous)
                return maximum;
            return static_cast<std::uint32_t>(previous + delta);
        };
        const combat_status_state state{
            add_saturated(current.count, input.count, limit.count),
            add_saturated(current.round_usages, input.round_usages, limit.round_usages)
        };
        return change_combat_status_state(library, table, context, random,
            combat_status_state_change{ input.status, state });
    }

    inline void compile(program_writer& writer, const givm::modify_combat_status_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_combat_status_state_modification<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_combat_status_state_modification<false> });
        writer.write(execute_fn{ finish_combat_status_state_change });
    }
}

#include "../../macro_undef.hpp"
#endif
