#ifndef GIVM_EXECUTOR_COMMANDS_ATTACH_HPP
#define GIVM_EXECUTOR_COMMANDS_ATTACH_HPP

#include <utility>

#include "add_attachment.hpp"

namespace givm::detail
{
    inline std::optional<execution_state> prepare_attachment_application(
        const definition_library& library, unrestricted_table& table, execution_context& context,
        random_fn& random, attach_input input, execution_position reapplication_resume,
        execution_position replacement_resume)
    {
        if(library.is_control(input.definition) && library.is_control_immune(std::as_const(table)[input.target]))
            return std::nullopt;
        const auto definition = library[input.definition];
        input.state = clamp_attachment_state(input.state, definition.query(attachment_state_limit{}));
        for(const auto attachment : std::as_const(table)[input.target].attachments())
        {
            if(attachment.definition_id() != input.definition) continue;
            if(not definition.can_handle<attachment_reapplication, attachment_view>()) return std::nullopt;
            context.stack().push(response_return{ table.state().self_player, reapplication_resume });
            attachment_reapplication event{ input.state };
            auto response = context.make_handle_context(table, random);
            const auto entry = definition.handle<attachment_reapplication>(attachment, event, response);
            if(entry)
            {
                table.state().self_player = attachment.player().id();
                return context.enter(entry);
            }
            context.stack().pop<response_return>();
            return std::nullopt;
        }
        return prepare_attachment_addition(library, table, context, random,
            { input.target, input.definition, input.state }, replacement_resume);
    }

    inline execution_state finish_attachment_reapplication(
        const definition_library&, unrestricted_table&, execution_context& context, random_fn&)
    {
        context.stack().pop<response_return>();
        return context.advance(2 * sizeof(execute_fn));
    }

    template<bool Fixed>
    execution_state execute_attachment_application(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        attach_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, attach>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            input = { *table[player].state().active_character, command.definition, command.state };
            context.advance(instruction_extent<1, attach>);
        }
        else
        {
            input = get<0>(context.stack().top<attach_input>());
            context.stack().pop<attach_input>();
            context.enter_next();
        }
        if(const auto state = prepare_attachment_application(library, table, context, random, input,
            context.position(), context.position() + sizeof(execute_fn))) return *state;
        return context.advance(2 * sizeof(execute_fn));
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
