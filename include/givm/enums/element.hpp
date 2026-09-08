#ifndef GIVM_ENUMS_ELEMENT_HPP
#define GIVM_ENUMS_ELEMENT_HPP

#include <cstdint>

namespace givm
{
    enum class element : std::uint8_t
    {
        none,
        cryo,
        hydro,
        pyro,
        electro,
        anemo,
        geo,
        dendro
    };
}

#endif
