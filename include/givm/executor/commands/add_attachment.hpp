#ifndef GIVM_EXECUTOR_COMMANDS_ADD_ATTACHMENT_HPP
#define GIVM_EXECUTOR_COMMANDS_ADD_ATTACHMENT_HPP

#include <algorithm>
#include <optional>
#include <utility>

#include "../broadcast.hpp"
#include "../../definition.hpp"

namespace givm::detail
{
    inline attachment_state clamp_attachment_state(attachment_state state, attachment_state limit) noexcept
    {
        return { std::min(state.count, limit.count), std::min(state.round_usages, limit.round_usages) };
    }

    inline std::optional<execution_state> continue_attachment_replacement(
        const definition_library&, unrestricted_table&, execution_context&, random_fn&);

    inline std::optional<execution_state> prepare_attachment_addition(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random, attachment_addition input, execution_position resume)
    {
        const auto character = table[input.target];
        const auto type = library.equipment_type(input.definition);
        if(type == equipment_type::none)
            character.add(input.definition, input.state);
        else
        {
            if(character.has(type))
            {
                const auto old = character.get(type).id();
                table[old].erase();
                context.stack().push(input);
                prepare_broadcast(library, attachment_removed{ old }, table, context.stack(), resume);
                return continue_attachment_replacement(library, table, context, random);
            }
            character.add(input.definition, input.state, type);
        }
        return std::nullopt;
    }

    inline std::optional<execution_state> continue_attachment_replacement(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<attachment_removed>(library, table, context, random))
            return continue_execution;
        const auto resume = get<1>(context.stack().top<attachment_removed, response_return>()).position;
        pop_broadcast<attachment_removed>(context);
        const auto input = get<0>(context.stack().top<attachment_addition>());
        context.stack().pop<attachment_addition>();
        // Removal responses can add immunity or install another equipment.
        if(library.is_control(input.definition) && library.is_control_immune(std::as_const(table)[input.target]))
            return std::nullopt;
        return prepare_attachment_addition(library, table, context, random, input, resume);
    }

    inline execution_state finish_replaced_attachment_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(const auto state = continue_attachment_replacement(library, table, context, random)) return *state;
        return context.enter_next();
    }

    template<bool Fixed>
    execution_state execute_attachment_addition(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        attachment_addition input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, add_attachment>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            input = {
                .target = *table[player].state().active_character,
                .definition = command.definition, .state = command.state
            };
            context.advance(instruction_extent<1, add_attachment>);
        }
        else
        {
            input = get<0>(context.stack().top<attachment_addition>());
            context.stack().pop<attachment_addition>();
            context.enter_next();
        }
        if(library.is_control(input.definition) && library.is_control_immune(std::as_const(table)[input.target]))
            return context.enter_next();
        input.state = clamp_attachment_state(input.state, library[input.definition].query(attachment_state_limit{}));
        if(const auto state = prepare_attachment_addition(library, table, context, random, input, context.position()))
            return *state;
        return context.enter_next();
    }

    inline void compile(program_writer& writer, const givm::add_attachment& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_attachment_addition<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_attachment_addition<false> });
        writer.write(execute_fn{ finish_replaced_attachment_removal });
    }
}

#endif
