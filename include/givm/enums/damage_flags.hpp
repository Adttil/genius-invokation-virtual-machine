#ifndef GIVM_ENUMS_DAMAGE_FLAGS_HPP
#define GIVM_ENUMS_DAMAGE_FLAGS_HPP

#include <cstdint>

namespace givm
{
    enum class damage_flag_bits : std::uint8_t
    {
        combat_damage = 1u << 0,
        skill_damage = 1u << 1,
        reaction_damage = 1u << 2,
        ignore_shield = 1u << 3
    };

    class damage_flags
    {
    public:
        constexpr damage_flags() noexcept = default;

        constexpr damage_flags(damage_flag_bits bit) noexcept
        : bits_{ value_of(bit) }
        {}

        static constexpr damage_flags from_bits(std::uint8_t bits) noexcept
        {
            damage_flags flags;
            flags.bits_ = bits;
            return flags;
        }

        [[nodiscard]] constexpr std::uint8_t value() const noexcept
        {
            return bits_;
        }

        [[nodiscard]] constexpr bool contains(damage_flag_bits bit) const noexcept
        {
            return (bits_ & value_of(bit)) != 0;
        }

        constexpr void set(damage_flag_bits bit) noexcept
        {
            bits_ |= value_of(bit);
        }

        constexpr void reset(damage_flag_bits bit) noexcept
        {
            bits_ &= static_cast<std::uint8_t>(~value_of(bit));
        }

        friend constexpr damage_flags operator|(damage_flags lhs, damage_flag_bits rhs) noexcept
        {
            lhs.set(rhs);
            return lhs;
        }

        friend constexpr damage_flags operator|(damage_flag_bits lhs, damage_flag_bits rhs) noexcept
        {
            return damage_flags{ lhs } | rhs;
        }

    private:
        static constexpr std::uint8_t value_of(damage_flag_bits bit) noexcept
        {
            return static_cast<std::uint8_t>(bit);
        }

        std::uint8_t bits_ = 0;
    };
}

#endif
