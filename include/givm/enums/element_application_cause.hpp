#ifndef GIVM_ENUMS_ELEMENT_APPLICATION_CAUSE_HPP
#define GIVM_ENUMS_ELEMENT_APPLICATION_CAUSE_HPP

#include <cstdint>

namespace givm
{
    enum class element_application_cause : std::uint8_t
    {
        effect,
        damage
    };
}

#endif
