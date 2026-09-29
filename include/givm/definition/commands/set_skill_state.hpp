#ifndef GIVM_DEFINITION_COMMANDS_SET_SKILL_STATE_HPP
#define GIVM_DEFINITION_COMMANDS_SET_SKILL_STATE_HPP

#include <cstddef>
#include <string>

#include "../../table.hpp"
#include "../events.hpp"

namespace givm
{
    struct set_skill_state_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_character_player,
            invalid_character_selection,
            invalid_definition
        };

        reason cause;
        std::size_t value{};
        std::size_t limit{};
    };

    inline std::string error_string(const set_skill_state_error& error)
    {
        using reason = set_skill_state_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "set_skill_state: cannot consume dynamic input in a root program";
        case reason::invalid_character_player:
            return "set_skill_state: character_player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_character_selection:
            return "set_skill_state: character.selection must be character; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "set_skill_state: definition ID " + std::to_string(error.value) + " is outside skill_view definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct set_skill_state_input
    {
        skill_id skill;
        skill_state state;
    };

    struct set_skill_state
    {
        using error_type = set_skill_state_error;

        using input_type = set_skill_state_input;

        relative_character_target character{};
        definition_id<skill_view> definition{};
        skill_state state{};
    };
}

#endif
