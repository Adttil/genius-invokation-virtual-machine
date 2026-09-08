#ifndef GIVM_TABLE_DATA_COMBAT_STATUS_DATA_HPP
#define GIVM_TABLE_DATA_COMBAT_STATUS_DATA_HPP

#include <cstdint>

#include "../../definition.hpp"

namespace givm
{
    struct combat_status_state
    {
        std::uint32_t count;
    };

    struct combat_status_data
    {
        definition_id<combat_status_view> definition_id;
        combat_status_state state;
    };
}

#endif
