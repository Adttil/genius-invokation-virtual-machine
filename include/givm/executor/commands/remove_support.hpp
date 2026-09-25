#ifndef GIVM_EXECUTOR_COMMANDS_REMOVE_SUPPORT_HPP
#define GIVM_EXECUTOR_COMMANDS_REMOVE_SUPPORT_HPP

#include <algorithm>

#include "../broadcast.hpp"
#include "../../definition.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state broadcast_support_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<support_removed>(library, table, context, random))
            return continue_execution;
        pop_broadcast<support_removed>(context);
        return context.enter_next();
    }

    inline support_id require_support(
        const unrestricted_table& table, player_id player, definition_id<support_view> definition)
    {
        auto supports = table[player].supports();
        const auto target = std::ranges::find_if(supports,
            [&](const auto entity) { return entity.definition_id() == definition; });
        const bool found = target != supports.end();
        GIVM_ASSERT(found);
        [[assume(found)]];
        return (*target).id();
    }

    template<bool Fixed>
    inline execution_state prepare_support_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        support_id support;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, remove_support>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            support = require_support(table, player, command.definition);
            context.advance(instruction_extent<1, remove_support>);
        }
        else
        {
            support = get<0>(context.stack().top<remove_support_input>()).support;
            context.stack().pop<remove_support_input>();
            context.enter_next();
        }
        const bool valid = static_cast<bool>(table[support]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        table[support].erase();
        prepare_broadcast(library, support_removed{ support }, table, context.stack(), context.position());
        return broadcast_support_removal(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::remove_support& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ prepare_support_removal<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ prepare_support_removal<false> });
        writer.write(execute_fn{ broadcast_support_removal });
    }
}

#include "../../macro_undef.hpp"
#endif
