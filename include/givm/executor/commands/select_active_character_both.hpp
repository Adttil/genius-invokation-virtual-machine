#ifndef GIVM_EXECUTOR_COMMANDS_SELECT_ACTIVE_CHARACTER_BOTH_HPP
#define GIVM_EXECUTOR_COMMANDS_SELECT_ACTIVE_CHARACTER_BOTH_HPP

#include "../executor.hpp"
#include "../../definition/commands.hpp"

#include <cstddef>

#include "../broadcast.hpp"
#include "../../definition/events.hpp"

#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state prepare_initial_character_selection(
        const definition_library&, unrestricted_table&,
        execution_context& context, random_fn&
    )
    {
        context.stack().push(std::size_t{ 0 }, character_id{});
        return context.yield_next(execution_state::initial_active_character_selection);
    }

    inline execution_state accept_first_character_selection(
        const definition_library&, unrestricted_table&,
        execution_context& context, random_fn&
    )
    {
        return context.yield_next(execution_state::remaining_active_character_selection);
    }

    inline execution_state prepare_initial_character_broadcasts(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        const auto input = context.stack().top<std::size_t, character_id>();
        const auto first = get<1>(input);
        const character_id second{ other_player(first.player_id), get<0>(input) };
        const auto player0 = first.player_id == player_id{ 0 } ? first : second;
        const auto player1 = first.player_id == player_id{ 0 } ? second : first;
        context.stack().pop<std::size_t, character_id>();

        prepare_broadcast(library, active_character_changed{ .current = player1 }, table, context.stack());
        prepare_broadcast(library, active_character_changed{ .current = player0 }, table, context.stack());
        return context.enter_next();
    }

    template<bool Observed>
    inline execution_state apply_initial_character_selections(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        const auto input = context.stack().top<std::size_t, character_id>();
        const auto first = get<1>(input);
        const character_id second{ other_player(first.player_id), get<0>(input) };
        const auto player0 = first.player_id == player_id{ 0 } ? first : second;
        const auto player1 = first.player_id == player_id{ 0 } ? second : first;
        GIVM_ASSERT(static_cast<bool>(table[player0]));
        GIVM_ASSERT(static_cast<bool>(table[player1]));
        table[player_id{ 0 }].state().active_character = player0;
        table[player_id{ 1 }].state().active_character = player1;

        if constexpr(Observed)
        {
            return context.yield_next(execution_state::initial_active_characters_selected);
        }
        else
        {
            return prepare_initial_character_broadcasts(library, table, context, random);
        }
    }

    inline execution_state broadcast_initial_active_character(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<active_character_changed>(library, table, context, random))
        {
            return continue_execution;
        }

        pop_broadcast<active_character_changed>(context);
        return context.enter_next();
    }

    inline void compile(program_writer& writer, const givm::select_active_character_both&, compile_mode mode)
    {
        writer.write(execute_fn{ &prepare_initial_character_selection });
        writer.write(execute_fn{ &accept_first_character_selection });
        if(mode == compile_mode::observed)
        {
            writer.write(execute_fn{ &apply_initial_character_selections<true> });
            writer.write(execute_fn{ &prepare_initial_character_broadcasts });
        }
        else
        {
            writer.write(execute_fn{ &apply_initial_character_selections<false> });
        }
        writer.write(execute_fn{ &broadcast_initial_active_character });
        writer.write(execute_fn{ &broadcast_initial_active_character });
    }
}

#include "../../macro_undef.hpp"
#endif
