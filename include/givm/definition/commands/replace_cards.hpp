#ifndef GIVM_DEFINITION_COMMANDS_REPLACE_CARDS_HPP
#define GIVM_DEFINITION_COMMANDS_REPLACE_CARDS_HPP

#include <cstddef>
#include <string>

#include "../../table.hpp"

namespace givm
{
    struct replace_cards_error
    {
        enum class reason
        {
            invalid_player
        };

        reason cause;
        std::size_t value{};
    };

    inline std::string error_string(const replace_cards_error& error)
    {
        using reason = replace_cards_error::reason;
        switch(error.cause)
        {
        case reason::invalid_player:
            return "replace_cards: player must be player 0 or 1; got " + std::to_string(error.value);
        }
        return {};
    }

    struct replace_cards
    {
        using error_type = replace_cards_error;

        player_id player;
    };
}

#endif
