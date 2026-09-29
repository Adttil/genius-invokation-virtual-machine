#ifndef GIVM_DEFINITION_COMMANDS_ADD_COMBAT_STATUS_HPP
#define GIVM_DEFINITION_COMMANDS_ADD_COMBAT_STATUS_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"

namespace givm
{
    struct add_combat_status_error
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

    inline std::string error_string(const add_combat_status_error& error)
    {
        using reason = add_combat_status_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "add_combat_status: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "add_combat_status: player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "add_combat_status: definition ID " + std::to_string(error.value) + " is outside combat_status_view definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct add_combat_status_input
    {
        player_id player;
        definition_id<combat_status_view> definition;
        combat_status_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct add_combat_status
    {
        using error_type = add_combat_status_error;

        using input_type = add_combat_status_input;

        relative_player player = relative_player::self;
        definition_id<combat_status_view> definition{};
        combat_status_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };
}

#endif
