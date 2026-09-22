#ifndef GIVM_ENUMS_SKILL_FLAGS_HPP
#define GIVM_ENUMS_SKILL_FLAGS_HPP

#include <cstdint>
#include "damage_flags.hpp"

namespace givm
{
    enum class skill_flag_bits : std::uint8_t
    {
        normal_attack = 1u << 0,
        elemental_skill = 1u << 1,
        elemental_burst = 1u << 2,
        charged_attack = 1u << 3,
        plunging_attack = 1u << 4
    };

    class skill_flags
    {
    public:
        constexpr skill_flags() noexcept = default;
        constexpr skill_flags(skill_flag_bits bit) noexcept : bits_{ static_cast<std::uint8_t>(bit) } {}

        [[nodiscard]] constexpr std::uint8_t value() const noexcept { return bits_; }
        [[nodiscard]] constexpr bool contains(skill_flag_bits bit) const noexcept
        {
            return (bits_ & static_cast<std::uint8_t>(bit)) != 0;
        }
        constexpr void set(skill_flag_bits bit) noexcept { bits_ |= static_cast<std::uint8_t>(bit); }
        constexpr void reset(skill_flag_bits bit) noexcept { bits_ &= static_cast<std::uint8_t>(~static_cast<std::uint8_t>(bit)); }

        [[nodiscard]] constexpr damage_flags to_damage_flags() const noexcept
        {
            damage_flags result{ damage_flag_bits::skill_damage };
            if(contains(skill_flag_bits::normal_attack)) result.set(damage_flag_bits::normal_attack);
            if(contains(skill_flag_bits::elemental_skill)) result.set(damage_flag_bits::elemental_skill);
            if(contains(skill_flag_bits::elemental_burst)) result.set(damage_flag_bits::elemental_burst);
            if(contains(skill_flag_bits::charged_attack)) result.set(damage_flag_bits::charged_attack);
            if(contains(skill_flag_bits::plunging_attack)) result.set(damage_flag_bits::plunging_attack);
            return result;
        }

        friend constexpr skill_flags operator|(skill_flags lhs, skill_flag_bits rhs) noexcept
        {
            lhs.set(rhs);
            return lhs;
        }

    private:
        std::uint8_t bits_ = 0;
    };

    constexpr skill_flags operator|(skill_flag_bits lhs, skill_flag_bits rhs) noexcept
    {
        return skill_flags{ lhs } | rhs;
    }
}

#endif
