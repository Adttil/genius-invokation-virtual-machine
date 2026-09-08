#ifndef GIVM_ENUMS_ATTACHMENT_SLOT_HPP
#define GIVM_ENUMS_ATTACHMENT_SLOT_HPP

#include <cstdint>

namespace givm
{
    enum class attachment_slot : std::uint8_t
    {
        none,
        weapon,
        artifact,
        talent,
        technique
    };
}

#endif
