#ifndef GIVM_EXECUTOR_COMMANDS_SET_SKILL_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_SKILL_STATE_HPP

#include <algorithm>

#include "../executor.hpp"
#include "../character_target.hpp"
#include "../../definition/commands.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<bool Fixed>
    inline execution_state execute_skill_state_change(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn&)
    {
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, set_skill_state>(library);
            const auto character = resolve_character_target<false>(table, command.character);
            const bool has_character = character.has_value();
            GIVM_ASSERT(has_character);
            [[assume(has_character)]];
            auto skills = table[*character].skills();
            const auto found = std::ranges::find_if(skills,
                [&](const auto skill) { return skill.definition_id() == command.definition; });
            const bool has_skill = found != skills.end();
            GIVM_ASSERT(has_skill);
            [[assume(has_skill)]];
            (*found).state() = command.state;
            return context.advance(instruction_extent<1, set_skill_state>);
        }
        else
        {
            const auto input = get<0>(context.stack().top<set_skill_state_input>());
            context.stack().pop<set_skill_state_input>();
            GIVM_ASSERT(table[input.skill].is_valid());
            table[input.skill].state() = input.state;
            return context.enter_next();
        }
    }

    inline void compile(program_writer& writer, const givm::set_skill_state& command, compile_mode)
    {
        if(command.definition)
        {
            GIVM_ASSERT(command.character.selection == character_selection::character);
            [[assume(command.character.selection == character_selection::character)]];
            writer.write(execute_fn{ execute_skill_state_change<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_skill_state_change<false> });
    }
}

#include "../../macro_undef.hpp"
#endif
