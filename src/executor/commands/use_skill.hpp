#ifndef GIVM_EXECUTOR_COMMANDS_USE_SKILL_HPP
#define GIVM_EXECUTOR_COMMANDS_USE_SKILL_HPP

#include "../program_writer.hpp"

#include <vector>

#include <algorithm>
#include <utility>

#include "../broadcast.hpp"
#include <givm/definition.hpp>
#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <givm/macro_define.hpp>

namespace givm::detail
{
    inline execution_state finish_skill_effect(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_single_response<skill_effect, skill_id>(library, table, context, random)) return continue_execution;
        pop_single_response<skill_effect, skill_id>(context);
        const auto event = get<0>(context.stack().top<skill_used>());
        context.stack().pop<skill_used>();
        prepare_broadcast(library, event, table, context.stack(), context.position() + response_extent<skill_effect>);
        return context.advance(response_extent<skill_effect>);
    }

    template<bool ActionSelection>
    inline execution_state broadcast_skill_will_be_used(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<skill_will_be_used>(library, table, context, random))
            return continue_execution;
        const auto event = get<0>(context.stack().top<skill_will_be_used, response_return>());
#ifndef NDEBUG
        if(event.speed != action_speed::fast && event.speed != action_speed::combat)
            throw command_input_error{ "use_skill", invalid_enum_argument{ "speed", static_cast<std::size_t>(event.speed) } };
#endif
        pop_broadcast<skill_will_be_used>(context);
        if constexpr(ActionSelection)
        {
            if(event.speed == action_speed::combat) table[event.skill.character_id.player_id].state().can_plunge = false;
        }
        context.advance(response_extent<skill_will_be_used>);
        context.stack().push(skill_used{
            .skill = event.skill, .flags = event.flags, .targets = event.targets, .speed = event.speed,
            .effect_cancelled = event.effect_cancelled
        });
        prepare_single_response(skill_effect{ .skill = event.skill, .flags = event.flags, .targets = event.targets },
            event.skill, table, context, context.position(), false, not event.effect_cancelled);
        return finish_skill_effect(library, table, context, random);
    }

    inline execution_state finish_skill_use(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<skill_used>(library, table, context, random))
            return continue_execution;
        pop_broadcast<skill_used>(context);
        return context.advance(response_extent<skill_used>);
    }

    template<bool Fixed>
    inline execution_state prepare_skill_command(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, use_skill>(library);
#ifndef NDEBUG
            debug_validate_entity(table, table.state().self_player, "use_skill", "self_player");
#endif
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            context.advance(instruction_extent<1, use_skill>);
            const auto character = table[player].state().active_character;
            if(not character) return context.advance(response_extent<skill_will_be_used>
                + response_extent<skill_effect> + response_extent<skill_used>);
#ifndef NDEBUG
            debug_validate_entity(table, *character, "use_skill", "active_character");
#endif
            auto skills = table[*character].skills();
            const auto found = std::ranges::find_if(skills,
                [&](const auto skill) { return skill.definition_id() == command.definition; });
            if(found == skills.end()) return context.advance(response_extent<skill_will_be_used>
                + response_extent<skill_effect> + response_extent<skill_used>);
            prepare_broadcast(library, skill_will_be_used{
                .skill = (*found).id(), .flags = library.skill_flags(command.definition),
                .targets = {}, .speed = action_speed::fast
            }, table, context.stack(), context.position());
        }
        else
        {
            const auto effect = get<0>(context.stack().top<use_skill_input>());
#ifndef NDEBUG
            debug_validate_entity(table, effect.skill, "use_skill", "skill");
            if(table[effect.skill.character_id.player_id].state().active_character != effect.skill.character_id)
                throw command_input_error{ "use_skill", invalid_entity_relation{ "skill", invalid_entity_relation::reason::inactive_character } };
#endif
            context.stack().pop<use_skill_input>();
            context.enter_next();
            GIVM_ASSERT(table[effect.skill.character_id.player_id].state().active_character == effect.skill.character_id);
            prepare_broadcast(library, skill_will_be_used{
                .skill = effect.skill, .flags = effect.flags, .targets = effect.targets, .speed = action_speed::fast
            }, table, context.stack(), context.position());
        }
        return broadcast_skill_will_be_used<false>(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::use_skill& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ prepare_skill_command<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ prepare_skill_command<false> });
        compile_broadcast<skill_will_be_used>(writer, broadcast_skill_will_be_used<false>);
        compile_single_response<skill_effect, skill_id>(writer, finish_skill_effect);
        compile_broadcast<skill_used>(writer, finish_skill_use);
    }
}

namespace givm::detail
{
    inline std::vector<use_skill::error_type> check(const use_skill& command, const definition_compile_context& context, program_kind kind)
    {
        using reason = use_skill::error_type::reason;
        std::vector<use_skill::error_type> errors;
        if(not command.definition)
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.player != relative_player::self && command.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_player, .value = static_cast<std::size_t>(command.player) });
        if(command.definition.value() >= context.definition_count<skill_view>())
            errors.push_back({ .cause = reason::invalid_definition, .value = command.definition.value(), .limit = context.definition_count<skill_view>() });
        return errors;
    }
}

#include <givm/macro_undef.hpp>

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const use_skill& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<use_skill::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
