#ifndef GIVM_EXECUTOR_COMMANDS_SUMMON_HPP
#define GIVM_EXECUTOR_COMMANDS_SUMMON_HPP

#include "add_summon.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state finish_resummoning(
        const definition_library&, unrestricted_table&, execution_context& context, random_fn&)
    {
        context.stack().pop<execution_position>();
        return context.enter_next();
    }

    template<bool Fixed>
    execution_state execute_summon(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        summoning input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, summon>(library);
            input = {
                .player = command.player == relative_player::current
                    ? table.state().active_player : other_player(table.state().active_player),
                .definition = command.definition, .state = command.state
            };
            context.advance(instruction_extent<1, summon>);
        }
        else
        {
            input = get<0>(context.stack().top<summoning>());
            context.stack().pop<summoning>();
            context.enter_next();
        }

        const auto definition = library[input.definition];
        input.state = clamp_summon_state(input.state, definition.query(summon_state_limit{}));
        const auto player = std::as_const(table)[input.player];
        size_t summon_count = 0;
        for(const auto existing : player.summons())
        {
            ++summon_count;
            if(existing.definition_id() != input.definition)
                continue;
            if(not definition.can_handle<resummoning, summon_view>())
                return context.enter_next();
            resummoning event{ input.state };
            context.stack().push(context.position());
            auto response = context.make_handle_context(table, random);
            const auto entry = definition.handle<resummoning>(existing, event, response);
            if(entry)
                return context.enter(entry);
            return finish_resummoning(library, table, context, random);
        }

        if(summon_count >= player.state().summon_limit)
            return context.enter_next();
        GIVM_ASSERT(input.state.usages != 0);
        [[assume(input.state.usages != 0)]];
        table[input.player].add(input.definition, input.state);
        return context.enter_next();
    }

    inline void compile(program_writer& writer, const givm::summon& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_summon<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_summon<false> });
        writer.write(execute_fn{ finish_resummoning });
    }
}

#include "../../macro_undef.hpp"
#endif
