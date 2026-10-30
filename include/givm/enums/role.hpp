#ifndef GIVM_ENUMS_ROLE_HPP
#define GIVM_ENUMS_ROLE_HPP

#include <cstdint>

namespace givm
{
    enum class role : std::uint8_t
    {
        full,
        player0,
        player1,
        spectator
    };
}

#endif
