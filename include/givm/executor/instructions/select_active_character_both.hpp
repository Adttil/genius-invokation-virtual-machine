#ifndef GIVM_EXECUTOR_INSTRUCTIONS_SELECT_ACTIVE_CHARACTER_BOTH_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_SELECT_ACTIVE_CHARACTER_BOTH_HPP

#include "../executor.hpp"
#include "../../definition/commands.hpp"

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
        context.stack().push(
            character_id{},
            character_id{ .player_id = player_id{ 0 }, .index = 0 }
        );
        return context.yield_next(execution_state::initial_active_character_selection);
    }

    inline execution_state accept_first_character_selection(
        const definition_library&, unrestricted_table&,
        execution_context& context, random_fn&
    )
    {
        auto&& [first_selection, input_selection] =
            context.stack().top<character_id, character_id>();
        first_selection = input_selection;
        input_selection = character_id{
            .player_id = other_player(first_selection.player_id),
            .index = 0
        };
        return context.yield_next(execution_state::remaining_active_character_selection);
    }

    inline execution_state prepare_initial_character_broadcasts(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        const auto [first, second] = context.stack().top<character_id, character_id>();
        const auto player0 = first.player_id == player_id{ 0 } ? first : second;
        const auto player1 = first.player_id == player_id{ 0 } ? second : first;
        context.stack().pop<character_id, character_id>();

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
        const auto [first, second] = context.stack().top<character_id, character_id>();
        GIVM_ASSERT(first.player_id != second.player_id);
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
