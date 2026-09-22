#ifndef GIVM_EXECUTOR_COMMANDS_MODIFY_SUPPORT_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_MODIFY_SUPPORT_STATE_HPP

#include <cstdint>

#include "set_support_state.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<bool Fixed>
    inline execution_state execute_support_state_modification(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        support_state_modification input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, modify_support_state>(library);
            const auto player = command.player == relative_player::current
                ? table.state().active_player : other_player(table.state().active_player);
            input = { require_support(table, player, command.definition), command.count, command.round_usages };
            context.advance(instruction_extent<1, modify_support_state>);
        }
        else
        {
            input = get<0>(context.stack().top<support_state_modification>());
            context.stack().pop<support_state_modification>();
            context.enter_next();
        }

        const bool valid = static_cast<bool>(table[input.support]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        const auto current = table[input.support].state();
        const auto limit = library[table[input.support].definition_id()].query(support_state_limit{});
        const auto add_saturated = [](std::uint32_t value, std::int64_t delta, std::uint32_t maximum)
        {
            const auto previous = static_cast<std::int64_t>(value);
            if(delta <= -previous)
                return std::uint32_t{};
            if(delta >= static_cast<std::int64_t>(maximum) - previous)
                return maximum;
            return static_cast<std::uint32_t>(previous + delta);
        };
        const support_state state{
            add_saturated(current.count, input.count, limit.count),
            add_saturated(current.round_usages, input.round_usages, limit.round_usages)
        };
        return change_support_state(library, table, context, random, support_state_change{ input.support, state });
    }

    inline void compile(program_writer& writer, const givm::modify_support_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_support_state_modification<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_support_state_modification<false> });
        writer.write(execute_fn{ finish_support_state_change });
    }
}

#include "../../macro_undef.hpp"
#endif
