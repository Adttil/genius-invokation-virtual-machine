#ifndef GIVM_EXECUTOR_INSTRUCTIONS_SET_ACTIVE_CHARACTER_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_SET_ACTIVE_CHARACTER_HPP

#include "../executor.hpp"

#include "../broadcast.hpp"
#include "../events.hpp"

#include "../../utils/debug.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    struct set_active_character
    {
        using context_type = void;

        character_id target;
        enum class stage_type : stage_t
        {
            prepare,
            broadcast
        };

        bool execute(card_table& table, execution_context& context, random_fn& random) const
        {
            if(static_cast<stage_type>(context.current_stage()) == stage_type::prepare)
            {
                GIVM_ASSERT(static_cast<bool>(table[target]));

                auto& state = table[target.player_id].state();
                const active_character_changed event{
                    .current = target
                };
                state.active_character = target;

                detail::prepare_broadcast(event, table, context.stack());
                context.current_stage() = static_cast<stage_t>(stage_type::broadcast);
            }

            if(not detail::continue_broadcast<active_character_changed>(table, context, random))
            {
                return true;
            }

            detail::pop_broadcast<active_character_changed>(context);
            return context.enter_next();
        }
    };
}

#include "../../macro_undef.hpp"
#endif
