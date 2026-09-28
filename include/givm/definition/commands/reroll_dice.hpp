#ifndef GIVM_DEFINITION_COMMANDS_REROLL_DICE_HPP
#define GIVM_DEFINITION_COMMANDS_REROLL_DICE_HPP

#include <cstddef>
#include <cstdint>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"

namespace givm
{
    struct reroll_dice_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_player
        };

        reason cause;
        std::size_t value{};
    };

    inline std::string error_string(const reroll_dice_error& error)
    {
        using reason = reroll_dice_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "reroll_dice: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "reroll_dice: player must be self or opponent; got " + std::to_string(error.value);
        }
        return {};
    }

    struct reroll_dice_input
    {
        player_id player;
        std::uint32_t reroll_count = 1;
    };

    struct reroll_dice
    {
        using error_type = reroll_dice_error;

        using input_type = reroll_dice_input;

        relative_player player = static_cast<relative_player>(-1);
        std::uint32_t reroll_count = 1;
    };
}

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const reroll_dice& command) noexcept
    {
        return command.player == static_cast<relative_player>(-1) ? TInputTypes::template index_of<reroll_dice::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
