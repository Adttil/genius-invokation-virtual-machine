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

        enum class stage_type : stage_t
        {
            prepare,
            first,
            second,
            broadcast_player0,
            broadcast_player1
        };
        bool execute(card_table& table, execution_context& context, random_fn& random) const
        {
            const auto stage = static_cast<stage_type>(context.current_stage());
            if(stage == stage_type::broadcast_player0 || stage == stage_type::broadcast_player1)
            {
                if(not detail::continue_broadcast<active_character_changed>(table, context, random))
                {
                    return true;
                }

                detail::pop_broadcast<active_character_changed>(context);
                if(stage == stage_type::broadcast_player0)
                {
                    return true;
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
                return context.yield();
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
                return context.yield();
            }

            GIVM_ASSERT(stage == stage_type::second);
            const auto first_selection_copy = first_selection;
            const auto second_selection_copy = input_selection;
            GIVM_ASSERT(not (first_selection_copy.player_id == second_selection_copy.player_id));
            context.stack().pop<character_id, character_id, stage_t>();

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

            auto& player0_state = table[player_id{ 0 }].state();
            auto& player1_state = table[player_id{ 1 }].state();
            const active_character_changed player0_event{
                .current = player0_selection
            };
            const active_character_changed player1_event{
                .current = player1_selection
            };
            player0_state.active_character = player0_selection;
            player1_state.active_character = player1_selection;

            detail::prepare_broadcast(player1_event, table, context.stack());
            context.current_stage() = static_cast<stage_t>(stage_type::broadcast_player1);
            detail::prepare_broadcast(player0_event, table, context.stack());
            context.current_stage() = static_cast<stage_t>(stage_type::broadcast_player0);
            return true;
        }
    };
}

#include "../../macro_undef.hpp"
#endif
