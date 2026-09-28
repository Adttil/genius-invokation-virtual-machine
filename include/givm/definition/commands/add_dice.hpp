#ifndef GIVM_DEFINITION_COMMANDS_ADD_DICE_HPP
#define GIVM_DEFINITION_COMMANDS_ADD_DICE_HPP

#include <cstddef>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"
#include "../events.hpp"

namespace givm
{
    struct add_dice_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_player
        };

        reason cause;
        std::size_t value{};
    };

    inline std::string error_string(const add_dice_error& error)
    {
        using reason = add_dice_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "add_dice: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "add_dice: player must be self or opponent; got " + std::to_string(error.value);
        }
        return {};
    }

    using add_dice_input = dice_added;

    struct add_dice
    {
        using error_type = add_dice_error;

        using input_type = add_dice_input;

        relative_player player = static_cast<relative_player>(-1);
        dice_counts dice{};
    };
}

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const add_dice& command) noexcept
    {
        return command.player == static_cast<relative_player>(-1) ? TInputTypes::template index_of<add_dice::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
