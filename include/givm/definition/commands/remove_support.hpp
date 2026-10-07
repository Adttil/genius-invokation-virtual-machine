#ifndef GIVM_DEFINITION_COMMANDS_REMOVE_SUPPORT_HPP
#define GIVM_DEFINITION_COMMANDS_REMOVE_SUPPORT_HPP

#include <cstddef>
#include <cstdint>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"

namespace givm
{
    struct remove_support_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_player,
            invalid_definition
        };

        reason cause;
        std::uint64_t value{};
        std::size_t limit{};
    };

    inline std::string error_string(const remove_support_error& error)
    {
        using reason = remove_support_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "remove_support: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "remove_support: player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "remove_support: definition ID " + std::to_string(error.value) + " is outside support_view definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct remove_support_input
    {
        support_id support;
    };

    struct remove_support
    {
        using error_type = remove_support_error;

        using input_type = remove_support_input;

        relative_player player = relative_player::self;
        optional_definition_id<definition_category::support> definition{};
    };
}

#endif
