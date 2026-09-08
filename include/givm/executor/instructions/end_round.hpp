#ifndef GIVM_EXECUTOR_INSTRUCTIONS_END_ROUND_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_END_ROUND_HPP

#include "../executor.hpp"

#include "../broadcast.hpp"
#include "../events.hpp"

namespace givm
{
    struct end_round
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
                auto& state = table.state();
                state.active_player = other_player(state.active_player);
                state.first_ended = false;

                detail::prepare_broadcast(round_ended{}, table, context.stack());
                context.current_stage() = static_cast<stage_t>(stage_type::broadcast);
            }

            if(not detail::continue_broadcast<round_ended>(table, context, random))
            {
                return true;
            }

            detail::pop_broadcast<round_ended>(context);

            return context.enter_next();
        }
    };
}

#endif
