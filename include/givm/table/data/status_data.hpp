#ifndef GIVM_TABLE_DATA_STATUS_DATA_HPP
#define GIVM_TABLE_DATA_STATUS_DATA_HPP

#include <cstddef>
#include <cstdint>

#include "../../definition.hpp"

namespace givm
{
    inline constexpr size_t invalid_status_index = static_cast<size_t>(-1);

    struct status_state
    {
        std::uint32_t count;
    };

    struct status_data
    {
        definition_id<status_definition> definition_id;
        status_state state;
    };

    struct status_slot
    {
        status_data data;
        size_t next = invalid_status_index;
    };
}

#endif
