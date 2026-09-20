#ifndef GIVM_ENUMS_EQUIPMENT_TYPE_HPP
#define GIVM_ENUMS_EQUIPMENT_TYPE_HPP

#include <cstdint>

namespace givm
{
    enum class equipment_type : std::uint8_t
    {
        weapon,
        artifact,
        talent,
        technique,
        none
    };
}

#endif
