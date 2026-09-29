#ifndef GIVM_EXECUTOR_COMMANDS_GENERATE_COMBAT_STATUS_HPP
#define GIVM_EXECUTOR_COMMANDS_GENERATE_COMBAT_STATUS_HPP

#include "../program_writer.hpp"

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include "add_combat_status.hpp"

namespace givm::detail
{
    inline program_entry prepare_combat_status_generation(
        const definition_library& library, unrestricted_table& table, execution_context& context,
        random_fn& random, generate_combat_status_input input, execution_position resume)
    {
#ifndef NDEBUG
        debug_validate_entity(table, input.player, "generate_combat_status", "player");
        debug_validate_definition(library, input.definition, "generate_combat_status", "definition");
#endif
        const auto definition = library[input.definition];
        input.state = clamp_combat_status_state(input.state, definition.query(combat_status_state_limit{}));
        for(const auto existing : std::as_const(table)[input.player].combat_statuses())
        {
            if(existing.definition_id() != input.definition)
                continue;
            if(not definition.can_handle<combat_status_regeneration, combat_status_view>())
                return {};
            combat_status_regeneration event{ input.state };
            context.stack().push(response_return{ table.state().self_player, resume });
            auto response = context.make_handle_context(library, table, random);
            const auto entry = definition.handle<combat_status_regeneration>(existing, event, response);
            if(not entry) context.stack().pop<response_return>();
            else table.state().self_player = existing.player().id();
            return entry;
        }

        table[input.player].add(input.definition, input.state);
        return {};
    }

    inline execution_state finish_combat_status_regeneration(
        const definition_library&, unrestricted_table&, execution_context& context, random_fn&)
    {
        context.stack().pop<response_return>();
        return context.enter_next();
    }

    template<bool Fixed>
    execution_state execute_combat_status_generation(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        generate_combat_status_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, generate_combat_status>(library);
            input = {
                .player = command.player == relative_player::self
                    ? table.state().self_player : other_player(table.state().self_player),
                .definition = command.definition, .state = command.state
            };
            context.advance(instruction_extent<1, generate_combat_status>);
        }
        else
        {
            input = get<0>(context.stack().top<generate_combat_status_input>());
            context.stack().pop<generate_combat_status_input>();
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

namespace givm::detail
{
    inline std::vector<generate_combat_status::error_type> check(const generate_combat_status& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = generate_combat_status::error_type::reason;
        std::vector<generate_combat_status::error_type> errors;
        if(not command.definition)
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.player != relative_player::self && command.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_player, .value = static_cast<std::size_t>(command.player) });
        if(command.definition.value() >= context.definition_count<combat_status_view>())
            errors.push_back({ .cause = reason::invalid_definition, .value = command.definition.value(), .limit = context.definition_count<combat_status_view>() });
        return errors;
    }
}


#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const generate_combat_status& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<generate_combat_status::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
