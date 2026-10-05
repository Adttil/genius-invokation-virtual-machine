#ifndef GIVM_DEFINITION_COMMANDS_DEFER_PROGRAM_HPP
#define GIVM_DEFINITION_COMMANDS_DEFER_PROGRAM_HPP

#include <string>
#include "../program_inputs.hpp"

namespace givm
{
    struct defer_program_error
    {
        enum class reason { dynamic_input_in_root };
        reason cause;
    };

    inline std::string error_string(const defer_program_error&)
    {
        return "defer_program: cannot consume dynamic input in a root program";
    }

    struct defer_program_input
    {
        program_entry entry;
        program_inputs inputs;
    };

    struct defer_program
    {
        using error_type = defer_program_error;
        using input_type = defer_program_input;

        defer_program_input input{};
    };
}

#endif
