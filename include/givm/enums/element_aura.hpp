#ifndef GIVM_ENUMS_ELEMENT_AURA_HPP
#define GIVM_ENUMS_ELEMENT_AURA_HPP

#include "element.hpp"

namespace givm
{
    enum class element_aura : unsigned char
    {
        none,
        cryo,
        hydro,
        pyro,
        electro,
        anemo,
        geo,
        dendro,
        cryo_dendro,
        dendro_cryo
    };

    constexpr element_aura aura_from_element(element value) noexcept
    {
        switch(value)
        {
        case element::cryo: return element_aura::cryo;
        case element::hydro: return element_aura::hydro;
        case element::pyro: return element_aura::pyro;
        case element::electro: return element_aura::electro;
        case element::anemo: return element_aura::anemo;
        case element::geo: return element_aura::geo;
        case element::dendro: return element_aura::dendro;
        case element::none: return element_aura::none;
        }
        return element_aura::none;
    }

    constexpr element primary_element_from_aura(element_aura aura) noexcept
    {
        switch(aura)
        {
        case element_aura::cryo: return element::cryo;
        case element_aura::hydro: return element::hydro;
        case element_aura::pyro: return element::pyro;
        case element_aura::electro: return element::electro;
        case element_aura::anemo: return element::anemo;
        case element_aura::geo: return element::geo;
        case element_aura::dendro: return element::dendro;
        case element_aura::cryo_dendro: return element::cryo;
        case element_aura::dendro_cryo: return element::dendro;
        case element_aura::none: return element::none;
        }
        return element::none;
    }

    constexpr element_aura aura_without_reaction(element_aura current, element incoming) noexcept
    {
        const auto incoming_aura = aura_from_element(incoming);
        if(current == element_aura::none || current == incoming_aura)
        {
            return incoming_aura;
        }
        if(current == element_aura::cryo && incoming == element::dendro) return element_aura::cryo_dendro;
        if(current == element_aura::dendro && incoming == element::cryo) return element_aura::dendro_cryo;

        // TODO: complete non-reactive aura preservation/replacement rules, especially for
        // Anemo/Geo and ordered Cryo/Dendro coexistence.
        return incoming_aura;
    }
}

#endif
