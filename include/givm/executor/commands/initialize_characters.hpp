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
            const auto definition = library[character.definition_id()];
            character.state() = definition.query(character_initial_state{});
            for(const auto skill : character.skills())
                skill.erase();
            for(std::size_t skill_index = 0; ; ++skill_index)
            {
                const auto skill = definition.query(character_initial_skill{ skill_index });
                if(not skill)
                    break;
                character.add(skill, {});
            }
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
