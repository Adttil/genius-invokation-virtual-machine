#ifndef GIVM_DEFINITION_COMMANDS_RETURN_RESPONSE_HPP
#define GIVM_DEFINITION_COMMANDS_RETURN_RESPONSE_HPP

#include <cstdint>
#include <limits>
#include <string>

namespace givm
{
    struct return_response_error
    {
        enum class reason { return_in_root };
        reason cause;
    };

    inline std::string error_string(const return_response_error&)
    {
        return "return_response: cannot return from a root program";
    }

    struct return_response_input
    {
        std::uint32_t index = std::numeric_limits<std::uint32_t>::max();
    };

    struct return_response
    {
        using error_type = return_response_error;
        using input_type = return_response_input;

        static constexpr std::uint32_t null = std::numeric_limits<std::uint32_t>::max();
        static constexpr std::uint32_t dynamic = null - 1;

        std::uint32_t index = dynamic;
    };
}

#endif
