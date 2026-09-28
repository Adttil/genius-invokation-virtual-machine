#ifndef GIVM_DEFINITION_COMMANDS_SHUFFLE_DECK_HPP
#define GIVM_DEFINITION_COMMANDS_SHUFFLE_DECK_HPP

#include <cstddef>
#include <string>

#include "../../table.hpp"

namespace givm
{
    struct shuffle_deck_error
    {
        enum class reason
        {
            invalid_player
        };

        reason cause;
        std::size_t value{};
    };

    inline std::string error_string(const shuffle_deck_error& error)
    {
        using reason = shuffle_deck_error::reason;
        switch(error.cause)
        {
        case reason::invalid_player:
            return "shuffle_deck: player must be player 0 or 1; got " + std::to_string(error.value);
        }
        return {};
    }

    struct shuffle_deck
    {
        using error_type = shuffle_deck_error;

        player_id player;
    };
}

#endif
