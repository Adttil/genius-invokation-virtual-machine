#ifndef GIVM_DEFINITION_COMMANDS_ADD_SUPPORT_HPP
#define GIVM_DEFINITION_COMMANDS_ADD_SUPPORT_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"

namespace givm
{
    struct add_support_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_player,
            invalid_definition
        };

        reason cause;
        std::size_t value{};
        std::size_t limit{};
    };

    inline std::string error_string(const add_support_error& error)
    {
        using reason = add_support_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "add_support: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "add_support: player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "add_support: definition ID " + std::to_string(error.value) + " is outside support_view definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct add_support_input
    {
        player_id player;
        definition_id<support_view> definition;
        support_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct add_support
    {
        using error_type = add_support_error;

        using input_type = add_support_input;

        relative_player player = relative_player::self;
        definition_id<support_view> definition{};
        support_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };
}

#endif
