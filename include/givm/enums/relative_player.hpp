#ifndef GIVM_ENUMS_RELATIVE_PLAYER_HPP
#define GIVM_ENUMS_RELATIVE_PLAYER_HPP

#include <cstdint>

namespace givm
{
    enum class relative_player : std::uint8_t
    {
        self,
        opponent
    };
}

#endif
