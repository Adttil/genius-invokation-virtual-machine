#ifndef GIVM_DEFINITION_COMMANDS_DRAW_CARDS_HPP
#define GIVM_DEFINITION_COMMANDS_DRAW_CARDS_HPP

#include <cstddef>
#include <span>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"

namespace givm
{
    struct draw_cards_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_player,
            duplicate_position
        };

        reason cause;
        std::size_t value{};
        std::size_t index{};
        std::size_t first_index{};
    };

    inline std::string error_string(const draw_cards_error& error)
    {
        using reason = draw_cards_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "draw_cards: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "draw_cards: player must be self or opponent; got " + std::to_string(error.value);
        case reason::duplicate_position:
            return "draw_cards: positions[" + std::to_string(error.index) + "] = " + std::to_string(error.value) + " duplicates positions[" + std::to_string(error.first_index) + "]";
        }
        return {};
    }

    struct draw_cards_input
    {
        std::span<const deck_card_id> cards;
    };

    struct draw_cards
    {
        using error_type = draw_cards_error;

        using input_type = draw_cards_input;

        relative_player player = relative_player::self;
        std::span<const std::size_t> positions{};
    };
}

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const draw_cards& command) noexcept
    {
        return command.positions.empty() ? TInputTypes::template index_of<draw_cards::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
