#ifndef GIVM_EXECUTOR_COMMANDS_INITIALIZE_CHARACTERS_HPP
#define GIVM_EXECUTOR_COMMANDS_INITIALIZE_CHARACTERS_HPP

#include "../executor.hpp"
#include "../../definition.hpp"

namespace givm::detail
{
    inline execution_state initialize_characters_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        const auto& instruction = context.instruction_data<1, givm::initialize_characters>(library);
        for(const auto character : table[instruction.player].characters())
        {
            character.state() = library[character.definition_id()].query(character_initial_state{});
        }
        return context.advance(instruction_extent<1, givm::initialize_characters>);
    }

    inline void compile(program_writer& writer, const givm::initialize_characters& command, compile_mode)
    {
        writer.write(execute_fn{ &initialize_characters_execute });
        writer.write(command);
    }
}

#endif
