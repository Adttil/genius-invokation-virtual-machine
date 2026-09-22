#ifndef GIVM_EXECUTOR_COMMANDS_SET_COMBAT_STATUS_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_COMBAT_STATUS_STATE_HPP

#include "add_combat_status.hpp"
#include "remove_combat_status.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state finish_combat_status_state_change(
        const definition_library&, unrestricted_table&, execution_context& context, random_fn&)
    {
        context.stack().pop<execution_position>();
        return context.enter_next();
    }

    inline execution_state change_combat_status_state(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random, const combat_status_state_change& input)
    {
        combat_status_state_changed event{ table[input.status].state(), input.state };
        table[input.status].state() = input.state;
        const auto status = std::as_const(table)[input.status];
        const auto definition = library[status.definition_id()];
        if(not definition.can_handle<combat_status_state_changed, combat_status_view>())
            return context.enter_next();
        context.stack().push(context.position());
        auto response = context.make_handle_context(table, random);
        const auto entry = definition.handle<combat_status_state_changed>(status, event, response);
        if(entry)
            return context.enter(entry);
        return finish_combat_status_state_change(library, table, context, random);
    }

    template<bool Fixed>
    execution_state execute_combat_status_state_change(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        combat_status_state_change input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, set_combat_status_state>(library);
            const auto player = command.player == relative_player::current
                ? table.state().active_player : other_player(table.state().active_player);
            input = { require_combat_status(table, player, command.definition), command.state };
            context.advance(instruction_extent<1, set_combat_status_state>);
        }
        else
        {
            input = get<0>(context.stack().top<combat_status_state_change>());
            context.stack().pop<combat_status_state_change>();
            context.enter_next();
        }
        const bool valid = static_cast<bool>(table[input.status]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        const auto definition = library[table[input.status].definition_id()];
        input.state = clamp_combat_status_state(input.state, definition.query(combat_status_state_limit{}));
        return change_combat_status_state(library, table, context, random, input);
    }

    inline void compile(program_writer& writer, const givm::set_combat_status_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_combat_status_state_change<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_combat_status_state_change<false> });
        writer.write(execute_fn{ finish_combat_status_state_change });
    }
}

#include "../../macro_undef.hpp"
#endif
