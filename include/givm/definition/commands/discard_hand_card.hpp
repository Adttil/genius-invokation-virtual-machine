#ifndef GIVM_DEFINITION_COMMANDS_DISCARD_HAND_CARD_HPP
#define GIVM_DEFINITION_COMMANDS_DISCARD_HAND_CARD_HPP

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"

namespace givm
{
    struct discard_hand_card_error
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

    inline std::string error_string(const discard_hand_card_error& error)
    {
        using reason = discard_hand_card_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "discard_hand_card: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "discard_hand_card: player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "discard_hand_card: definition ID " + std::to_string(error.value) + " is outside card_definition definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct discard_hand_card_input
    {
        std::span<const hand_card_id> cards;
    };

    struct discard_hand_card
    {
        using error_type = discard_hand_card_error;

        using input_type = discard_hand_card_input;

        relative_player player = relative_player::self;
        definition_id<card_definition> definition{};
        std::uint32_t count = 1;
    };
}

#endif
