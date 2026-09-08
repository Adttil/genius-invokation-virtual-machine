#ifndef GIVM_ENUMS_ELEMENTAL_DICE_HPP
#define GIVM_ENUMS_ELEMENTAL_DICE_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace givm
{
    enum class elemental_dice : std::uint8_t
    {
        omni,
        cryo,
        hydro,
        pyro,
        electro,
        anemo,
        geo,
        dendro
    };

    constexpr elemental_dice elemental_dice_from_random(std::uint32_t random) noexcept
    {
        return static_cast<elemental_dice>(random & 7u);
    }

    class dice_counts
    {
    public:
        template<class Self> constexpr auto& operator[](this Self& self, elemental_dice dice) noexcept
        {
            return self.values_[std::to_underlying(dice)];
        }

        friend constexpr bool operator==(const dice_counts&, const dice_counts&) = default;

        constexpr std::uint32_t total() const noexcept
        {
            std::uint32_t result = 0;
            for(auto value : values_)
            {
                result += value;
            }
            return result;
        }

        constexpr bool contains(const dice_counts& other) const noexcept
        {
            for(size_t index = 0; index < values_.size(); ++index)
            {
                if(values_[index] < other.values_[index])
                {
                    return false;
                }
            }
            return true;
        }

        constexpr dice_counts& operator+=(const dice_counts& other) noexcept
        {
            for(size_t index = 0; index < values_.size(); ++index)
            {
                values_[index] += other.values_[index];
            }
            return *this;
        }

        constexpr dice_counts& operator-=(const dice_counts& other) noexcept
        {
            for(size_t index = 0; index < values_.size(); ++index)
            {
                values_[index] -= other.values_[index];
            }
            return *this;
        }

    private:
        std::array<std::uint8_t, 8> values_{};
    };

    class elemental_dice_cost
    {
    public:
        constexpr elemental_dice_cost() noexcept : cost_list_{}
        {
        }

        constexpr elemental_dice_cost(elemental_dice dice, unsigned char count) : cost_list_{}
        {
            cost_list_[std::to_underlying(dice)] = count;
        }

        template<class Self> constexpr auto& operator[](this Self& self, elemental_dice dice) noexcept
        {
            return self.cost_list_[std::to_underlying(dice)];
        }

        friend constexpr elemental_dice_cost operator+(elemental_dice_cost l, elemental_dice_cost r) noexcept
        {
            elemental_dice_cost result;
            for(int i = 0; i < 8; ++i)
            {
                result.cost_list_[i] = l.cost_list_[i] + r.cost_list_[i];
            }
            return result;
        }

        friend constexpr elemental_dice_cost operator-(elemental_dice_cost l, elemental_dice_cost r) noexcept
        {
            elemental_dice_cost result;
            for(int i = 0; i < 8; ++i)
            {
                result.cost_list_[i] = l.cost_list_[i] - r.cost_list_[i];
            }
            return result;
        }

        friend constexpr elemental_dice_cost operator*(elemental_dice_cost cost, unsigned char multiplier) noexcept
        {
            elemental_dice_cost result;
            for(int i = 0; i < 8; ++i)
            {
                result.cost_list_[i] = cost.cost_list_[i] * multiplier;
            }
            return result;
        }

        friend constexpr elemental_dice_cost operator/(elemental_dice_cost cost, unsigned char divisor) noexcept
        {
            elemental_dice_cost result;
            for(int i = 0; i < 8; ++i)
            {
                result.cost_list_[i] = cost.cost_list_[i] / divisor;
            }
            return result;
        }

        friend constexpr bool operator==(elemental_dice_cost, elemental_dice_cost) = default;

    private:
        unsigned char cost_list_[8];
    };

    constexpr elemental_dice_cost operator*(elemental_dice dice, unsigned char count) noexcept
    {
        return elemental_dice_cost{ dice, count };
    }

    constexpr elemental_dice_cost operator*(unsigned char count, elemental_dice dice) noexcept
    {
        return elemental_dice_cost{ dice, count };
    }
}

#endif
