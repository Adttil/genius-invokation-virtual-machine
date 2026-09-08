#ifndef GIVM_TABLE_DATA_SUMMON_DATA_HPP
#define GIVM_TABLE_DATA_SUMMON_DATA_HPP

#include <cstdint>

#include "../../definition.hpp"

namespace givm
{
    struct summon_state
    {
        std::uint32_t count;
    };

    struct summon_data
    {
        definition_id<summon_view> definition_id;
        summon_state state;
    };
}

#endif
