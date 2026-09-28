#ifndef GIVM_EXECUTOR_COMMANDS_MODIFY_COMBAT_STATUS_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_MODIFY_COMBAT_STATUS_STATE_HPP

#include <algorithm>
#include <limits>
#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <cstdint>

#include "set_combat_status_state.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    struct combat_status_state_modification_data
    {
        relative_player player;
        definition_id<combat_status_view> definition;
        std::int64_t count;
        std::int64_t round_usages;
    };

    template<bool Fixed, bool IgnoreLimit>
    inline execution_state execute_combat_status_state_modification(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        modify_combat_status_state_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, combat_status_state_modification_data>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
#ifndef NDEBUG
            debug_validate_required_entity(table, player, command.definition, "modify_combat_status_state", "status");
#endif
            input = {
                require_combat_status(table, player, command.definition), command.count, command.round_usages
            };
            context.advance(instruction_extent<1, combat_status_state_modification_data>);
        }
        else
        {
            input = get<0>(context.stack().top<modify_combat_status_state_input>());
            context.stack().pop<modify_combat_status_state_input>();
            context.enter_next();
        }

#ifndef NDEBUG
        debug_validate_entity(table, input.status, "modify_combat_status_state", "status");
#endif
        const bool valid = static_cast<bool>(table[input.status]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        const auto current = table[input.status].state();
        combat_status_state limit{
            std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max()
        };
        if constexpr(not IgnoreLimit)
        {
            limit = library[table[input.status].definition_id()].query(combat_status_state_limit{});
            limit.count = std::max(current.count, limit.count);
            limit.round_usages = std::max(current.round_usages, limit.round_usages);
        }
        const auto add_saturated = [](std::uint32_t value, std::int64_t delta, std::uint32_t maximum)
        {
            const auto previous = static_cast<std::int64_t>(value);
            if(delta <= -previous)
                return std::uint32_t{};
            if(delta >= static_cast<std::int64_t>(maximum) - previous)
                return maximum;
            return static_cast<std::uint32_t>(previous + delta);
        };
        const combat_status_state state{
            add_saturated(current.count, input.count, limit.count),
            add_saturated(current.round_usages, input.round_usages, limit.round_usages)
        };
        return change_combat_status_state(library, table, context, random,
            set_combat_status_state_input{ input.status, state });
    }

    inline void compile(program_writer& writer, const givm::modify_combat_status_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(command.ignore_limit
                ? execute_fn{ execute_combat_status_state_modification<true, true> }
                : execute_fn{ execute_combat_status_state_modification<true, false> });
            writer.write(combat_status_state_modification_data{ command.player, command.definition, command.count, command.round_usages });
        }
        else
            writer.write(command.ignore_limit
                ? execute_fn{ execute_combat_status_state_modification<false, true> }
                : execute_fn{ execute_combat_status_state_modification<false, false> });
        writer.write(execute_fn{ finish_combat_status_state_change });
    }
}

namespace givm
{
    inline std::vector<modify_combat_status_state::error_type> check(const modify_combat_status_state& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = modify_combat_status_state::error_type::reason;
        std::vector<modify_combat_status_state::error_type> errors;
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

#include "../../macro_undef.hpp"
#endif
