#ifndef GIVM_EXECUTOR_COMMANDS_SHUFFLE_DECK_HPP
#define GIVM_EXECUTOR_COMMANDS_SHUFFLE_DECK_HPP

#include "../program_writer.hpp"

#include <vector>

#include <givm/executor/executor.hpp>
#include <givm/definition.hpp>
#include <cstddef>
#include <cstdint>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

namespace givm::detail
{
    inline execution_state shuffle_deck_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        const auto& instruction = context.instruction_data<1, givm::shuffle_deck>(library);
#ifndef NDEBUG
        debug_validate_entity(table, instruction.player, "shuffle_deck", "player");
#endif
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

namespace givm::detail
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
