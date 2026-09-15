#ifndef GIVM_EXECUTOR_PAYMENT_HPP
#define GIVM_EXECUTOR_PAYMENT_HPP

#include <cstdint>

#include "../definition.hpp"

namespace givm
{
    enum class payment_check_result : std::uint8_t
    {
        valid,
        requirement_mismatch,
        insufficient_dice
    };
}

namespace givm::detail
{
    constexpr bool payment_matches(
        const elemental_dice_requirement& requirement, const dice_counts& paid
    ) noexcept
    {
        const std::uint32_t required_total =
            requirement.fixed.total() + requirement.same + requirement.any;
        if(paid.total() != required_total)
        {
            return false;
        }

        std::uint32_t required_omni = requirement.fixed[elemental_dice::omni];
        std::uint32_t largest_remaining_group = 0;
        for(std::uint8_t index = 1; index < 8; ++index)
        {
            const auto dice = static_cast<elemental_dice>(index);
            const std::uint32_t fixed = requirement.fixed[dice];
            const std::uint32_t count = paid[dice];
            if(count < fixed)
            {
                required_omni += fixed - count;
            }
            else
            {
                const auto remaining = count - fixed;
                if(remaining > largest_remaining_group)
                {
                    largest_remaining_group = remaining;
                }
            }
        }

        const std::uint32_t omni = paid[elemental_dice::omni];
        return omni >= required_omni
            && largest_remaining_group + (omni - required_omni) >= requirement.same;
    }
}

#endif
