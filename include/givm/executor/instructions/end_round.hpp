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
    };

    namespace detail
    {
        template<>
        struct instruction_implementation<end_round>
        {
            enum class stage_type : stage_t
            {
                prepare,
                broadcast,
                start
            };

            template<bool Observed>
            static execution_state execute(
                const givm::end_round& instruction, const definition_library& library,
                card_table& table, execution_context& context, random_fn& random
            )
            {
                auto stage = static_cast<stage_type>(context.current_stage());
                if constexpr(Observed)
                {
                    if(stage == stage_type::prepare)
                    {
                        context.current_stage() = static_cast<stage_t>(stage_type::start);
                        return execution_state::round_ending;
                    }
                    if(stage == stage_type::start)
                    {
                        stage = stage_type::prepare;
                    }
                }

                if(stage == stage_type::prepare)
                {
                    auto& state = table.state();
                    state.active_player = other_player(state.active_player);
                    state.first_ended = false;

                    detail::prepare_broadcast(library, round_ended{}, table, context.stack());
                    context.current_stage() = static_cast<stage_t>(stage_type::broadcast);
                }

                if(not detail::continue_broadcast<round_ended>(library, table, context, random))
                {
                    return continue_execution;
                }

                detail::pop_broadcast<round_ended>(context);

                return context.enter_next();
            }

        };
    }
}

#endif
