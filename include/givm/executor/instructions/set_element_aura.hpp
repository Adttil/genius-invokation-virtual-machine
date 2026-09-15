#ifndef GIVM_EXECUTOR_INSTRUCTIONS_SET_ELEMENT_AURA_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_SET_ELEMENT_AURA_HPP

#include "../executor.hpp"

#include "../../table.hpp"
#include "../instruction.hpp"
#include "../../definition/commands.hpp"

namespace givm::detail
{
    namespace set_element_aura_command
    {
        inline execution_state execute(
            const definition_library& library,
            unrestricted_table& table, execution_context& context, random_fn&
        )
        {
            const auto& command = context.instruction_data<1, givm::set_element_aura>(library);
            table[command.target].state().aura = command.aura;
            return context.advance(instruction_extent<1, givm::set_element_aura>);
        }
    }

    inline void compile(program_writer& writer, const givm::set_element_aura& command, compile_mode)
    {
        writer.write(execute_fn{ &set_element_aura_command::execute });
        writer.write(command);
    }
}

#endif
