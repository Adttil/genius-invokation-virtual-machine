#ifndef GIVM_TABLE_DATA_COMBAT_STATUS_DATA_HPP
#define GIVM_TABLE_DATA_COMBAT_STATUS_DATA_HPP

#include <cstddef>
#include <cstdint>

namespace givm
{
    struct combat_status_state
    {
        std::uint32_t count;
        std::uint32_t round_usages;
    };
}

namespace givm::detail
{
    struct combat_status_data
    {
        size_t definition_and_flags = static_cast<size_t>(-1);
        combat_status_state state;
    };
}

#endif
