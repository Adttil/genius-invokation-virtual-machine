#ifndef GIVM_DEFINITION_COMMANDS_SELECT_ACTIVE_CHARACTER_BOTH_HPP
#define GIVM_DEFINITION_COMMANDS_SELECT_ACTIVE_CHARACTER_BOTH_HPP

#include <string>

namespace givm
{
    enum class select_active_character_both_error {};

    inline std::string error_string(select_active_character_both_error)
    {
        return {};
    }

    struct select_active_character_both
    {
        using error_type = select_active_character_both_error;
    };
}

#endif
