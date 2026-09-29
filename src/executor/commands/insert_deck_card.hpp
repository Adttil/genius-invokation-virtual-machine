#ifndef GIVM_EXECUTOR_COMMANDS_INSERT_DECK_CARD_HPP
#define GIVM_EXECUTOR_COMMANDS_INSERT_DECK_CARD_HPP

#include "../program_writer.hpp"

#include <vector>

#include <givm/executor/executor.hpp>
#include <givm/definition.hpp>
#include <cstddef>
#include <cstdint>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <givm/macro_define.hpp>

namespace givm::detail
{
    inline execution_state insert_deck_card_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        const auto& instruction = context.instruction_data<1, givm::insert_deck_card>(library);
#ifndef NDEBUG
        debug_validate_entity(table, instruction.player, "insert_deck_card", "player");
        debug_validate_definition(library, instruction.definition, "insert_deck_card", "definition");
#endif
        auto player_entity = table[instruction.player];
        const auto size = player_entity.deck_card_count();
#ifndef NDEBUG
        const auto requested = instruction.position >= 0 ? static_cast<std::uint64_t>(instruction.position)
            : static_cast<std::uint64_t>(-static_cast<std::int64_t>(instruction.position) - 1);
        if(requested > size)
            throw command_input_error{ "insert_deck_card", invalid_numeric_argument{ "position", requested, size } };
        if(instruction.position == std::numeric_limits<std::int32_t>::min())
            throw command_input_error{ "insert_deck_card", invalid_numeric_argument{ "position", requested, std::numeric_limits<std::int32_t>::max() - 1u } };
#endif
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

namespace givm::detail
{
    inline std::vector<insert_deck_card::error_type> check(const insert_deck_card& command,
        const definition_compile_context& context, program_kind)
    {
        using reason = insert_deck_card::error_type::reason;
        std::vector<insert_deck_card::error_type> errors;
        if(command.player.index >= 2)
            errors.push_back({ .cause = reason::invalid_player, .value = command.player.index });
        if(command.definition.value() >= context.definition_count<card_definition>())
            errors.push_back({ .cause = reason::invalid_definition, .value = command.definition.value(), .limit = context.definition_count<card_definition>() });
        return errors;
    }
}

#include <givm/macro_undef.hpp>
#endif
