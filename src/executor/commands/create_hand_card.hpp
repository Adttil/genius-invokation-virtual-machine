#ifndef GIVM_EXECUTOR_COMMANDS_CREATE_HAND_CARD_HPP
#define GIVM_EXECUTOR_COMMANDS_CREATE_HAND_CARD_HPP

#include "../program_writer.hpp"

#include <vector>

#include "../broadcast.hpp"
#include <givm/definition.hpp>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

namespace givm::detail
{
    template<bool Fixed>
    inline execution_state create_hand_card_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        create_hand_card_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, create_hand_card>(library);
#ifndef NDEBUG
            debug_validate_entity(table, table.state().self_player, "create_hand_card", "self_player");
#endif
            input = {
                .player = command.player == relative_player::self
                    ? table.state().self_player : other_player(table.state().self_player),
                .definition = command.definition
            };
            context.advance(instruction_extent<1, create_hand_card>);
        }
        else
        {
            input = get<0>(context.stack().top<create_hand_card_input>());
#ifndef NDEBUG
            debug_validate_entity(table, input.player, "create_hand_card", "player");
            debug_validate_definition(library, input.definition, "create_hand_card", "definition");
#endif
            context.stack().pop<create_hand_card_input>();
            context.enter_next();
        }

        const auto player = table[input.player];
        const auto state = library[input.definition].query(card_initial_state{});
        const auto card = player.add_hand_card(input.definition, state);
        const bool overflow = player.hand_card_count() > player.state().hand_limit;
        const auto id = card.id();
        if(overflow) card.erase();
        record_hand_entry(context, id, hand_entry_kind::added, overflow);
        return continue_execution;
    }

    inline void compile(program_writer& writer, const create_hand_card& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ create_hand_card_execute<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ create_hand_card_execute<false> });
    }
}

namespace givm::detail
{
    inline std::vector<create_hand_card::error_type> check(const create_hand_card& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = create_hand_card::error_type::reason;
        std::vector<create_hand_card::error_type> errors;
        if(not command.definition)
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.player != relative_player::self && command.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_player, .value = static_cast<std::size_t>(command.player) });
        if(command.definition.value() >= context.definition_count<card_definition>())
            errors.push_back({ .cause = reason::invalid_definition, .value = command.definition.value(), .limit = context.definition_count<card_definition>() });
        return errors;
    }
}


namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const create_hand_card& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<create_hand_card::input_type>() : std::size_t(-1);
    }
}

#endif
