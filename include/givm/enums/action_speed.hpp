#ifndef GIVM_ENUMS_ACTION_SPEED_HPP
#define GIVM_ENUMS_ACTION_SPEED_HPP

#include <cstdint>

namespace givm
{
    enum class action_speed : std::uint8_t
    {
        fast,
        combat
    };
}

#endif
