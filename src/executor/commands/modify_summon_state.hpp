#ifndef GIVM_EXECUTOR_COMMANDS_MODIFY_SUMMON_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_MODIFY_SUMMON_STATE_HPP

#include "../program_writer.hpp"

#include <algorithm>
#include <limits>
#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <cstdint>

#include "set_summon_state.hpp"
#include <givm/macro_define.hpp>

namespace givm::detail
{
    struct summon_state_modification_data
    {
        relative_player player;
        definition_id<definition_category::summon> definition;
        std::int64_t value;
        std::int64_t usages;
    };

    template<bool Fixed, bool IgnoreLimit>
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
            summon_state limit{
                std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max()
            };
            if constexpr(not IgnoreLimit)
            {
                limit = library[summon.definition_id()].query(summon_state_limit{});
                limit.value = std::max(current.value, limit.value);
                limit.usages = std::max(current.usages, limit.usages);
            }
            summon.state() = {
                add_saturated(current.value, value, limit.value),
                add_saturated(current.usages, usages, limit.usages)
            };
            return summon;
        };

        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, summon_state_modification_data>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player.get() : other_player(table.state().self_player.get());
#ifndef NDEBUG
            debug_validate_required_entity(table, player, command.definition, "modify_summon_state", "summon");
#endif
            const auto summon = require_summon(table, player, command.definition);
            const auto changed = modify(summon, command.value, command.usages);
            context.advance(instruction_extent<1, summon_state_modification_data>);
            if(changed.state().usages == 0 && library.remove_at_zero_usages(command.definition))
                remove_summon_and_record(table, context, summon);
            return continue_execution;
        }
        else
        {
            const auto [summons, value, usages] = context.stack().top<summon_id[], std::int64_t, std::int64_t>();
#ifndef NDEBUG
            debug_validate_unique(std::span<const summon_id>{ summons }, "modify_summon_state", "summons");
            for(std::size_t index = 0; index != summons.size(); ++index)
                debug_validate_entity(table, summons[index], "modify_summon_state", "summons[" + std::to_string(index) + "]");
#endif
            for(const auto id : summons)
            {
                const auto changed = modify(id, value, usages);
                if(changed.state().usages == 0 && library.remove_at_zero_usages(changed.definition_id()))
                    remove_summon_and_record(table, context, id);
            }
            context.stack().pop<summon_id[], std::int64_t, std::int64_t>();
            return context.enter_next();
        }
    }

    inline void compile(program_writer& writer, const givm::modify_summon_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(command.ignore_limit
                ? execute_fn{ execute_summon_state_modification<true, true> }
                : execute_fn{ execute_summon_state_modification<true, false> });
            writer.write(summon_state_modification_data{ command.player, command.definition.get<definition_category::summon>(), command.value, command.usages });
        }
        else
        {
            writer.write(command.ignore_limit
                ? execute_fn{ execute_summon_state_modification<false, true> }
                : execute_fn{ execute_summon_state_modification<false, false> });
        }
    }
}

namespace givm::detail
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
        if(command.definition.get<definition_category::summon>().value() >= context.definition_count<definition_category::summon>())
            errors.push_back({ .cause = reason::invalid_definition, .value = command.definition.get<definition_category::summon>().value(), .limit = context.definition_count<definition_category::summon>() });
        return errors;
    }
}

#include <givm/macro_undef.hpp>

namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const modify_summon_state& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<modify_summon_state::input_type>() : std::size_t(-1);
    }
}

#endif
