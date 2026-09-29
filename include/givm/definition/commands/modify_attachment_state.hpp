#ifndef GIVM_DEFINITION_COMMANDS_MODIFY_ATTACHMENT_STATE_HPP
#define GIVM_DEFINITION_COMMANDS_MODIFY_ATTACHMENT_STATE_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>

#include "../../table.hpp"
#include "attachment_target.hpp"

namespace givm
{
    struct modify_attachment_state_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_definition,
            invalid_equipment_type,
            invalid_target_character_player,
            invalid_target_character_selection
        };

        reason cause;
        std::size_t value{};
        std::size_t limit{};
    };

    inline std::string error_string(const modify_attachment_state_error& error)
    {
        using reason = modify_attachment_state_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "modify_attachment_state: cannot consume dynamic input in a root program";
        case reason::invalid_definition:
            return "modify_attachment_state: target.selector definition ID " + std::to_string(error.value) + " is outside attachment_view definitions [0, " + std::to_string(error.limit) + ")";
        case reason::invalid_equipment_type:
            return "modify_attachment_state: target.selector must be an equipment type before none; got " + std::to_string(error.value);
        case reason::invalid_target_character_player:
            return "modify_attachment_state: target.character.player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_target_character_selection:
            return "modify_attachment_state: target.character.selection must be character; got " + std::to_string(error.value);
        }
        return {};
    }

    struct modify_attachment_state_input
    {
        attachment_target attachment;
        std::int64_t count{};
        std::int64_t round_usages{};
    };

    struct modify_attachment_state
    {
        using error_type = modify_attachment_state_error;

        using input_type = modify_attachment_state_input;

        relative_attachment_target target{};
        std::int64_t count{};
        std::int64_t round_usages{};
        bool ignore_limit = false;
    };
}

#endif
