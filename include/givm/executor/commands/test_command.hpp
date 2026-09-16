#ifndef GIVM_EXECUTOR_COMMANDS_TEST_COMMAND_HPP
#define GIVM_EXECUTOR_COMMANDS_TEST_COMMAND_HPP
#include "../broadcast.hpp"
#include "../../definition/commands.hpp"
namespace givm::detail
{
    inline execution_state execute_test_broadcast(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<test_event>(library, table, context, random))
            return continue_execution;
        pop_broadcast<test_event>(context);
        return context.enter_next();
    }
    inline execution_state execute_test_command(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        prepare_broadcast(library, test_event{}, table, context.stack());
        return context.enter_next();
    }
    inline void compile(program_writer& writer, const test_command&, compile_mode)
    {
        writer.write(execute_fn{ execute_test_command });
        writer.write(execute_fn{ execute_test_broadcast });
    }
}
#endif
