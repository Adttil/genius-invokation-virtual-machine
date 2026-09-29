#ifndef GIVM_DEFINITION_COMMANDS_INCREASE_MAX_HEALTH_HPP
#define GIVM_DEFINITION_COMMANDS_INCREASE_MAX_HEALTH_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include "../events.hpp"

namespace givm
{
    struct increase_max_health_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_source_player,
            invalid_source_selection,
            invalid_target_player,
            invalid_target_selection
        };

        reason cause;
        std::size_t value{};
    };

    inline std::string error_string(const increase_max_health_error& error)
    {
        using reason = increase_max_health_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "increase_max_health: cannot consume dynamic input in a root program";
        case reason::invalid_source_player:
            return "increase_max_health: source.player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_source_selection:
            return "increase_max_health: source.selection must be character; got " + std::to_string(error.value);
        case reason::invalid_target_player:
            return "increase_max_health: target.player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_target_selection:
            return "increase_max_health: target.selection must be character; got " + std::to_string(error.value);
        }
        return {};
    }

    using increase_max_health_input = healing;

    struct increase_max_health
    {
        using error_type = increase_max_health_error;

        using input_type = increase_max_health_input;

        relative_character_target source{};
        relative_character_target target{ {}, std::numeric_limits<std::int32_t>::max() };
        std::uint32_t value{};
    };
}

#endif
