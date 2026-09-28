#ifndef GIVM_EXECUTOR_COMMANDS_ADD_SUMMON_HPP
#define GIVM_EXECUTOR_COMMANDS_ADD_SUMMON_HPP

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <algorithm>

#include "../executor.hpp"
#include "../../definition.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    constexpr summon_state clamp_summon_state(summon_state state, summon_state limit) noexcept
    {
        return { std::min(state.value, limit.value), std::min(state.usages, limit.usages) };
    }

    template<bool Fixed>
    execution_state apply_summon_addition(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        add_summon_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, add_summon>(library);
            input = {
                .player = command.player == relative_player::self
                    ? table.state().self_player : other_player(table.state().self_player),
                .definition = command.definition, .state = command.state
            };
            context.advance(instruction_extent<1, add_summon>);
        }
        else
        {
            input = get<0>(context.stack().top<add_summon_input>());
            context.stack().pop<add_summon_input>();
            context.enter_next();
        }
#ifndef NDEBUG
        debug_validate_entity(table, input.player, "add_summon", "player");
        debug_validate_definition(library, input.definition, "add_summon", "definition");
#endif
        const auto player = std::as_const(table)[input.player];
        auto remaining_capacity = player.state().summon_limit;
        if(remaining_capacity == 0)
            return continue_execution;
        for([[maybe_unused]] const auto existing : player.summons())
        {
            if(--remaining_capacity == 0)
                return continue_execution;
        }
        input.state = clamp_summon_state(input.state, library[input.definition].query(summon_state_limit{}));
        table[input.player].add(input.definition, input.state);
        return continue_execution;
    }

    inline void compile(program_writer& writer, const givm::add_summon& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ apply_summon_addition<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ apply_summon_addition<false> });
    }
}

namespace givm
{
    inline std::vector<add_summon::error_type> check(const add_summon& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = add_summon::error_type::reason;
        std::vector<add_summon::error_type> errors;
        if(not command.definition)
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.player != relative_player::self && command.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_player, .value = static_cast<std::size_t>(command.player) });
        if(command.definition.value() >= context.definition_count<summon_view>())
            errors.push_back({ .cause = reason::invalid_definition, .value = command.definition.value(), .limit = context.definition_count<summon_view>() });
        return errors;
    }
}

#include "../../macro_undef.hpp"
#endif
