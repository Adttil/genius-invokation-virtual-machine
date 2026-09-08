#ifndef GIVM_ENUMS_ELEMENTAL_REACTION_HPP
#define GIVM_ENUMS_ELEMENTAL_REACTION_HPP

#include <cstdint>

#include "element_aura.hpp"

namespace givm
{
    enum class elemental_reaction : std::uint8_t
    {
        none,
        melt,
        vaporize,
        overloaded,
        superconduct,
        electro_charged,
        lunar_charged,
        frozen,
        swirl,
        crystallize,
        burning,
        bloom,
        quicken
    };

    namespace detail
    {
        constexpr bool is_element_pair(element lhs, element rhs, element first, element second) noexcept
        {
            return (lhs == first && rhs == second) || (lhs == second && rhs == first);
        }

        constexpr bool is_swirl_or_crystallize_target(element value) noexcept
        {
            return value == element::cryo || value == element::hydro || value == element::pyro ||
                   value == element::electro;
        }
    }

    constexpr elemental_reaction reaction_between(element aura, element incoming) noexcept
    {
        if(aura == element::none || incoming == element::none || aura == incoming)
        {
            return elemental_reaction::none;
        }
        if(detail::is_element_pair(aura, incoming, element::cryo, element::pyro))
            return elemental_reaction::melt;
        if(detail::is_element_pair(aura, incoming, element::hydro, element::pyro))
            return elemental_reaction::vaporize;
        if(detail::is_element_pair(aura, incoming, element::pyro, element::electro))
            return elemental_reaction::overloaded;
        if(detail::is_element_pair(aura, incoming, element::cryo, element::electro))
            return elemental_reaction::superconduct;
        if(detail::is_element_pair(aura, incoming, element::hydro, element::electro))
        {
            // TODO: distinguish Electro-Charged and Lunar-Charged when table stores the required lunar state.
            return elemental_reaction::electro_charged;
        }
        if(detail::is_element_pair(aura, incoming, element::cryo, element::hydro))
            return elemental_reaction::frozen;
        if(detail::is_element_pair(aura, incoming, element::pyro, element::dendro))
            return elemental_reaction::burning;
        if(detail::is_element_pair(aura, incoming, element::hydro, element::dendro))
            return elemental_reaction::bloom;
        if(detail::is_element_pair(aura, incoming, element::electro, element::dendro))
            return elemental_reaction::quicken;
        if((aura == element::anemo && detail::is_swirl_or_crystallize_target(incoming)) ||
           (incoming == element::anemo && detail::is_swirl_or_crystallize_target(aura)))
        {
            return elemental_reaction::swirl;
        }
        if((aura == element::geo && detail::is_swirl_or_crystallize_target(incoming)) ||
           (incoming == element::geo && detail::is_swirl_or_crystallize_target(aura)))
        {
            return elemental_reaction::crystallize;
        }
        return elemental_reaction::none;
    }

    constexpr elemental_reaction reaction_from_aura(element_aura aura, element incoming) noexcept
    {
        return reaction_between(primary_element_from_aura(aura), incoming);
    }

    constexpr element_aura aura_after_reaction(element_aura, element, elemental_reaction) noexcept
    {
        // TODO: complete official aura consumption rules.
        return element_aura::none;
    }
}

#endif
