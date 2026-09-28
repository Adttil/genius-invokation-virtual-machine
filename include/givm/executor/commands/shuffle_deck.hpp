#ifndef GIVM_EXECUTOR_COMMANDS_SHUFFLE_DECK_HPP
#define GIVM_EXECUTOR_COMMANDS_SHUFFLE_DECK_HPP

#include <vector>

#include "../executor.hpp"
#include "../../definition.hpp"
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

namespace givm
{
    inline std::vector<shuffle_deck::error_type> check(const shuffle_deck& command, const definition_compile_context&, program_kind)
    {
        using reason = shuffle_deck::error_type::reason;
        std::vector<shuffle_deck::error_type> errors;
        if(command.player.index >= 2)
            errors.push_back({ .cause = reason::invalid_player, .value = command.player.index });
        return errors;
    }
}

#endif
