#ifndef GIVM_DEFINITION_COMMANDS_END_ROUND_HPP
#define GIVM_DEFINITION_COMMANDS_END_ROUND_HPP

#include <string>

namespace givm
{
    enum class end_round_error {};

    inline std::string error_string(end_round_error)
    {
        return {};
    }

    struct end_round
    {
        using error_type = end_round_error;
    };
}

#endif
