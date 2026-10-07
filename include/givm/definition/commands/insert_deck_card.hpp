#ifndef GIVM_DEFINITION_COMMANDS_INSERT_DECK_CARD_HPP
#define GIVM_DEFINITION_COMMANDS_INSERT_DECK_CARD_HPP

#include <cstddef>
#include <cstdint>
#include <string>

#include "../../table.hpp"

namespace givm
{
    struct insert_deck_card_error
    {
        enum class reason
        {
            invalid_player,
            invalid_definition
        };

        reason cause;
        std::uint64_t value{};
        std::size_t limit{};
    };

    inline std::string error_string(const insert_deck_card_error& error)
    {
        using reason = insert_deck_card_error::reason;
        switch(error.cause)
        {
        case reason::invalid_player:
            return "insert_deck_card: player must be player 0 or 1; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "insert_deck_card: definition ID " + std::to_string(error.value) + " is outside card_definition definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct insert_deck_card
    {
        using error_type = insert_deck_card_error;

        player_id player;
        optional_definition_id<definition_category::card> definition{};
        std::int32_t position = -1;
    };
}

#endif
