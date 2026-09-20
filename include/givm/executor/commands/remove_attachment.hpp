#ifndef GIVM_EXECUTOR_COMMANDS_REMOVE_ATTACHMENT_HPP
#define GIVM_EXECUTOR_COMMANDS_REMOVE_ATTACHMENT_HPP

#include "../broadcast.hpp"
#include "../../definition.hpp"

namespace givm::detail
{
    inline execution_state prepare_attachment_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        const auto event = get<0>(context.stack().top<attachment_removal>());
        context.stack().pop<attachment_removal>();
        prepare_broadcast(library, entity_will_leave{ event.attachment }, table, context.stack(),
            context.position() + sizeof(execute_fn));
        return context.enter_next();
    }

    inline execution_state apply_attachment_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<entity_will_leave>(library, table, context, random))
            return continue_execution;
        const auto attachment = std::get<attachment_id>(get<0>(context.stack().top<
            entity_will_leave, execution_position>()).entity);
        pop_broadcast<entity_will_leave>(context);
        if(not table[attachment])
            return context.advance(2 * sizeof(execute_fn));
        table[attachment].erase();
        prepare_broadcast(library, entity_left{ attachment }, table, context.stack(),
            context.position() + sizeof(execute_fn));
        return context.enter_next();
    }

    inline execution_state broadcast_attachment_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<entity_left>(library, table, context, random))
            return continue_execution;
        pop_broadcast<entity_left>(context);
        return context.enter_next();
    }

    inline void compile(program_writer& writer, const givm::remove_attachment&, compile_mode)
    {
        writer.write(execute_fn{ prepare_attachment_removal });
        writer.write(execute_fn{ apply_attachment_removal });
        writer.write(execute_fn{ broadcast_attachment_removal });
    }
}

#endif
