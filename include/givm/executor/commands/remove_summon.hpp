#ifndef GIVM_EXECUTOR_COMMANDS_REMOVE_SUMMON_HPP
#define GIVM_EXECUTOR_COMMANDS_REMOVE_SUMMON_HPP

#include <algorithm>

#include "../broadcast.hpp"
#include "../../definition.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state broadcast_summon_removal(
        const definition_library&, unrestricted_table&, execution_context&, random_fn&);

    inline summon_id require_summon(
        const unrestricted_table& table, player_id player, definition_id<summon_view> definition)
    {
        auto summons = table[player].summons();
        const auto target = std::ranges::find_if(summons,
            [&](const auto entity) { return entity.definition_id() == definition; });
        const bool found = target != summons.end();
        GIVM_ASSERT(found);
        [[assume(found)]];
        return (*target).id();
    }

    inline execution_state remove_summon_and_broadcast(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random, summon_id summon)
    {
        const bool valid = static_cast<bool>(table[summon]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        table[summon].erase();
        prepare_broadcast(library, summon_removed{ summon }, table, context.stack(), context.position());
        return broadcast_summon_removal(library, table, context, random);
    }

    template<bool Fixed>
    execution_state prepare_summon_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        summon_id summon;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, remove_summon>(library);
            const auto player = command.player == relative_player::current
                ? table.state().active_player : other_player(table.state().active_player);
            summon = require_summon(table, player, command.definition);
            context.advance(instruction_extent<1, remove_summon>);
        }
        else
        {
            summon = get<0>(context.stack().top<summon_removal>()).summon;
            context.stack().pop<summon_removal>();
            context.enter_next();
        }
        return remove_summon_and_broadcast(library, table, context, random, summon);
    }

    inline execution_state broadcast_summon_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<summon_removed>(library, table, context, random))
            return continue_execution;
        pop_broadcast<summon_removed>(context);
        return context.enter_next();
    }

    inline void compile(program_writer& writer, const givm::remove_summon& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ prepare_summon_removal<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ prepare_summon_removal<false> });
        writer.write(execute_fn{ broadcast_summon_removal });
    }
}

#include "../../macro_undef.hpp"
#endif
