#ifndef GIVM_EXECUTOR_COMMANDS_REMOVE_COMBAT_STATUS_HPP
#define GIVM_EXECUTOR_COMMANDS_REMOVE_COMBAT_STATUS_HPP

#include <algorithm>

#include "../broadcast.hpp"
#include "../../definition.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state broadcast_combat_status_removal(
        const definition_library&, unrestricted_table&, execution_context&, random_fn&);

    inline combat_status_id require_combat_status(
        const unrestricted_table& table, player_id player, definition_id<combat_status_view> definition)
    {
        auto statuses = table[player].combat_statuses();
        const auto target = std::ranges::find_if(statuses,
            [&](const auto entity) { return entity.definition_id() == definition; });
        const bool found = target != statuses.end();
        GIVM_ASSERT(found);
        [[assume(found)]];
        return (*target).id();
    }

    template<bool Fixed>
    execution_state prepare_combat_status_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        combat_status_id status;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, remove_combat_status>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            status = require_combat_status(table, player, command.definition);
            context.advance(instruction_extent<1, remove_combat_status>);
        }
        else
        {
            status = get<0>(context.stack().top<combat_status_removal>()).status;
            context.stack().pop<combat_status_removal>();
            context.enter_next();
        }
        const bool valid = static_cast<bool>(table[status]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        table[status].erase();
        prepare_broadcast(library, combat_status_removed{ status }, table, context.stack(), context.position());
        return broadcast_combat_status_removal(library, table, context, random);
    }

    inline execution_state broadcast_combat_status_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<combat_status_removed>(library, table, context, random))
            return continue_execution;
        pop_broadcast<combat_status_removed>(context);
        return context.enter_next();
    }

    inline void compile(program_writer& writer, const givm::remove_combat_status& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ prepare_combat_status_removal<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ prepare_combat_status_removal<false> });
        writer.write(execute_fn{ broadcast_combat_status_removal });
    }
}

#include "../../macro_undef.hpp"
#endif
