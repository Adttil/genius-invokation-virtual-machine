#ifndef GIVM_DEFINITION_COMMANDS_START_BATTLE_HPP
#define GIVM_DEFINITION_COMMANDS_START_BATTLE_HPP

#include <string>

namespace givm
{
    enum class start_battle_error {};

    inline std::string error_string(start_battle_error)
    {
        return {};
    }

    struct start_battle
    {
        using error_type = start_battle_error;
    };
}

#endif
