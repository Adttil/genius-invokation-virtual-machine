#ifndef GIVM_DEFINITION_COMMANDS_SET_ENERGY_HPP
#define GIVM_DEFINITION_COMMANDS_SET_ENERGY_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include "../../table.hpp"
#include "../events.hpp"

namespace givm
{
    struct set_energy_error
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

    inline std::string error_string(const set_energy_error& error)
    {
        using reason = set_energy_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "set_energy: cannot consume dynamic input in a root program";
        case reason::invalid_target_player:
            return "set_energy: target.player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_target_selection:
            return "set_energy: target.selection must be character; got " + std::to_string(error.value);
        }
        return {};
    }

    struct set_energy_input
    {
        character_id target;
        std::uint32_t value;
    };

    struct set_energy
    {
        using error_type = set_energy_error;

        using input_type = set_energy_input;

        relative_character_target target{ {}, std::numeric_limits<std::int32_t>::max() };
        std::uint32_t value{};
    };
}

#endif
