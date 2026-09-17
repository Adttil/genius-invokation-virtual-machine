#ifndef GIVM_EXECUTOR_COMMANDS_ENTER_CHARACTER_HPP
#define GIVM_EXECUTOR_COMMANDS_ENTER_CHARACTER_HPP

#include "../executor.hpp"
#include "../../definition.hpp"

namespace givm::detail
{
    inline execution_state enter_character_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        const auto& instruction = context.instruction_data<1, givm::enter_character>(library);
        const auto definition = library[instruction.definition];
        const auto state = definition.query(character_initial_state{});
        const auto character = table[instruction.player].add(instruction.definition, state);
        for(std::size_t skill_index = 0; ; ++skill_index)
        {
            const auto skill = definition.query(character_initial_skill{ skill_index });
            if(not skill)
                break;
            character.add(skill, {});
        }

        return context.advance(instruction_extent<1, givm::enter_character>);
    }

    inline void compile(program_writer& writer, const givm::enter_character& command, compile_mode)
    {
        writer.write(execute_fn{ &enter_character_execute });
        writer.write(command);
    }
}

#endif
