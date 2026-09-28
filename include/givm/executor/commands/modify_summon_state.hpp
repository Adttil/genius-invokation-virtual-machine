#ifndef GIVM_EXECUTOR_COMMANDS_MODIFY_SUMMON_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_MODIFY_SUMMON_STATE_HPP

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <cstdint>

#include "set_summon_state.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<bool Fixed>
    inline execution_state execute_summon_state_modification(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        const auto add_saturated = [](std::uint32_t value, std::int64_t delta, std::uint32_t maximum)
        {
            const auto previous = static_cast<std::int64_t>(value);
            if(delta <= -previous)
                return std::uint32_t{};
            if(delta >= static_cast<std::int64_t>(maximum) - previous)
                return maximum;
            return static_cast<std::uint32_t>(previous + delta);
        };
        const auto modify = [&](summon_id id, std::int64_t value, std::int64_t usages)
        {
            auto summon = table[id];
            const bool valid = static_cast<bool>(summon);
            GIVM_ASSERT(valid);
            [[assume(valid)]];
            const auto current = summon.state();
            const auto limit = library[summon.definition_id()].query(summon_state_limit{});
            summon.state() = {
                add_saturated(current.value, value, limit.value),
                add_saturated(current.usages, usages, limit.usages)
            };
            return summon;
        };

        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, modify_summon_state>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
#ifndef NDEBUG
            debug_validate_required_entity(table, player, command.definition, "modify_summon_state", "summon");
#endif
            const auto summon = require_summon(table, player, command.definition);
            const auto changed = modify(summon, command.value, command.usages);
            context.advance(instruction_extent<1, modify_summon_state>);
            if(changed.state().usages == 0 && library.remove_at_zero_usages(command.definition))
                return remove_summon_and_broadcast(library, table, context, random, summon);
            return context.enter_next();
        }
        else
        {
            const auto [summons, value, usages] = context.stack().top<summon_id[], std::int64_t, std::int64_t>();
#ifndef NDEBUG
            debug_validate_unique(std::span<const summon_id>{ summons }, "modify_summon_state", "summons");
            for(std::size_t index = 0; index != summons.size(); ++index)
                debug_validate_entity(table, summons[index], "modify_summon_state", "summons[" + std::to_string(index) + "]");
#endif
            auto first = summons.size();
            for(std::size_t index = 0; index != summons.size(); ++index)
            {
                const auto changed = modify(summons[index], value, usages);
                if(first == summons.size() && changed.state().usages == 0
                    && library.remove_at_zero_usages(changed.definition_id()))
                    first = index;
            }
            context.enter_next();
            if(first == summons.size())
            {
                context.stack().pop<summon_id[], std::int64_t, std::int64_t>();
                return context.enter_next();
            }
            const auto summon = summons[first];
            context.stack().push(stack_count_t{ first + 1 });
            table[summon].erase();
            prepare_broadcast(library, summon_removed{ summon }, table, context.stack(), context.position());
            return broadcast_summon_removals<true>(library, table, context, random);
        }
    }

    inline void compile(program_writer& writer, const givm::modify_summon_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_summon_state_modification<true> });
            writer.write(command);
            writer.write(execute_fn{ broadcast_summon_removal });
        }
        else
        {
            writer.write(execute_fn{ execute_summon_state_modification<false> });
            writer.write(execute_fn{ broadcast_summon_removals<true> });
        }
    }
}

namespace givm
{
    inline std::vector<modify_summon_state::error_type> check(const modify_summon_state& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = modify_summon_state::error_type::reason;
        std::vector<modify_summon_state::error_type> errors;
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
