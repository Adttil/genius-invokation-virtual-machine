#ifndef GIVM_EXECUTOR_INSTRUCTIONS_ENTER_CHARACTER_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_ENTER_CHARACTER_HPP

#include "../executor.hpp"
#include "../../definition/commands.hpp"
#include "../../definition/events.hpp"
#include <utility>

namespace givm::detail
{
    inline execution_state enter_character_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        const auto& instruction = context.instruction_data<1, givm::enter_character>(library);
        const auto character = table[instruction.player].add(instruction.definition, character_state{}).id();
        character_initialization event{};
        const auto character_entity = std::as_const(table)[character];
        (void)library[character_entity.definition_id()].template handle<character_initialization>(
            character_entity,
            event,
            std::as_const(table),
            random
        );
        table[character].state() = event.state;

        return context.advance(instruction_extent<1, givm::enter_character>);
    }

    inline void compile(program_writer& writer, const givm::enter_character& command, compile_mode)
    {
        writer.write(execute_fn{ &enter_character_execute });
        writer.write(command);
    }
}

#endif
