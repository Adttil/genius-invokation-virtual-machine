#ifndef GIVM_ENUMS_ELEMENT_HPP
#define GIVM_ENUMS_ELEMENT_HPP

#include <cstdint>

namespace givm
{
    enum class element : std::uint8_t
    {
        cryo,
        hydro,
        pyro,
        electro,
        geo,
        dendro,
        anemo,
        none
    };
}

#endif
