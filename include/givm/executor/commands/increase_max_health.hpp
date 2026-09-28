#ifndef GIVM_EXECUTOR_COMMANDS_INCREASE_MAX_HEALTH_HPP
#define GIVM_EXECUTOR_COMMANDS_INCREASE_MAX_HEALTH_HPP

#include <vector>

#include "heal.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<bool Fixed>
    inline execution_state execute_max_health_increase(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto input = [&]() -> std::optional<increase_max_health_input>
        {
            if constexpr(Fixed)
            {
                const auto command = context.instruction_data<1, increase_max_health>(library);
                context.advance(instruction_extent<1, increase_max_health>);
                const auto source = resolve_character_target<false>(table, command.source);
                const auto target = resolve_character_target<false>(table, command.target);
                if(not source || not target) return std::nullopt;
                return increase_max_health_input{ *source, *target, command.value };
            }
            else
            {
                const auto event = get<0>(context.stack().top<increase_max_health_input>());
                context.stack().pop<increase_max_health_input>();
                context.enter_next();
                return event;
            }
        }();
        if(not input) return context.enter_next();
        const bool valid = static_cast<bool>(table[input->target]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        auto& state = table[input->target].state();
        GIVM_ASSERT(state.health <= state.max_health);
        [[assume(state.health <= state.max_health)]];
        const auto value = std::min(input->value, std::numeric_limits<std::uint32_t>::max() - state.max_health);
        state.max_health += value;
        state.health += value;
        prepare_broadcast(library, healed{ input->source, input->target, value }, table, context.stack(), context.position());
        return broadcast_healing_completed(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::increase_max_health& command, compile_mode)
    {
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
            writer.write(execute_fn{ execute_max_health_increase<false> });
        else
        {
            writer.write(execute_fn{ execute_max_health_increase<true> });
            writer.write(command);
        }
        writer.write(execute_fn{ broadcast_healing_completed });
    }
}

namespace givm
{
    inline std::vector<increase_max_health::error_type> check(const increase_max_health& command,
        const definition_compile_context&, program_kind kind)
    {
        using reason = increase_max_health::error_type::reason;
        std::vector<increase_max_health::error_type> errors;
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.source.player != relative_player::self && command.source.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_source_player, .value = static_cast<std::size_t>(command.source.player) });
        if(command.source.selection != character_selection::character)
            errors.push_back({ .cause = reason::invalid_source_selection, .value = static_cast<std::size_t>(command.source.selection) });
        if(command.target.player != relative_player::self && command.target.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_target_player, .value = static_cast<std::size_t>(command.target.player) });
        if(command.target.selection != character_selection::character)
            errors.push_back({ .cause = reason::invalid_target_selection, .value = static_cast<std::size_t>(command.target.selection) });
        return errors;
    }
}

#include "../../macro_undef.hpp"
#endif
