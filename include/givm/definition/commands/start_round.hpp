#ifndef GIVM_DEFINITION_COMMANDS_START_ROUND_HPP
#define GIVM_DEFINITION_COMMANDS_START_ROUND_HPP

#include <string>

namespace givm
{
    enum class start_round_error {};

    inline std::string error_string(start_round_error)
    {
        return {};
    }

    struct start_round
    {
        using error_type = start_round_error;
    };
}

#endif
