#ifndef GIVM_ENUMS_WEAPON_TYPE_HPP
#define GIVM_ENUMS_WEAPON_TYPE_HPP

#include <cstdint>

namespace givm
{
    enum class weapon_type : std::uint8_t
    {
        none,
        sword,
        claymore,
        polearm,
        bow,
        catalyst
    };
}

#endif
