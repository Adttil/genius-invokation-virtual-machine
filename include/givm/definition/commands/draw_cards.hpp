#ifndef GIVM_DEFINITION_COMMANDS_DRAW_CARDS_HPP
#define GIVM_DEFINITION_COMMANDS_DRAW_CARDS_HPP

#include <cstddef>
#include <limits>
#include <span>
#include <string>
#include <tuple>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"
#include "../../utils/stack.hpp"

namespace givm
{
    struct draw_cards_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_player
        };

        reason cause;
        std::size_t value{};
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
        std::size_t position = std::numeric_limits<std::size_t>::max();
        std::size_t count = 1;
    };
}

namespace givm::detail
{
    inline auto command_input_members(const draw_cards_input& input) noexcept
    {
        return std::tuple{ dynamic_array<deck_card_id>(input.cards) };
    }
}

#endif
