#ifndef GIVM_EXECUTOR_INSTRUCTIONS_SELECT_ACTIVE_CHARACTER_BOTH_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_SELECT_ACTIVE_CHARACTER_BOTH_HPP

#include "../executor.hpp"

#include <cstdint>

#include "../broadcast.hpp"
#include "../events.hpp"

#include "../../utils/debug.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    struct select_active_character_both
    {
        using context_type = void;

    };

    template<>
    struct detail::instruction_implementation<select_active_character_both>
    {
        enum class stage_type : stage_t
        {
            prepare,
            first,
            second,
            broadcast_player0,
            broadcast_player1,
            selections_applied
        };
        template<bool Observed>
        static execution_state execute(
            const givm::select_active_character_both& instruction,
            const definition_library& library,
            card_table& table,
            execution_context& context,
            random_fn& random
        )
        {
            const auto stage = static_cast<stage_type>(context.current_stage());
            if constexpr(Observed)
            {
                if(stage == stage_type::selections_applied)
                {
                    const auto [first, second, stored_stage] =
                        context.stack().top<character_id, character_id, stage_t>();
                    const auto player0 = first.player_id == player_id{ 0 } ? first : second;
                    const auto player1 = first.player_id == player_id{ 0 } ? second : first;
                    context.stack().pop<character_id, character_id, stage_t>();
                    return prepare_broadcasts(library, table, context, player0, player1);
                }
            }
            if(stage == stage_type::broadcast_player0 || stage == stage_type::broadcast_player1)
            {
                if(not detail::continue_broadcast<active_character_changed>(library, table, context, random))
                {
                    return continue_execution;
                }

                detail::pop_broadcast<active_character_changed>(context);
                if(stage == stage_type::broadcast_player0)
                {
                    return continue_execution;
                }

                return context.enter_next();
            }

            if(stage == stage_type::prepare)
            {
                context.stack().push(
                    character_id{},
                    character_id{ .player_id = player_id{ 0 }, .index = 0 },
                    static_cast<stage_t>(stage_type::first)
                );
                return context.yield(execution_state::initial_active_character_selection);
            }

            auto&& [first_selection, input_selection, stored_stage] =
                context.stack().top<character_id, character_id, stage_t>();
            if(stage == stage_type::first)
            {
                first_selection = input_selection;
                input_selection = character_id{
                    .player_id = other_player(first_selection.player_id),
                    .index = 0
                };
                stored_stage = static_cast<stage_t>(stage_type::second);
                return context.yield(execution_state::remaining_active_character_selection);
            }

            GIVM_ASSERT(stage == stage_type::second);
            const auto first_selection_copy = first_selection;
            const auto second_selection_copy = input_selection;
            GIVM_ASSERT(not (first_selection_copy.player_id == second_selection_copy.player_id));

            const auto player0_selection = first_selection_copy.player_id == player_id{ 0 }
                ? first_selection_copy
                : second_selection_copy;
            const auto player1_selection = first_selection_copy.player_id == player_id{ 0 }
                ? second_selection_copy
                : first_selection_copy;
            GIVM_ASSERT(player0_selection.player_id == player_id{ 0 });
            GIVM_ASSERT(player1_selection.player_id == player_id{ 1 });

            GIVM_ASSERT(static_cast<bool>(table[player0_selection]));
            GIVM_ASSERT(static_cast<bool>(table[player1_selection]));

            table[player_id{ 0 }].state().active_character = player0_selection;
            table[player_id{ 1 }].state().active_character = player1_selection;
            if constexpr(Observed)
            {
                stored_stage = static_cast<stage_t>(stage_type::selections_applied);
                return execution_state::initial_active_characters_selected;
            }
            context.stack().pop<character_id, character_id, stage_t>();
            return prepare_broadcasts(library, table, context, player0_selection, player1_selection);
        }

    private:
        static execution_state prepare_broadcasts(
            const definition_library& library,
            card_table& table,
            execution_context& context,
            character_id player0_selection,
            character_id player1_selection
        )
        {
            const active_character_changed player0_event{
                .current = player0_selection
            };
            const active_character_changed player1_event{
                .current = player1_selection
            };

            detail::prepare_broadcast(library, player1_event, table, context.stack());
            context.current_stage() = static_cast<stage_t>(stage_type::broadcast_player1);
            detail::prepare_broadcast(library, player0_event, table, context.stack());
            context.current_stage() = static_cast<stage_t>(stage_type::broadcast_player0);
            return continue_execution;
        }
    };
}

#include "../../macro_undef.hpp"
#endif
