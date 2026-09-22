#ifndef GIVM_EXECUTOR_COMMANDS_SET_SUPPORT_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_SUPPORT_STATE_HPP

#include "add_support.hpp"
#include "remove_support.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state finish_support_state_change(
        const definition_library&, unrestricted_table&, execution_context& context, random_fn&)
    {
        context.stack().pop<execution_position>();
        return context.enter_next();
    }

    inline execution_state change_support_state(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random, const support_state_change& input)
    {
        support_state_changed event{ table[input.support].state(), input.state };
        table[input.support].state() = input.state;
        const auto support = std::as_const(table)[input.support];
        const auto definition = library[support.definition_id()];
        if(not definition.can_handle<support_state_changed, support_view>())
            return context.enter_next();
        context.stack().push(context.position());
        auto response = context.make_handle_context(table, random);
        const auto entry = definition.handle<support_state_changed>(support, event, response);
        if(entry)
            return context.enter(entry);
        return finish_support_state_change(library, table, context, random);
    }

    template<bool Fixed>
    inline execution_state execute_support_state_change(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        support_state_change input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, set_support_state>(library);
            const auto player = command.player == relative_player::current
                ? table.state().active_player : other_player(table.state().active_player);
            input = { require_support(table, player, command.definition), command.state };
            context.advance(instruction_extent<1, set_support_state>);
        }
        else
        {
            input = get<0>(context.stack().top<support_state_change>());
            context.stack().pop<support_state_change>();
            context.enter_next();
        }
        const bool valid = static_cast<bool>(table[input.support]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        const auto definition = library[table[input.support].definition_id()];
        input.state = clamp_support_state(input.state, definition.query(support_state_limit{}));
        return change_support_state(library, table, context, random, input);
    }

    inline void compile(program_writer& writer, const givm::set_support_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_support_state_change<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_support_state_change<false> });
        writer.write(execute_fn{ finish_support_state_change });
    }
}

#include "../../macro_undef.hpp"
#endif
