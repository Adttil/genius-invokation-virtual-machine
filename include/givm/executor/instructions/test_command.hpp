#ifndef GIVM_EXECUTOR_INSTRUCTIONS_TEST_COMMAND_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_TEST_COMMAND_HPP

#include "../executor.hpp"

#include "../broadcast.hpp"
#include "../events.hpp"

namespace givm
{
    struct test_command
    {
        using context_type = void;
    };

    namespace detail
    {
        template<>
        struct instruction_implementation<test_command>
        {
            enum class stage_type : stage_t
            {
                prepare,
                broadcast
            };

            template<bool Observed>
            static execution_state execute(
                const givm::test_command& instruction, const definition_library& library,
                card_table& table, execution_context& context, random_fn& random
            )
            {
                if(static_cast<stage_type>(context.current_stage()) == stage_type::prepare)
                {
                    detail::prepare_broadcast(library, test_event{}, table, context.stack());
                    context.current_stage() = static_cast<stage_t>(stage_type::broadcast);
                }

                if(not detail::continue_broadcast<test_event>(library, table, context, random))
                {
                    return continue_execution;
                }

                detail::pop_broadcast<test_event>(context);
                return context.enter_next();
            }

        };
    }
}

#endif
