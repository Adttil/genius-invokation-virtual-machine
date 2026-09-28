#ifndef GIVM_EXECUTOR_COMMANDS_ENTER_CHARACTER_HPP
#define GIVM_EXECUTOR_COMMANDS_ENTER_CHARACTER_HPP

#include <vector>

#include "../executor.hpp"
#include "../../definition.hpp"

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

namespace givm::detail
{
    inline execution_state enter_character_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        const auto& instruction = context.instruction_data<1, givm::enter_character>(library);
#ifndef NDEBUG
        debug_validate_entity(table, instruction.player, "enter_character", "player");
        debug_validate_definition(library, instruction.definition, "enter_character", "definition");
#endif
        const auto definition = library[instruction.definition];
        const auto state = definition.query(character_initial_state{});
        const auto character = table[instruction.player].add(instruction.definition, state);
        for(std::size_t skill_index = 0; ; ++skill_index)
        {
            const auto skill = definition.query(character_initial_skill{ skill_index });
            if(not skill)
                break;
#ifndef NDEBUG
            debug_validate_definition(library, skill, "enter_character", "initial_skill");
#endif
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

namespace givm
{
    inline std::vector<enter_character::error_type> check(const enter_character& command,
        const definition_compile_context& context, program_kind)
    {
        using reason = enter_character::error_type::reason;
        std::vector<enter_character::error_type> errors;
        if(command.player.index >= 2)
            errors.push_back({ .cause = reason::invalid_player, .value = command.player.index });
        if(command.definition.value() >= context.definition_count<character_view>())
            errors.push_back({ .cause = reason::invalid_definition, .value = command.definition.value(), .limit = context.definition_count<character_view>() });
        return errors;
    }
}

#endif
