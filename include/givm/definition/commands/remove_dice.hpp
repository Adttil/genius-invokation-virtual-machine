#ifndef GIVM_DEFINITION_COMMANDS_REMOVE_DICE_HPP
#define GIVM_DEFINITION_COMMANDS_REMOVE_DICE_HPP

#include <cstddef>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"
#include "../events.hpp"

namespace givm
{
    struct remove_dice_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_player
        };

        reason cause;
        std::size_t value{};
    };

    inline std::string error_string(const remove_dice_error& error)
    {
        using reason = remove_dice_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "remove_dice: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "remove_dice: player must be self or opponent; got " + std::to_string(error.value);
        }
        return {};
    }

    using remove_dice_input = dice_removed;

    struct remove_dice
    {
        using error_type = remove_dice_error;

        using input_type = remove_dice_input;

        relative_player player = static_cast<relative_player>(-1);
        dice_counts dice{};
    };
}

#endif
