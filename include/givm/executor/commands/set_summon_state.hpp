#ifndef GIVM_EXECUTOR_COMMANDS_SET_SUMMON_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_SUMMON_STATE_HPP

#include "add_summon.hpp"
#include "remove_summon.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state finish_summon_state_change(
        const definition_library&, unrestricted_table&, execution_context& context, random_fn&)
    {
        context.stack().pop<response_return>();
        return context.enter_next();
    }

    inline execution_state change_summon_state(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random, const summon_state_change& input)
    {
        summon_state_changed event{ table[input.summon].state(), input.state };
        table[input.summon].state() = input.state;
        const auto summon = std::as_const(table)[input.summon];
        const auto definition = library[summon.definition_id()];
        if(not definition.can_handle<summon_state_changed, summon_view>())
            return context.enter_next();
        context.stack().push(response_return{ table.state().self_player, context.position() });
        auto response = context.make_handle_context(table, random);
        const auto entry = definition.handle<summon_state_changed>(summon, event, response);
        if(entry)
        {
            table.state().self_player = summon.player().id();
            return context.enter(entry);
        }
        return finish_summon_state_change(library, table, context, random);
    }

    template<bool Fixed>
    inline execution_state execute_summon_state_change(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        summon_state_change input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, set_summon_state>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            input = { require_summon(table, player, command.definition), command.state };
            context.advance(instruction_extent<1, set_summon_state>);
        }
        else
        {
            input = get<0>(context.stack().top<summon_state_change>());
            context.stack().pop<summon_state_change>();
            context.enter_next();
        }

        const bool valid = static_cast<bool>(table[input.summon]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        const auto definition = library[table[input.summon].definition_id()];
        input.state = clamp_summon_state(input.state, definition.query(summon_state_limit{}));
        return change_summon_state(library, table, context, random, input);
    }

    inline void compile(program_writer& writer, const givm::set_summon_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_summon_state_change<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_summon_state_change<false> });
        writer.write(execute_fn{ finish_summon_state_change });
    }
}

#include "../../macro_undef.hpp"
#endif
