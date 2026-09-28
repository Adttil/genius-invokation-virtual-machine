#ifndef GIVM_DEFINITION_COMMANDS_DISCARD_DECK_CARDS_HPP
#define GIVM_DEFINITION_COMMANDS_DISCARD_DECK_CARDS_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"

namespace givm
{
    struct discard_deck_cards_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_player
        };

        reason cause;
        std::size_t value{};
    };

    inline std::string error_string(const discard_deck_cards_error& error)
    {
        using reason = discard_deck_cards_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "discard_deck_cards: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "discard_deck_cards: player must be self or opponent; got " + std::to_string(error.value);
        }
        return {};
    }

    struct discard_deck_cards_input
    {
        player_id player;
        std::uint32_t count;
    };

    struct discard_deck_cards
    {
        using error_type = discard_deck_cards_error;

        using input_type = discard_deck_cards_input;

        std::uint32_t count = std::numeric_limits<std::uint32_t>::max();
        relative_player player = relative_player::self;
    };
}

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const discard_deck_cards& command) noexcept
    {
        return command.count == std::numeric_limits<std::uint32_t>::max() ? TInputTypes::template index_of<discard_deck_cards::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
