#ifndef GIVM_DEFINITION_COMMANDS_MODIFY_SUPPORT_STATE_HPP
#define GIVM_DEFINITION_COMMANDS_MODIFY_SUPPORT_STATE_HPP

#include <cstddef>
#include <cstdint>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"

namespace givm
{
    struct modify_support_state_error
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

    inline std::string error_string(const modify_support_state_error& error)
    {
        using reason = modify_support_state_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "modify_support_state: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "modify_support_state: player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "modify_support_state: definition ID " + std::to_string(error.value) + " is outside support_view definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct modify_support_state_input
    {
        support_id support;
        std::int64_t count{};
        std::int64_t round_usages{};
    };

    struct modify_support_state
    {
        using error_type = modify_support_state_error;

        using input_type = modify_support_state_input;

        relative_player player = relative_player::self;
        definition_id<support_view> definition{};
        std::int64_t count{};
        std::int64_t round_usages{};
    };
}

#endif
