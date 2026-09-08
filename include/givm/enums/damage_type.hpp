#ifndef GIVM_ENUMS_DAMAGE_TYPE_HPP
#define GIVM_ENUMS_DAMAGE_TYPE_HPP

#include <cstdint>

#include "element.hpp"

namespace givm
{
    enum class damage_type : std::uint8_t
    {
        cryo,
        hydro,
        pyro,
        electro,
        anemo,
        geo,
        dendro,
        physical,
        piercing,
        true_damage
    };

    constexpr element element_from_damage_type(damage_type type) noexcept
    {
        switch(type)
        {
        case damage_type::cryo: return element::cryo;
        case damage_type::hydro: return element::hydro;
        case damage_type::pyro: return element::pyro;
        case damage_type::electro: return element::electro;
        case damage_type::anemo: return element::anemo;
        case damage_type::geo: return element::geo;
        case damage_type::dendro: return element::dendro;
        case damage_type::physical:
        case damage_type::piercing:
        case damage_type::true_damage: return element::none;
        }
        return element::none;
    }
}

#endif
