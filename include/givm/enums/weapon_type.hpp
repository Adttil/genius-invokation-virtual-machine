#ifndef GIVM_ENUMS_WEAPON_TYPE_HPP
#define GIVM_ENUMS_WEAPON_TYPE_HPP

#include <bitset>
#include <cstddef>
#include <cstdint>

namespace givm
{
    enum class weapon_type : std::uint8_t
    {
        none,
        sword,
        claymore,
        polearm,
        bow,
        catalyst
    };

    class weapon_type_mask
    {
    public:
        constexpr weapon_type_mask() noexcept = default;

        constexpr weapon_type_mask(weapon_type type) noexcept
        {
            set(type);
        }

        constexpr bool operator[](weapon_type type) const noexcept
        {
            return bits_[static_cast<size_t>(type) - 1];
        }

        constexpr bool all() const noexcept
        {
            return bits_.all();
        }

        constexpr bool any() const noexcept
        {
            return bits_.any();
        }

        constexpr bool none() const noexcept
        {
            return bits_.none();
        }

        constexpr size_t count() const noexcept
        {
            return bits_.count();
        }

        constexpr size_t size() const noexcept
        {
            return bits_.size();
        }

        constexpr weapon_type_mask& set() noexcept
        {
            bits_.set();
            return *this;
        }

        constexpr weapon_type_mask& set(weapon_type type, bool value = true) noexcept
        {
            bits_[static_cast<size_t>(type) - 1] = value;
            return *this;
        }

        constexpr weapon_type_mask& reset() noexcept
        {
            bits_.reset();
            return *this;
        }

        constexpr weapon_type_mask& reset(weapon_type type) noexcept
        {
            bits_[static_cast<size_t>(type) - 1] = false;
            return *this;
        }

        constexpr weapon_type_mask& flip() noexcept
        {
            bits_.flip();
            return *this;
        }

        constexpr weapon_type_mask& flip(weapon_type type) noexcept
        {
            bits_[static_cast<size_t>(type) - 1].flip();
            return *this;
        }

        constexpr weapon_type_mask& operator|=(const weapon_type_mask& other) noexcept
        {
            bits_ |= other.bits_;
            return *this;
        }

        constexpr weapon_type_mask& operator&=(const weapon_type_mask& other) noexcept
        {
            bits_ &= other.bits_;
            return *this;
        }

        constexpr weapon_type_mask& operator^=(const weapon_type_mask& other) noexcept
        {
            bits_ ^= other.bits_;
            return *this;
        }

        constexpr bool operator==(const weapon_type_mask&) const noexcept = default;

    private:
        std::bitset<static_cast<size_t>(weapon_type::catalyst)> bits_{};
    };

    constexpr weapon_type_mask operator|(weapon_type_mask lhs, const weapon_type_mask& rhs) noexcept
    {
        return lhs |= rhs;
    }

    constexpr weapon_type_mask operator&(weapon_type_mask lhs, const weapon_type_mask& rhs) noexcept
    {
        return lhs &= rhs;
    }

    constexpr weapon_type_mask operator^(weapon_type_mask lhs, const weapon_type_mask& rhs) noexcept
    {
        return lhs ^= rhs;
    }

    constexpr weapon_type_mask operator~(weapon_type_mask value) noexcept
    {
        return value.flip();
    }

    constexpr weapon_type_mask operator|(weapon_type lhs, weapon_type rhs) noexcept
    {
        return weapon_type_mask(lhs) | rhs;
    }

    constexpr weapon_type_mask operator&(weapon_type lhs, weapon_type rhs) noexcept
    {
        return weapon_type_mask(lhs) & rhs;
    }

    constexpr weapon_type_mask operator^(weapon_type lhs, weapon_type rhs) noexcept
    {
        return weapon_type_mask(lhs) ^ rhs;
    }

    constexpr weapon_type_mask operator~(weapon_type type) noexcept
    {
        return ~weapon_type_mask(type);
    }
}

#endif
