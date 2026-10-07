#ifndef GIVM_EXECUTOR_COMMANDS_SET_SUMMON_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_SUMMON_STATE_HPP

#include "../program_writer.hpp"

#include <algorithm>
#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include "add_summon.hpp"
#include "remove_summon.hpp"
#include <givm/macro_define.hpp>

namespace givm::detail
{
    struct summon_state_change_data
    {
        relative_player player;
        definition_id<definition_category::summon> definition;
        summon_state state;
    };

    template<bool Fixed, bool IgnoreLimit>
    inline execution_state execute_summon_state_change(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        const auto set_state = [&](summon_id id, summon_state state)
        {
            auto summon = table[id];
            const bool valid = static_cast<bool>(summon);
            GIVM_ASSERT(valid);
            [[assume(valid)]];
            if constexpr(not IgnoreLimit)
            {
                const auto current = summon.state();
                const auto limit = library[summon.definition_id()].query(summon_state_limit{});
                state = {
                    std::min(state.value, std::max(current.value, limit.value)),
                    std::min(state.usages, std::max(current.usages, limit.usages))
                };
            }
            summon.state() = state;
        };
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, summon_state_change_data>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player.get() : other_player(table.state().self_player.get());
#ifndef NDEBUG
            debug_validate_required_entity(table, player, command.definition, "set_summon_state", "summon");
#endif
            const auto summon = require_summon(table, player, command.definition);
            set_state(summon, command.state);
            return context.advance(instruction_extent<1, summon_state_change_data>);
        }
        else
        {
            const auto changes = get<0>(context.stack().top<set_summon_state_input::change[]>());
#ifndef NDEBUG
            for(std::size_t index = 0; index != changes.size(); ++index)
            {
                debug_validate_entity(table, changes[index].summon, "set_summon_state", "changes[" + std::to_string(index) + "].summon");
                for(std::size_t first = 0; first != index; ++first)
                    if(changes[first].summon == changes[index].summon)
                        throw command_input_error{ "set_summon_state", duplicate_entity_argument{ "changes", first, index } };
            }
#endif
            for(const auto& change : changes)
                set_state(change.summon, change.state);
            context.stack().pop<set_summon_state_input::change[]>();
            return context.enter_next();
        }
    }

    inline void compile(program_writer& writer, const givm::set_summon_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(command.ignore_limit
                ? execute_fn{ execute_summon_state_change<true, true> }
                : execute_fn{ execute_summon_state_change<true, false> });
            writer.write(summon_state_change_data{ command.player, command.definition.get<definition_category::summon>(), command.state });
        }
        else
            writer.write(command.ignore_limit
                ? execute_fn{ execute_summon_state_change<false, true> }
                : execute_fn{ execute_summon_state_change<false, false> });
    }
}

namespace givm::detail
{
    inline std::vector<set_summon_state::error_type> check(const set_summon_state& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = set_summon_state::error_type::reason;
        std::vector<set_summon_state::error_type> errors;
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
    constexpr std::size_t input_marker(const set_summon_state& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<set_summon_state::input_type>() : std::size_t(-1);
    }
}

#endif
