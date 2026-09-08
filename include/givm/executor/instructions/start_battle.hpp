#ifndef GIVM_EXECUTOR_INSTRUCTIONS_START_BATTLE_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_START_BATTLE_HPP

#include "../executor.hpp"

#include "../broadcast.hpp"
#include "../events.hpp"

namespace givm
{
    struct start_battle
    {
        using context_type = void;

        enum class stage_type : stage_t
        {
            prepare,
            broadcast
        };

        bool execute(card_table& table, execution_context& context, random_fn& random) const
        {
            if(static_cast<stage_type>(context.current_stage()) == stage_type::prepare)
            {
                if(table.state().round_number != 1)
                {
                    return context.enter_next();
                }
                detail::prepare_broadcast(battle_started{}, table, context.stack());
                context.current_stage() = static_cast<stage_t>(stage_type::broadcast);
            }

            if(not detail::continue_broadcast<battle_started>(table, context, random))
            {
                return true;
            }

            detail::pop_broadcast<battle_started>(context);
            return context.enter_next();
        }
    };
}

#endif
