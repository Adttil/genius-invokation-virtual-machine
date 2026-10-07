#ifndef GIVM_DEFINITION_COMMANDS_TRANSFER_ATTACHMENT_HPP
#define GIVM_DEFINITION_COMMANDS_TRANSFER_ATTACHMENT_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>

#include "../../table.hpp"
#include "../events.hpp"
#include "attachment_target.hpp"

namespace givm
{
    struct transfer_attachment_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_definition,
            invalid_equipment_type,
            invalid_source_character_player,
            invalid_source_character_selection,
            invalid_target_player,
            invalid_target_selection
        };

        reason cause;
        std::uint64_t value{};
        std::size_t limit{};
    };

    inline std::string error_string(const transfer_attachment_error& error)
    {
        using reason = transfer_attachment_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "transfer_attachment: cannot consume dynamic input in a root program";
        case reason::invalid_definition:
            return "transfer_attachment: source.selector definition ID " + std::to_string(error.value) + " is outside attachment_view definitions [0, " + std::to_string(error.limit) + ")";
        case reason::invalid_equipment_type:
            return "transfer_attachment: source.selector must be an equipment type before none; got " + std::to_string(error.value);
        case reason::invalid_source_character_player:
            return "transfer_attachment: source.character.player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_source_character_selection:
            return "transfer_attachment: source.character.selection must be character; got " + std::to_string(error.value);
        case reason::invalid_target_player:
            return "transfer_attachment: target.player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_target_selection:
            return "transfer_attachment: target.selection must be character; got " + std::to_string(error.value);
        }
        return {};
    }

    struct transfer_attachment_input
    {
        attachment_target attachment;
        character_id target;
        bool reset_round_usages = false;
    };

    struct transfer_attachment
    {
        using error_type = transfer_attachment_error;

        using input_type = transfer_attachment_input;

        relative_attachment_target source{};
        relative_character_target target{};
        bool reset_round_usages = false;
    };
}

#endif
