#ifndef GIVM_EXECUTOR_COMMANDS_SET_COMBAT_STATUS_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_COMBAT_STATUS_STATE_HPP

#include "../program_writer.hpp"

#include <algorithm>
#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include "add_combat_status.hpp"
#include "remove_combat_status.hpp"
#include <givm/macro_define.hpp>

namespace givm::detail
{
    struct combat_status_state_change_data
    {
        relative_player player;
        definition_id<combat_status_view> definition;
        combat_status_state state;
    };

    inline execution_state change_combat_status_state(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random, const set_combat_status_state_input& input)
    {
        combat_status_state_changed event{ table[input.status].state(), input.state };
        table[input.status].state() = input.state;
        const auto status = std::as_const(table)[input.status];
        const auto definition = library[status.definition_id()];
        if(not definition.can_handle<combat_status_state_changed, combat_status_view>())
            return continue_execution;
        append_single_event_record(context, input.status, event);
        return continue_execution;
    }

    template<bool Fixed, bool IgnoreLimit>
    inline execution_state execute_combat_status_state_change(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        set_combat_status_state_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, combat_status_state_change_data>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
#ifndef NDEBUG
            debug_validate_required_entity(table, player, command.definition, "set_combat_status_state", "status");
#endif
            input = { require_combat_status(table, player, command.definition), command.state };
            context.advance(instruction_extent<1, combat_status_state_change_data>);
        }
        else
        {
            input = get<0>(context.stack().top<set_combat_status_state_input>());
            context.stack().pop<set_combat_status_state_input>();
            context.enter_next();
        }
#ifndef NDEBUG
        debug_validate_entity(table, input.status, "set_combat_status_state", "status");
#endif
        const bool valid = static_cast<bool>(table[input.status]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        if constexpr(not IgnoreLimit)
        {
            const auto current = table[input.status].state();
            const auto limit = library[table[input.status].definition_id()].query(combat_status_state_limit{});
            input.state = {
                std::min(input.state.count, std::max(current.count, limit.count)),
                std::min(input.state.round_usages, std::max(current.round_usages, limit.round_usages))
            };
        }
        return change_combat_status_state(library, table, context, random, input);
    }

    inline void compile(program_writer& writer, const givm::set_combat_status_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(command.ignore_limit
                ? execute_fn{ execute_combat_status_state_change<true, true> }
                : execute_fn{ execute_combat_status_state_change<true, false> });
            writer.write(combat_status_state_change_data{ command.player, command.definition, command.state });
        }
        else
            writer.write(command.ignore_limit
                ? execute_fn{ execute_combat_status_state_change<false, true> }
                : execute_fn{ execute_combat_status_state_change<false, false> });
    }
}

namespace givm::detail
{
    inline std::vector<set_combat_status_state::error_type> check(const set_combat_status_state& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = set_combat_status_state::error_type::reason;
        std::vector<set_combat_status_state::error_type> errors;
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

#include <givm/macro_undef.hpp>

namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const set_combat_status_state& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<set_combat_status_state::input_type>() : std::size_t(-1);
    }
}

#endif
