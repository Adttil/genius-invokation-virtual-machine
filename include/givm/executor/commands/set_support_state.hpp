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
        context.stack().pop<response_return>();
        return context.enter_next();
    }

    inline execution_state change_support_state(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random, const set_support_state_input& input)
    {
        support_state_changed event{ table[input.support].state(), input.state };
        table[input.support].state() = input.state;
        const auto support = std::as_const(table)[input.support];
        const auto definition = library[support.definition_id()];
        if(not definition.can_handle<support_state_changed, support_view>())
            return context.enter_next();
        context.stack().push(response_return{ table.state().self_player, context.position() });
        auto response = context.make_handle_context(table, random);
        const auto entry = definition.handle<support_state_changed>(support, event, response);
        if(entry)
        {
            table.state().self_player = support.player().id();
            return context.enter(entry);
        }
        return finish_support_state_change(library, table, context, random);
    }

    template<bool Fixed>
    inline execution_state execute_support_state_change(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        set_support_state_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, set_support_state>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            input = { require_support(table, player, command.definition), command.state };
            context.advance(instruction_extent<1, set_support_state>);
        }
        else
        {
            input = get<0>(context.stack().top<set_support_state_input>());
            context.stack().pop<set_support_state_input>();
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
