#ifndef GIVM_EXECUTOR_COMMANDS_ADD_COMBAT_STATUS_HPP
#define GIVM_EXECUTOR_COMMANDS_ADD_COMBAT_STATUS_HPP

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <algorithm>

#include "../executor.hpp"
#include "../../definition.hpp"

namespace givm::detail
{
    constexpr combat_status_state clamp_combat_status_state(
        combat_status_state state, combat_status_state limit) noexcept
    {
        return { std::min(state.count, limit.count), std::min(state.round_usages, limit.round_usages) };
    }

    template<bool Fixed>
    execution_state apply_combat_status_addition(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        add_combat_status_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, add_combat_status>(library);
            input = {
                .player = command.player == relative_player::self
                    ? table.state().self_player : other_player(table.state().self_player),
                .definition = command.definition, .state = command.state
            };
            context.advance(instruction_extent<1, add_combat_status>);
        }
        else
        {
            input = get<0>(context.stack().top<add_combat_status_input>());
            context.stack().pop<add_combat_status_input>();
            context.enter_next();
        }
#ifndef NDEBUG
        debug_validate_entity(table, input.player, "add_combat_status", "player");
        debug_validate_definition(library, input.definition, "add_combat_status", "definition");
#endif
        input.state = clamp_combat_status_state(
            input.state, library[input.definition].query(combat_status_state_limit{}));
        table[input.player].add(input.definition, input.state);
        return continue_execution;
    }

    inline void compile(program_writer& writer, const givm::add_combat_status& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ apply_combat_status_addition<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ apply_combat_status_addition<false> });
    }
}

namespace givm
{
    inline std::vector<add_combat_status::error_type> check(const add_combat_status& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = add_combat_status::error_type::reason;
        std::vector<add_combat_status::error_type> errors;
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

#endif
