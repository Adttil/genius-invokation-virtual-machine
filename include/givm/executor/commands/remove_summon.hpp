#ifndef GIVM_EXECUTOR_COMMANDS_REMOVE_SUMMON_HPP
#define GIVM_EXECUTOR_COMMANDS_REMOVE_SUMMON_HPP

#include <algorithm>
#include <cstdint>
#include <type_traits>

#include "../broadcast.hpp"
#include "../../definition.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
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

    inline execution_state broadcast_summon_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<summon_removed>(library, table, context, random))
            return continue_execution;
        pop_broadcast<summon_removed>(context);
        return context.enter_next();
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

    template<bool Modified>
    using summon_removal_input_frame = std::conditional_t<Modified,
        frame_t<summon_id[], std::int64_t, std::int64_t>, frame_t<summon_id[]>>;

    template<bool Modified>
    inline bool prepare_next_summon_removal(
        const definition_library& library, unrestricted_table& table, execution_context& context)
    {
        const auto [input, progress] = context.stack().top<
            summon_removal_input_frame<Modified>{}, frame_t<stack_count_t>{}>();
        const auto summons = get<0>(input);
        auto& cursor = get<0>(progress);
        while(cursor != summons.size())
        {
            const auto id = summons[cursor++];
            auto summon = table[id];
            if(not summon) continue;
            if constexpr(Modified)
            {
                if(summon.state().usages != 0
                    || not library.remove_at_zero_usages(summon.definition_id()))
                    continue;
            }
            summon.erase();
            prepare_broadcast(library, summon_removed{ id }, table, context.stack(), context.position());
            return true;
        }
        return false;
    }

    template<bool Modified>
    inline execution_state broadcast_summon_removals(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        for(;;)
        {
            if(not continue_broadcast<summon_removed>(library, table, context, random))
                return continue_execution;
            pop_broadcast<summon_removed>(context);
            if(not prepare_next_summon_removal<Modified>(library, table, context))
            {
                context.stack().pop<summon_removal_input_frame<Modified>{}, frame_t<stack_count_t>{}>();
                return context.enter_next();
            }
        }
    }

    template<bool Fixed>
    inline execution_state prepare_summon_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, remove_summon>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            const auto summon = require_summon(table, player, command.definition);
            context.advance(instruction_extent<1, remove_summon>);
            return remove_summon_and_broadcast(library, table, context, random, summon);
        }
        else
        {
            const auto summons = get<0>(context.stack().top<summon_id[]>());
            GIVM_ASSERT(std::ranges::all_of(summons,
                [&](summon_id id) { return static_cast<bool>(table[id]); }));
            context.enter_next();
            if(summons.empty())
            {
                context.stack().pop<summon_id[]>();
                return context.enter_next();
            }
            const auto first = summons.front();
            context.stack().push(stack_count_t{ 1 });
            table[first].erase();
            prepare_broadcast(library, summon_removed{ first }, table, context.stack(), context.position());
            return broadcast_summon_removals<false>(library, table, context, random);
        }
    }

    inline void compile(program_writer& writer, const givm::remove_summon& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ prepare_summon_removal<true> });
            writer.write(command);
            writer.write(execute_fn{ broadcast_summon_removal });
        }
        else
        {
            writer.write(execute_fn{ prepare_summon_removal<false> });
            writer.write(execute_fn{ broadcast_summon_removals<false> });
        }
    }
}

#include "../../macro_undef.hpp"
#endif
