#ifndef GIVM_DEFINITION_COMMANDS_CREATE_HAND_CARD_HPP
#define GIVM_DEFINITION_COMMANDS_CREATE_HAND_CARD_HPP

#include <cstddef>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"

namespace givm
{
    struct create_hand_card_error
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

    inline std::string error_string(const create_hand_card_error& error)
    {
        using reason = create_hand_card_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "create_hand_card: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "create_hand_card: player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "create_hand_card: definition ID " + std::to_string(error.value) + " is outside card_definition definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct create_hand_card_input
    {
        player_id player;
        definition_id<card_definition> definition;
    };

    struct create_hand_card
    {
        using error_type = create_hand_card_error;

        using input_type = create_hand_card_input;

        relative_player player = relative_player::self;
        definition_id<card_definition> definition{};
    };
}

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const create_hand_card& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<create_hand_card::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
