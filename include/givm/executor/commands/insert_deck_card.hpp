#ifndef GIVM_EXECUTOR_COMMANDS_INSERT_DECK_CARD_HPP
#define GIVM_EXECUTOR_COMMANDS_INSERT_DECK_CARD_HPP

#include "../executor.hpp"
#include "../../definition/commands.hpp"
#include <cstddef>
#include <cstdint>

#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state insert_deck_card_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        const auto& instruction = context.instruction_data<1, givm::insert_deck_card>(library);
        auto player_entity = table[instruction.player];
        const auto size = player_entity.deck_card_count();
        size_t index;
        if(instruction.position >= 0)
        {
            index = static_cast<size_t>(instruction.position);
            GIVM_ASSERT(index <= size);
        }
        else
        {
            const auto offset_from_top = static_cast<size_t>(-instruction.position - 1);
            GIVM_ASSERT(offset_from_top <= size);
            index = size - offset_from_top;
        }
        const auto state = library[instruction.definition].query(card_initial_state{});
        player_entity.insert_deck_card(index, instruction.definition, state);

        return context.advance(instruction_extent<1, givm::insert_deck_card>);
    }

    inline void compile(program_writer& writer, const givm::insert_deck_card& command, compile_mode)
    {
        writer.write(execute_fn{ &insert_deck_card_execute });
        writer.write(command);
    }
}

#include "../../macro_undef.hpp"
#endif
