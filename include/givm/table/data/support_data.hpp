#ifndef GIVM_TABLE_DATA_SUPPORT_DATA_HPP
#define GIVM_TABLE_DATA_SUPPORT_DATA_HPP

#include <cstdint>

#include "../../definition.hpp"

namespace givm
{
    struct support_state
    {
        std::uint32_t count;
    };

    struct support_data
    {
        definition_id<support_view> definition_id;
        support_state state;
    };
}

#endif
