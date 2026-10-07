#ifndef GIVM_EXECUTOR_COMMANDS_SET_SKILL_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_SKILL_STATE_HPP

#include "../program_writer.hpp"

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <algorithm>

#include <givm/executor/executor.hpp>
#include "../character_target.hpp"
#include <givm/definition.hpp>
#include <givm/macro_define.hpp>

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
#ifndef NDEBUG
            if(not has_character)
                throw command_input_error{ "set_skill_state", missing_entity_argument{ "character" } };
#endif
            GIVM_ASSERT(has_character);
            [[assume(has_character)]];
            auto skills = table[*character].skills();
            const auto found = std::ranges::find_if(skills,
                [&](const auto skill) { return skill.definition_id() == command.definition.get<definition_category::skill>(); });
            const bool has_skill = found != skills.end();
#ifndef NDEBUG
            if(not has_skill)
                throw command_input_error{ "set_skill_state", missing_entity_argument{ "skill", command_entity_id{ *character }, command.definition.get<definition_category::skill>().value() } };
#endif
            GIVM_ASSERT(has_skill);
            [[assume(has_skill)]];
            (*found).state() = command.state;
            return context.advance(instruction_extent<1, set_skill_state>);
        }
        else
        {
            const auto input = get<0>(context.stack().top<set_skill_state_input>());
            context.stack().pop<set_skill_state_input>();
#ifndef NDEBUG
            debug_validate_entity(table, input.skill, "set_skill_state", "skill");
#endif
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

namespace givm::detail
{
    inline std::vector<set_skill_state::error_type> check(const set_skill_state& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = set_skill_state::error_type::reason;
        std::vector<set_skill_state::error_type> errors;
        if(not command.definition)
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.character.player != relative_player::self && command.character.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_character_player, .value = static_cast<std::size_t>(command.character.player) });
        if(command.character.selection != character_selection::character)
            errors.push_back({ .cause = reason::invalid_character_selection, .value = static_cast<std::size_t>(command.character.selection) });
        if(command.definition.get<definition_category::skill>().value() >= context.definition_count<definition_category::skill>())
            errors.push_back({ .cause = reason::invalid_definition, .value = command.definition.get<definition_category::skill>().value(), .limit = context.definition_count<definition_category::skill>() });
        return errors;
    }
}

#include <givm/macro_undef.hpp>

namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const set_skill_state& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<set_skill_state::input_type>() : std::size_t(-1);
    }
}

#endif
