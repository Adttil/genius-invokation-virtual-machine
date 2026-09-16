#ifndef GIVM_TABLE_DATA_STATUS_DATA_HPP
#define GIVM_TABLE_DATA_STATUS_DATA_HPP

#include <cstddef>
#include <cstdint>

namespace givm
{
    struct status_state
    {
        std::uint32_t count;
    };
}

namespace givm::detail
{
    inline constexpr size_t invalid_status_index = static_cast<size_t>(-1);

    struct status_data
    {
        size_t definition_and_flags = static_cast<size_t>(-1);
        status_state state;
    };

    struct status_slot
    {
        status_data data;
        size_t next = invalid_status_index;
    };
}

#endif
