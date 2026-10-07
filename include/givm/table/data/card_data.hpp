#ifndef GIVM_TABLE_DATA_CARD_DATA_HPP
#define GIVM_TABLE_DATA_CARD_DATA_HPP

#include <cstddef>

#include "../definition_id.hpp"

#include "../action_cost_requirement.hpp"
#include "status_data.hpp"

namespace givm
{
    struct card_state
    {
        action_cost_requirement cost{ .dice_requirement = {}, .speed = action_speed::fast };
        bool elemental_tuning_allowed = true;
    };
}

namespace givm::detail
{
    struct card_data
    {
        std::uint64_t definition_and_flags = static_cast<std::uint64_t>(-1);
        card_state state;
        size_t first_status = invalid_status_index;
        size_t last_status = invalid_status_index;
    };
}

#endif
