#ifndef GIVM_EXECUTOR_COMMANDS_USE_SKILL_HPP
#define GIVM_EXECUTOR_COMMANDS_USE_SKILL_HPP

#include <algorithm>
#include <utility>

#include "../broadcast.hpp"
#include "../../definition/commands.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state finish_skill_effect(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        const auto event = get<0>(context.stack().top<skill_used, response_return>());
        context.stack().pop<skill_used, response_return>();
        prepare_broadcast(library, event, table, context.stack(), context.position() + sizeof(execute_fn));
        return context.enter_next();
    }

    template<bool ActionSelection>
    inline execution_state broadcast_skill_will_be_used(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<skill_will_be_used>(library, table, context, random))
            return continue_execution;
        const auto event = get<0>(context.stack().top<skill_will_be_used, response_return>());
        pop_broadcast<skill_will_be_used>(context);
        if constexpr(ActionSelection)
        {
            if(event.speed == action_speed::combat) table[event.skill.character_id.player_id].state().can_plunge = false;
        }
        context.enter_next();
        context.stack().push(skill_used{
            .skill = event.skill, .flags = event.flags, .targets = event.targets, .speed = event.speed,
            .effect_cancelled = event.effect_cancelled
        }, response_return{ table.state().self_player, context.position() });
        if(not event.effect_cancelled)
        {
            skill_effect effect{ .skill = event.skill, .flags = event.flags, .targets = event.targets };
            const auto skill = std::as_const(table)[event.skill];
            auto response = context.make_handle_context(table, random);
            const auto entry = library[skill.definition_id()].handle<skill_effect>(skill, effect, response);
            if(entry)
            {
                table.state().self_player = skill.player().id();
                return context.enter(entry);
            }
        }
        return finish_skill_effect(library, table, context, random);
    }

    inline execution_state finish_skill_use(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<skill_used>(library, table, context, random))
            return continue_execution;
        pop_broadcast<skill_used>(context);
        return context.enter_next();
    }

    template<bool Fixed>
    inline execution_state prepare_skill_command(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, use_skill>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            context.advance(instruction_extent<1, use_skill>);
            const auto character = table[player].state().active_character;
            if(not character) return context.advance(3 * sizeof(execute_fn));
            auto skills = table[*character].skills();
            const auto found = std::ranges::find_if(skills,
                [&](const auto skill) { return skill.definition_id() == command.definition; });
            if(found == skills.end()) return context.advance(3 * sizeof(execute_fn));
            prepare_broadcast(library, skill_will_be_used{
                .skill = (*found).id(), .flags = library.skill_flags(command.definition),
                .targets = {}, .speed = action_speed::fast
            }, table, context.stack(), context.position());
        }
        else
        {
            const auto effect = get<0>(context.stack().top<skill_effect>());
            context.stack().pop<skill_effect>();
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
        writer.write(execute_fn{ broadcast_skill_will_be_used<false> });
        writer.write(execute_fn{ finish_skill_effect });
        writer.write(execute_fn{ finish_skill_use });
    }
}

#include "../../macro_undef.hpp"
#endif
