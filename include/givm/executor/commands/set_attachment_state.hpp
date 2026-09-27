#ifndef GIVM_EXECUTOR_COMMANDS_SET_ATTACHMENT_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_ATTACHMENT_STATE_HPP

#include <utility>

#include "add_attachment.hpp"
#include "remove_attachment.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<class Selector>
    struct attachment_state_change_data
    {
        fixed_attachment_target<Selector> target;
        attachment_state state;
    };

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

    template<class Selector = void>
    execution_state execute_attachment_state_change(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        attachment_id id;
        attachment_state state;
        if constexpr(not std::is_void_v<Selector>)
        {
            const auto& data = context.instruction_data<1, attachment_state_change_data<Selector>>(library);
            id = require_attachment(library, table, data.target);
            state = data.state;
            context.advance(instruction_extent<1, attachment_state_change_data<Selector>>);
        }
        else
        {
            const auto& input = get<0>(context.stack().top<set_attachment_state_input>());
            id = require_attachment(table, input.attachment);
            state = input.state;
            context.stack().pop<set_attachment_state_input>();
            context.enter_next();
        }
        const auto attachment = table[id];
        const bool valid = static_cast<bool>(attachment);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        state = clamp_attachment_state(state, library[attachment.definition_id()].query(attachment_state_limit{}));
        return change_attachment_state(library, table, context, random, id, state);
    }

    inline void compile(program_writer& writer, const givm::set_attachment_state& command, compile_mode)
    {
        std::visit([&](auto selector)
        {
            using selector_type = decltype(selector);
            if constexpr(std::is_same_v<selector_type, definition_id<attachment_view>>)
            {
                if(not selector)
                {
                    writer.write(execute_fn{ execute_attachment_state_change<> });
                    return;
                }
            }
            else GIVM_ASSERT(selector != equipment_type::none);
            GIVM_ASSERT(command.target.character.selection == character_selection::character);
            writer.write(execute_fn{ execute_attachment_state_change<selector_type> });
            writer.write(attachment_state_change_data<selector_type>{
                { command.target.character, selector }, command.state });
        }, command.target.selector);
        writer.write(execute_fn{ finish_attachment_state_change });
    }
}

#include "../../macro_undef.hpp"
#endif
