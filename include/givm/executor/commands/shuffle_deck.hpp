#ifndef GIVM_EXECUTOR_COMMANDS_SHUFFLE_DECK_HPP
#define GIVM_EXECUTOR_COMMANDS_SHUFFLE_DECK_HPP

#include "../executor.hpp"
#include "../../definition/commands.hpp"
#include <cstddef>
#include <cstdint>

namespace givm::detail
{
    inline execution_state shuffle_deck_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        const auto& instruction = context.instruction_data<1, givm::shuffle_deck>(library);
        const auto target = table[instruction.player];
        for(size_t remaining = target.deck_card_count(); remaining > 1; --remaining)
        {
            const size_t selected = static_cast<size_t>(
                static_cast<std::uint64_t>(random()) * remaining >> 32
            );
            target.swap_deck_cards(remaining - 1, selected);
        }
        return context.advance(instruction_extent<1, givm::shuffle_deck>);
    }

    inline void compile(program_writer& writer, const givm::shuffle_deck& command, compile_mode)
    {
        writer.write(execute_fn{ &shuffle_deck_execute });
        writer.write(command);
    }
}

#endif
