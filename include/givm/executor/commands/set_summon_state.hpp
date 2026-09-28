#ifndef GIVM_EXECUTOR_COMMANDS_SET_SUMMON_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_SUMMON_STATE_HPP

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include "add_summon.hpp"
#include "remove_summon.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<bool Fixed>
    inline execution_state execute_summon_state_change(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, set_summon_state>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
#ifndef NDEBUG
            debug_validate_required_entity(table, player, command.definition, "set_summon_state", "summon");
#endif
            const auto summon = require_summon(table, player, command.definition);
            table[summon].state() = clamp_summon_state(command.state,
                library[command.definition].query(summon_state_limit{}));
            return context.advance(instruction_extent<1, set_summon_state>);
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
            {
                auto summon = table[change.summon];
                const bool valid = static_cast<bool>(summon);
                GIVM_ASSERT(valid);
                [[assume(valid)]];
                summon.state() = clamp_summon_state(change.state,
                    library[summon.definition_id()].query(summon_state_limit{}));
            }
            context.stack().pop<set_summon_state_input::change[]>();
            return context.enter_next();
        }
    }

    inline void compile(program_writer& writer, const givm::set_summon_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_summon_state_change<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_summon_state_change<false> });
    }
}

namespace givm
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
        if(command.definition.value() >= context.definition_count<summon_view>())
            errors.push_back({ .cause = reason::invalid_definition, .value = command.definition.value(), .limit = context.definition_count<summon_view>() });
        return errors;
    }
}

#include "../../macro_undef.hpp"
#endif
