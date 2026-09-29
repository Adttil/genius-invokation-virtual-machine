#ifndef GIVM_DEFINITION_COMMANDS_REMOVE_ATTACHMENT_HPP
#define GIVM_DEFINITION_COMMANDS_REMOVE_ATTACHMENT_HPP

#include <cstddef>
#include <string>
#include <variant>

#include "../../table.hpp"
#include "attachment_target.hpp"

namespace givm
{
    struct remove_attachment_error
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

    inline std::string error_string(const remove_attachment_error& error)
    {
        using reason = remove_attachment_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "remove_attachment: cannot consume dynamic input in a root program";
        case reason::invalid_definition:
            return "remove_attachment: target.selector definition ID " + std::to_string(error.value) + " is outside attachment_view definitions [0, " + std::to_string(error.limit) + ")";
        case reason::invalid_equipment_type:
            return "remove_attachment: target.selector must be an equipment type before none; got " + std::to_string(error.value);
        case reason::invalid_target_character_player:
            return "remove_attachment: target.character.player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_target_character_selection:
            return "remove_attachment: target.character.selection must be character; got " + std::to_string(error.value);
        }
        return {};
    }

    struct remove_attachment_input
    {
        attachment_target attachment;
    };

    struct remove_attachment
    {
        using error_type = remove_attachment_error;

        using input_type = remove_attachment_input;

        relative_attachment_target target{};
    };
}

#endif
