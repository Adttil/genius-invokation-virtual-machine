#ifndef GIVM_ENUMS_ELEMENTAL_REACTION_HPP
#define GIVM_ENUMS_ELEMENTAL_REACTION_HPP

#include <bit>
#include <cstddef>
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
        frozen,
        burning,
        bloom,
        quicken,
        swirl_cryo,
        swirl_hydro,
        swirl_pyro,
        swirl_electro,
        crystallize_cryo,
        crystallize_hydro,
        crystallize_pyro,
        crystallize_electro
    };

    inline constexpr std::size_t elemental_reaction_count = 17;

    class elemental_reaction_mask
    {
    public:
        constexpr elemental_reaction_mask() noexcept = default;
        constexpr elemental_reaction_mask(elemental_reaction type) noexcept { set(type); }

        constexpr bool operator[](elemental_reaction type) const noexcept
        {
            if(type == elemental_reaction::none) return false;
            return (bits_ & (std::uint32_t{ 1 } << (static_cast<std::size_t>(type) - 1))) != 0;
        }

        constexpr bool all() const noexcept { return bits_ == all_bits; }
        constexpr bool any() const noexcept { return bits_ != 0; }
        constexpr bool none() const noexcept { return bits_ == 0; }
        constexpr std::size_t count() const noexcept { return std::popcount(bits_); }
        constexpr std::size_t size() const noexcept { return elemental_reaction_count; }

        constexpr elemental_reaction_mask& set() noexcept { bits_ = all_bits; return *this; }
        constexpr elemental_reaction_mask& set(elemental_reaction type, bool value = true) noexcept
        {
            if(type == elemental_reaction::none) return *this;
            const auto bit = std::uint32_t{ 1 } << (static_cast<std::size_t>(type) - 1);
            if(value) bits_ |= bit;
            else bits_ &= ~bit;
            return *this;
        }
        constexpr elemental_reaction_mask& reset() noexcept { bits_ = 0; return *this; }
        constexpr elemental_reaction_mask& reset(elemental_reaction type) noexcept { return set(type, false); }
        constexpr elemental_reaction_mask& flip() noexcept { bits_ ^= all_bits; return *this; }
        constexpr elemental_reaction_mask& flip(elemental_reaction type) noexcept
        {
            if(type == elemental_reaction::none) return *this;
            bits_ ^= std::uint32_t{ 1 } << (static_cast<std::size_t>(type) - 1);
            return *this;
        }
        constexpr elemental_reaction_mask& operator|=(const elemental_reaction_mask& other) noexcept { bits_ |= other.bits_; return *this; }
        constexpr elemental_reaction_mask& operator&=(const elemental_reaction_mask& other) noexcept { bits_ &= other.bits_; return *this; }
        constexpr elemental_reaction_mask& operator^=(const elemental_reaction_mask& other) noexcept { bits_ ^= other.bits_; return *this; }
        constexpr bool operator==(const elemental_reaction_mask&) const noexcept = default;

    private:
        static constexpr std::uint32_t all_bits = (std::uint32_t{ 1 } << elemental_reaction_count) - 1;
        std::uint32_t bits_ = 0;
    };

    constexpr elemental_reaction_mask operator|(elemental_reaction_mask lhs, const elemental_reaction_mask& rhs) noexcept { return lhs |= rhs; }
    constexpr elemental_reaction_mask operator&(elemental_reaction_mask lhs, const elemental_reaction_mask& rhs) noexcept { return lhs &= rhs; }
    constexpr elemental_reaction_mask operator^(elemental_reaction_mask lhs, const elemental_reaction_mask& rhs) noexcept { return lhs ^= rhs; }
    constexpr elemental_reaction_mask operator~(elemental_reaction_mask value) noexcept { return value.flip(); }
    constexpr elemental_reaction_mask operator|(elemental_reaction lhs, elemental_reaction rhs) noexcept { return elemental_reaction_mask(lhs) | rhs; }

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
            // Definitions select replacement effects after this base reaction is determined.
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
        if(incoming == element::anemo && detail::is_swirl_or_crystallize_target(aura))
        {
            return static_cast<elemental_reaction>(static_cast<unsigned>(elemental_reaction::swirl_cryo)
                + static_cast<unsigned>(aura));
        }
        if(incoming == element::geo && detail::is_swirl_or_crystallize_target(aura))
        {
            return static_cast<elemental_reaction>(static_cast<unsigned>(elemental_reaction::crystallize_cryo)
                + static_cast<unsigned>(aura));
        }
        return elemental_reaction::none;
    }

    constexpr elemental_reaction reaction_from_aura(element_aura aura, element incoming) noexcept
    {
        return reaction_between(primary_element_from_aura(aura), incoming);
    }

    constexpr element_aura aura_after_reaction(element_aura aura, element, elemental_reaction) noexcept
    {
        return aura == element_aura::cryo_dendro ? element_aura::dendro : element_aura::none;
    }
}

#endif
