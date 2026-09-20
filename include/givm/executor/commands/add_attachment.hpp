#ifndef GIVM_EXECUTOR_COMMANDS_ADD_ATTACHMENT_HPP
#define GIVM_EXECUTOR_COMMANDS_ADD_ATTACHMENT_HPP

#include "../broadcast.hpp"
#include "../../definition.hpp"

namespace givm::detail
{
    inline execution_state apply_attachment_addition(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        const auto event = get<0>(context.stack().top<attachment_addition>());
        const auto character = table[event.target];
        const auto type = library.equipment_type(event.definition);
        attachment_id attachment;
        if(type == equipment_type::none)
        {
            attachment = character.add(event.definition, event.state).id();
        }
        else
        {
            if(character.has(type))
            {
                const auto old = character.get(type).id();
                prepare_broadcast(library, entity_will_leave{ old }, table, context.stack(),
                    context.position() + sizeof(execute_fn));
                return context.enter_next();
            }
            attachment = character.add(event.definition, event.state, type).id();
        }

        context.stack().pop<attachment_addition>();
        prepare_broadcast(library, attachment_added{ attachment }, table, context.stack(),
            context.position() + 3 * sizeof(execute_fn));
        return context.advance(3 * sizeof(execute_fn));
    }

    inline execution_state remove_replaced_attachment(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<entity_will_leave>(library, table, context, random))
            return continue_execution;
        const auto old = std::get<attachment_id>(get<0>(context.stack().top<
            entity_will_leave, execution_position>()).entity);
        pop_broadcast<entity_will_leave>(context);
        // A nested response may already have removed or replaced this equipment.
        if(not table[old])
            return context.jump(context.position() - sizeof(execute_fn));
        table[old].erase();
        prepare_broadcast(library, entity_left{ old }, table, context.stack(),
            context.position() + sizeof(execute_fn));
        return context.enter_next();
    }

    inline execution_state finish_replaced_attachment_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<entity_left>(library, table, context, random))
            return continue_execution;
        pop_broadcast<entity_left>(context);
        // A response may have installed another equipment of this category.
        return context.jump(context.position() - 2 * sizeof(execute_fn));
    }

    inline execution_state broadcast_attachment_addition(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<attachment_added>(library, table, context, random))
            return continue_execution;
        pop_broadcast<attachment_added>(context);
        return context.enter_next();
    }

    inline execution_state prepare_active_character_attachment(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        const auto& command = context.instruction_data<1, add_attachment>(library);
        const auto player = command.player == relative_player::current
            ? table.state().active_player : other_player(table.state().active_player);
        context.stack().push(attachment_addition{
            .target = *table[player].state().active_character,
            .definition = command.definition, .state = command.state
        });
        return context.advance(instruction_extent<1, add_attachment>);
    }

    inline void compile(program_writer& writer, const givm::add_attachment& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ prepare_active_character_attachment });
            writer.write(command);
        }
        writer.write(execute_fn{ apply_attachment_addition });
        writer.write(execute_fn{ remove_replaced_attachment });
        writer.write(execute_fn{ finish_replaced_attachment_removal });
        writer.write(execute_fn{ broadcast_attachment_addition });
    }
}

#endif
