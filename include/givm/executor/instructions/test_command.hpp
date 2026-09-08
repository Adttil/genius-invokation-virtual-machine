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

        enum class stage_type : stage_t
        {
            prepare,
            broadcast
        };

        bool execute(card_table& table, execution_context& context, random_fn& random) const
        {
            if(static_cast<stage_type>(context.current_stage()) == stage_type::prepare)
            {
                detail::prepare_broadcast(test_event{}, table, context.stack());
                context.current_stage() = static_cast<stage_t>(stage_type::broadcast);
            }

            if(not detail::continue_broadcast<test_event>(table, context, random))
            {
                return true;
            }

            detail::pop_broadcast<test_event>(context);
            return context.enter_next();
        }
    };
}

#endif
