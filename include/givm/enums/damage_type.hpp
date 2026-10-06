#ifndef GIVM_ENUMS_DAMAGE_TYPE_HPP
#define GIVM_ENUMS_DAMAGE_TYPE_HPP

#include <bit>
#include <cstddef>
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

    class damage_type_mask
    {
    public:
        constexpr damage_type_mask() noexcept = default;
        constexpr damage_type_mask(damage_type type) noexcept { set(type); }

        constexpr bool operator[](damage_type type) const noexcept
        {
            return (bits_ & (std::uint16_t{ 1 } << static_cast<std::size_t>(type))) != 0;
        }

        constexpr bool all() const noexcept { return bits_ == all_bits; }
        constexpr bool any() const noexcept { return bits_ != 0; }
        constexpr bool none() const noexcept { return bits_ == 0; }
        constexpr std::size_t count() const noexcept { return std::popcount(bits_); }
        constexpr std::size_t size() const noexcept { return 10; }

        constexpr damage_type_mask& set() noexcept { bits_ = all_bits; return *this; }
        constexpr damage_type_mask& set(damage_type type, bool value = true) noexcept
        {
            const auto bit = std::uint16_t{ 1 } << static_cast<std::size_t>(type);
            if(value) bits_ |= bit;
            else bits_ &= ~bit;
            return *this;
        }
        constexpr damage_type_mask& reset() noexcept { bits_ = 0; return *this; }
        constexpr damage_type_mask& reset(damage_type type) noexcept { return set(type, false); }
        constexpr damage_type_mask& flip() noexcept { bits_ ^= all_bits; return *this; }
        constexpr damage_type_mask& flip(damage_type type) noexcept
        {
            bits_ ^= std::uint16_t{ 1 } << static_cast<std::size_t>(type);
            return *this;
        }
        constexpr damage_type_mask& operator|=(const damage_type_mask& other) noexcept { bits_ |= other.bits_; return *this; }
        constexpr damage_type_mask& operator&=(const damage_type_mask& other) noexcept { bits_ &= other.bits_; return *this; }
        constexpr damage_type_mask& operator^=(const damage_type_mask& other) noexcept { bits_ ^= other.bits_; return *this; }
        constexpr bool operator==(const damage_type_mask&) const noexcept = default;

    private:
        static constexpr std::uint16_t all_bits = (std::uint16_t{ 1 } << 10) - 1;
        std::uint16_t bits_ = 0;
    };

    constexpr damage_type_mask operator|(damage_type_mask lhs, const damage_type_mask& rhs) noexcept { return lhs |= rhs; }
    constexpr damage_type_mask operator&(damage_type_mask lhs, const damage_type_mask& rhs) noexcept { return lhs &= rhs; }
    constexpr damage_type_mask operator^(damage_type_mask lhs, const damage_type_mask& rhs) noexcept { return lhs ^= rhs; }
    constexpr damage_type_mask operator~(damage_type_mask value) noexcept { return value.flip(); }
    constexpr damage_type_mask operator|(damage_type lhs, damage_type rhs) noexcept { return damage_type_mask(lhs) | rhs; }

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
