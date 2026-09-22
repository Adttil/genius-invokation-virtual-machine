#ifndef GIVM_EXECUTOR_COMMANDS_ADD_ATTACHMENT_HPP
#define GIVM_EXECUTOR_COMMANDS_ADD_ATTACHMENT_HPP

#include <algorithm>

#include "../broadcast.hpp"
#include "../../definition.hpp"

namespace givm::detail
{
    inline attachment_state clamp_attachment_state(attachment_state state, attachment_state limit) noexcept
    {
        return { std::min(state.count, limit.count), std::min(state.round_usages, limit.round_usages) };
    }

    inline execution_state finish_replaced_attachment_removal(
        const definition_library&, unrestricted_table&, execution_context&, random_fn&);

    inline execution_state apply_attachment_addition(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        const auto input = get<0>(context.stack().top<attachment_addition>());
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
                prepare_broadcast(library, attachment_removed{ old }, table, context.stack(), context.position());
                return finish_replaced_attachment_removal(library, table, context, random);
            }
            character.add(input.definition, input.state, type);
        }
        context.stack().pop<attachment_addition>();
        return context.enter_next();
    }

    inline execution_state finish_replaced_attachment_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<attachment_removed>(library, table, context, random))
            return continue_execution;
        pop_broadcast<attachment_removed>(context);
        // A response may have installed another equipment of this category.
        return apply_attachment_addition(library, table, context, random);
    }

    template<bool Fixed>
    execution_state execute_attachment_addition(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, add_attachment>(library);
            const auto player = command.player == relative_player::current
                ? table.state().active_player : other_player(table.state().active_player);
            context.stack().push(attachment_addition{
                .target = *table[player].state().active_character,
                .definition = command.definition, .state = command.state
            });
            context.advance(instruction_extent<1, add_attachment>);
        }
        else
            context.enter_next();
        auto& input = get<0>(context.stack().top<attachment_addition>());
        input.state = clamp_attachment_state(input.state, library[input.definition].query(attachment_state_limit{}));
        return apply_attachment_addition(library, table, context, random);
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
