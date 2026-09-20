#ifndef GIVM_TABLE_ACTION_COST_REQUIREMENT_HPP
#define GIVM_TABLE_ACTION_COST_REQUIREMENT_HPP

#include <cstdint>

#include "../enums/action_speed.hpp"
#include "../enums/elemental_dice.hpp"
#include "issued_id.hpp"

namespace givm
{
    struct action_cost_requirement
    {
        elemental_dice_requirement dice_requirement;
        action_speed speed = action_speed::combat;
        std::uint32_t energy = 0;
        tag_id energy_tag{};
    };
}

#endif
