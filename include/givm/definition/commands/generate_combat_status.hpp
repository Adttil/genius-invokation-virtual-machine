#ifndef GIVM_DEFINITION_COMMANDS_GENERATE_COMBAT_STATUS_HPP
#define GIVM_DEFINITION_COMMANDS_GENERATE_COMBAT_STATUS_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"

namespace givm
{
    struct generate_combat_status_error
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

    inline std::string error_string(const generate_combat_status_error& error)
    {
        using reason = generate_combat_status_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "generate_combat_status: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "generate_combat_status: player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "generate_combat_status: definition ID " + std::to_string(error.value) + " is outside combat_status_view definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct generate_combat_status_input
    {
        player_id player;
        definition_id<definition_category::combat_status> definition;
        combat_status_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct generate_combat_status
    {
        using error_type = generate_combat_status_error;

        using input_type = generate_combat_status_input;

        relative_player player = relative_player::self;
        optional_definition_id<definition_category::combat_status> definition{};
        combat_status_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };
}

#endif
