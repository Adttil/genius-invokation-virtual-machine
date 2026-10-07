#ifndef GIVM_DEFINITION_COMMANDS_ADD_ATTACHMENT_HPP
#define GIVM_DEFINITION_COMMANDS_ADD_ATTACHMENT_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"

namespace givm
{
    struct add_attachment_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_player,
            invalid_definition
        };

        reason cause;
        std::uint64_t value{};
        std::size_t limit{};
    };

    inline std::string error_string(const add_attachment_error& error)
    {
        using reason = add_attachment_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "add_attachment: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "add_attachment: player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "add_attachment: definition ID " + std::to_string(error.value) + " is outside attachment_view definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct add_attachment_input
    {
        character_id target;
        definition_id<definition_category::attachment> definition;
        attachment_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct add_attachment
    {
        using error_type = add_attachment_error;

        using input_type = add_attachment_input;

        relative_player player = relative_player::self;
        optional_definition_id<definition_category::attachment> definition{};
        attachment_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };
}

#endif
