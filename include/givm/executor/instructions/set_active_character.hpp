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
    };

    namespace detail
    {
        template<>
        struct instruction_implementation<set_active_character>
        {
            enum class stage_type : stage_t
            {
                prepare,
                broadcast,
                apply
            };

            template<bool Observed>
            static execution_state execute(
                const givm::set_active_character& instruction, const definition_library& library,
                unrestricted_table& table, execution_context& context, random_fn& random
            )
            {
                if constexpr(Observed)
                {
                    if(static_cast<stage_type>(context.current_stage()) == stage_type::apply)
                    {
                        auto&& [event, handler, stage] = context.stack().top<
                            active_character_changed, detail::handler_id<active_character_changed>, stage_t
                        >();
                        table[event.current.player_id].state().active_character = event.current;
                        stage = static_cast<stage_t>(stage_type::broadcast);
                    }
                }
                if(static_cast<stage_type>(context.current_stage()) == stage_type::prepare)
                {
                    GIVM_ASSERT(static_cast<bool>(table[instruction.target]));

                    auto& state = table[instruction.target.player_id].state();
                    const active_character_changed event{
                        .current = instruction.target
                    };
                    if constexpr(Observed)
                    {
                        if(state.active_character != instruction.target)
                        {
                            detail::prepare_broadcast(library, event, table, context.stack());
                            context.current_stage() = static_cast<stage_t>(stage_type::apply);
                            return execution_state::active_character_changed;
                        }
                    }
                    state.active_character = instruction.target;

                    detail::prepare_broadcast(library, event, table, context.stack());
                    context.current_stage() = static_cast<stage_t>(stage_type::broadcast);
                }

                if(not detail::continue_broadcast<active_character_changed>(library, table, context, random))
                {
                    return continue_execution;
                }

                detail::pop_broadcast<active_character_changed>(context);
                return context.enter_next();
            }

        };
    }
}

#include "../../macro_undef.hpp"
#endif
