#ifndef GIVM_DEFINITION_COMMANDS_SET_ACTIVE_CHARACTER_HPP
#define GIVM_DEFINITION_COMMANDS_SET_ACTIVE_CHARACTER_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include "../events.hpp"

namespace givm
{
    struct set_active_character_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_target_player,
            invalid_target_selection
        };

        reason cause;
        std::size_t value{};
    };

    inline std::string error_string(const set_active_character_error& error)
    {
        using reason = set_active_character_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "set_active_character: cannot consume dynamic input in a root program";
        case reason::invalid_target_player:
            return "set_active_character: target.player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_target_selection:
            return "set_active_character: target.selection must be character; got " + std::to_string(error.value);
        }
        return {};
    }

    using set_active_character_input = active_character_changed;

    struct set_active_character
    {
        using error_type = set_active_character_error;

        using input_type = set_active_character_input;

        relative_character_target target{ {}, std::numeric_limits<std::int32_t>::max() };
    };
}

#endif
