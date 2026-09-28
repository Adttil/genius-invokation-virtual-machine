#ifndef GIVM_DEFINITION_COMMANDS_REPLACE_CARDS_BOTH_HPP
#define GIVM_DEFINITION_COMMANDS_REPLACE_CARDS_BOTH_HPP

#include <string>

namespace givm
{
    enum class replace_cards_both_error {};

    inline std::string error_string(replace_cards_both_error)
    {
        return {};
    }

    struct replace_cards_both
    {
        using error_type = replace_cards_both_error;
    };
}

#endif
