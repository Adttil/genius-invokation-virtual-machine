#ifndef GIVM_TABLE_DATA_SUPPORT_DATA_HPP
#define GIVM_TABLE_DATA_SUPPORT_DATA_HPP

#include <cstddef>

#include "../definition_id.hpp"
#include <cstdint>

namespace givm
{
    struct support_state
    {
        std::uint32_t count;
        std::uint32_t round_usages;
    };
}

namespace givm::detail
{
    struct support_data
    {
        std::uint64_t definition_and_flags = static_cast<std::uint64_t>(-1);
        support_state state;
    };
}

#endif
