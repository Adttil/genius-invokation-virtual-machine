#ifndef GIVM_EXECUTOR_COMMANDS_ATTACH_HPP
#define GIVM_EXECUTOR_COMMANDS_ATTACH_HPP

#include <utility>

#include "add_attachment.hpp"

namespace givm::detail
{
    inline execution_state finish_attachment_reapplication(
        const definition_library&, unrestricted_table&, execution_context& context, random_fn&)
    {
        context.stack().pop<execution_position>();
        return context.advance(2 * sizeof(execute_fn));
    }

    template<bool Fixed>
    execution_state execute_attachment_application(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        attachment_application input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, attach>(library);
            const auto player = command.player == relative_player::current
                ? table.state().active_player : other_player(table.state().active_player);
            input = { *table[player].state().active_character, command.definition, command.state };
            context.advance(instruction_extent<1, attach>);
        }
        else
        {
            input = get<0>(context.stack().top<attachment_application>());
            context.stack().pop<attachment_application>();
            context.enter_next();
        }
        const auto definition = library[input.definition];
        input.state = clamp_attachment_state(input.state, definition.query(attachment_state_limit{}));
        for(const auto attachment : std::as_const(table)[input.target].attachments())
        {
            if(attachment.definition_id() != input.definition)
                continue;
            if(not definition.can_handle<attachment_reapplication, attachment_view>())
                return context.advance(2 * sizeof(execute_fn));
            context.stack().push(context.position());
            attachment_reapplication event{ input.state };
            auto response = context.make_handle_context(table, random);
            const auto entry = definition.handle<attachment_reapplication>(attachment, event, response);
            if(entry)
                return context.enter(entry);
            return finish_attachment_reapplication(library, table, context, random);
        }
        context.stack().push(attachment_addition{ input.target, input.definition, input.state });
        context.enter_next();
        return apply_attachment_addition(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::attach& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_attachment_application<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_attachment_application<false> });
        writer.write(execute_fn{ finish_attachment_reapplication });
        writer.write(execute_fn{ finish_replaced_attachment_removal });
    }
}

#endif
