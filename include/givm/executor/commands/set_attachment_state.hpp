#ifndef GIVM_EXECUTOR_COMMANDS_SET_ATTACHMENT_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_ATTACHMENT_STATE_HPP

#include <utility>

#include "add_attachment.hpp"
#include "remove_attachment.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state finish_attachment_state_change(
        const definition_library&, unrestricted_table&, execution_context& context, random_fn&)
    {
        context.stack().pop<response_return>();
        return context.enter_next();
    }

    inline execution_state change_attachment_state(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random, attachment_id id, attachment_state state)
    {
        const auto attachment = table[id];
        attachment_state_changed event{ attachment.state(), state };
        attachment.state() = state;
        const auto definition = library[attachment.definition_id()];
        if(not definition.can_handle<attachment_state_changed, attachment_view>())
            return context.enter_next();
        context.stack().push(response_return{ table.state().self_player, context.position() });
        auto response = context.make_handle_context(table, random);
        const auto entry = definition.handle<attachment_state_changed>(std::as_const(table)[id], event, response);
        if(entry)
        {
            table.state().self_player = attachment.player().id();
            return context.enter(entry);
        }
        return finish_attachment_state_change(library, table, context, random);
    }

    template<bool Fixed>
    execution_state execute_attachment_state_change(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        attachment_state_change input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, set_attachment_state>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            input = { require_attachment(table, player, command.definition), command.state };
            context.advance(instruction_extent<1, set_attachment_state>);
        }
        else
        {
            input = get<0>(context.stack().top<attachment_state_change>());
            context.stack().pop<attachment_state_change>();
            context.enter_next();
        }
        const auto attachment = table[input.attachment];
        const bool valid = static_cast<bool>(attachment);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        input.state = clamp_attachment_state(input.state, library[attachment.definition_id()].query(attachment_state_limit{}));
        return change_attachment_state(library, table, context, random, input.attachment, input.state);
    }

    inline void compile(program_writer& writer, const givm::set_attachment_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_attachment_state_change<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_attachment_state_change<false> });
        writer.write(execute_fn{ finish_attachment_state_change });
    }
}

#include "../../macro_undef.hpp"
#endif
