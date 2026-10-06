#ifndef GIVM_EXECUTOR_COMMANDS_SETTLE_HPP
#define GIVM_EXECUTOR_COMMANDS_SETTLE_HPP

#include <givm/definition.hpp>
#include <givm/executor/command_input_error.hpp>
#include "../program_writer.hpp"
#include "../settlement.hpp"

namespace givm::detail
{
    inline execution_state execute_settle(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        return begin_settlement(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::settle&, compile_mode)
    {
        compile_settlement(writer, execute_settle);
    }
}

#endif
