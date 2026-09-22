#ifndef GIVM_EXECUTOR_COMMANDS_GENERATE_COMBAT_STATUS_HPP
#define GIVM_EXECUTOR_COMMANDS_GENERATE_COMBAT_STATUS_HPP

#include "add_combat_status.hpp"

namespace givm::detail
{
    inline program_entry prepare_combat_status_generation(
        const definition_library& library, unrestricted_table& table, execution_context& context,
        random_fn& random, combat_status_generation input, execution_position resume)
    {
        const auto definition = library[input.definition];
        input.state = clamp_combat_status_state(input.state, definition.query(combat_status_state_limit{}));
        for(const auto existing : std::as_const(table)[input.player].combat_statuses())
        {
            if(existing.definition_id() != input.definition)
                continue;
            if(not definition.can_handle<combat_status_regeneration, combat_status_view>())
                return {};
            combat_status_regeneration event{ input.state };
            context.stack().push(resume);
            auto response = context.make_handle_context(table, random);
            const auto entry = definition.handle<combat_status_regeneration>(existing, event, response);
            if(not entry) context.stack().pop<execution_position>();
            return entry;
        }

        table[input.player].add(input.definition, input.state);
        return {};
    }

    inline execution_state finish_combat_status_regeneration(
        const definition_library&, unrestricted_table&, execution_context& context, random_fn&)
    {
        context.stack().pop<execution_position>();
        return context.enter_next();
    }

    template<bool Fixed>
    execution_state execute_combat_status_generation(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        combat_status_generation input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, generate_combat_status>(library);
            input = {
                .player = command.player == relative_player::current
                    ? table.state().active_player : other_player(table.state().active_player),
                .definition = command.definition, .state = command.state
            };
            context.advance(instruction_extent<1, generate_combat_status>);
        }
        else
        {
            input = get<0>(context.stack().top<combat_status_generation>());
            context.stack().pop<combat_status_generation>();
            context.enter_next();
        }

        if(const auto entry = prepare_combat_status_generation(library, table, context, random, input, context.position()))
            return context.enter(entry);
        return context.enter_next();
    }

    inline void compile(program_writer& writer, const givm::generate_combat_status& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_combat_status_generation<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_combat_status_generation<false> });
        writer.write(execute_fn{ finish_combat_status_regeneration });
    }
}

#endif
