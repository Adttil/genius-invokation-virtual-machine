#ifndef GIVM_DEFINITION_COMMANDS_START_DICE_ROLL_PHASE_HPP
#define GIVM_DEFINITION_COMMANDS_START_DICE_ROLL_PHASE_HPP

#include <array>
#include <cstdint>
#include <string>

namespace givm
{
    enum class start_dice_roll_phase_error {};

    inline std::string error_string(start_dice_roll_phase_error)
    {
        return {};
    }

    struct start_dice_roll_phase
    {
        using error_type = start_dice_roll_phase_error;

        std::uint32_t count = 8;
        std::array<std::uint32_t, 2> reroll_count{ 1, 1 };

    };
}

#endif
