#ifndef GIVM_DEFINITION_COMMANDS_ATTACH_HPP
#define GIVM_DEFINITION_COMMANDS_ATTACH_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"

namespace givm
{
    struct attach_error
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

    inline std::string error_string(const attach_error& error)
    {
        using reason = attach_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "attach: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "attach: player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "attach: definition ID " + std::to_string(error.value) + " is outside attachment_view definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct attach_input
    {
        character_id target;
        definition_id<definition_category::attachment> definition;
        attachment_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct attach
    {
        using error_type = attach_error;

        using input_type = attach_input;

        relative_player player = relative_player::self;
        optional_definition_id<definition_category::attachment> definition{};
        attachment_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };
}

#endif
