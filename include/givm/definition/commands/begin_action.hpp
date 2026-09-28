#ifndef GIVM_DEFINITION_COMMANDS_BEGIN_ACTION_HPP
#define GIVM_DEFINITION_COMMANDS_BEGIN_ACTION_HPP

#include <string>

namespace givm
{
    enum class begin_action_error {};

    inline std::string error_string(begin_action_error)
    {
        return {};
    }

    struct begin_action
    {
        using error_type = begin_action_error;
    };
}

#endif
