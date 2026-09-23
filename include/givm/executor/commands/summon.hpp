#ifndef GIVM_EXECUTOR_COMMANDS_SUMMON_HPP
#define GIVM_EXECUTOR_COMMANDS_SUMMON_HPP

#include "add_summon.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline program_entry prepare_summoning(
        const definition_library& library, unrestricted_table& table, execution_context& context,
        random_fn& random, summoning input, execution_position resume)
    {
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
                return {};
            resummoning event{ input.state };
            context.stack().push(response_return{ table.state().self_player, resume });
            auto response = context.make_handle_context(table, random);
            const auto entry = definition.handle<resummoning>(existing, event, response);
            if(not entry) context.stack().pop<response_return>();
            else table.state().self_player = existing.player().id();
            return entry;
        }

        if(summon_count >= player.state().summon_limit)
            return {};
        GIVM_ASSERT(input.state.usages != 0);
        [[assume(input.state.usages != 0)]];
        table[input.player].add(input.definition, input.state);
        return {};
    }

    inline execution_state finish_resummoning(
        const definition_library&, unrestricted_table&, execution_context& context, random_fn&)
    {
        context.stack().pop<response_return>();
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
                .player = command.player == relative_player::self
                    ? table.state().self_player : other_player(table.state().self_player),
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

        if(const auto entry = prepare_summoning(library, table, context, random, input, context.position()))
            return context.enter(entry);
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
