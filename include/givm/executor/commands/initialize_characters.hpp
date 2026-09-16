#ifndef GIVM_EXECUTOR_COMMANDS_INITIALIZE_CHARACTERS_HPP
#define GIVM_EXECUTOR_COMMANDS_INITIALIZE_CHARACTERS_HPP

#include "../executor.hpp"
#include "../../definition/commands.hpp"
#include "../../definition/events.hpp"
#include <utility>

namespace givm::detail
{
    inline execution_state initialize_characters_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        const auto& instruction = context.instruction_data<1, givm::initialize_characters>(library);
        for(const auto character : table[instruction.player].characters())
        {
            character_initialization event{};
            const auto entity = std::as_const(table)[character.id()];
            (void)library[entity.definition_id()].template handle<character_initialization>(
                entity,
                event,
                std::as_const(table),
                random
            );
            character.state() = event.state;
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
