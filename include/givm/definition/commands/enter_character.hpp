#ifndef GIVM_DEFINITION_COMMANDS_ENTER_CHARACTER_HPP
#define GIVM_DEFINITION_COMMANDS_ENTER_CHARACTER_HPP

#include <cstddef>
#include <string>

#include "../../table.hpp"

namespace givm
{
    struct enter_character_error
    {
        enum class reason
        {
            invalid_player,
            invalid_definition
        };

        reason cause;
        std::size_t value{};
        std::size_t limit{};
    };

    inline std::string error_string(const enter_character_error& error)
    {
        using reason = enter_character_error::reason;
        switch(error.cause)
        {
        case reason::invalid_player:
            return "enter_character: player must be player 0 or 1; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "enter_character: definition ID " + std::to_string(error.value) + " is outside character_view definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct enter_character
    {
        using error_type = enter_character_error;

        player_id player;
        definition_id<character_view> definition;
    };
}

#endif
