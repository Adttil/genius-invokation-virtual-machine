#ifndef GIVM_TABLE_DATA_SUPPORT_DATA_HPP
#define GIVM_TABLE_DATA_SUPPORT_DATA_HPP

#include <cstddef>
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
        size_t definition_and_flags = static_cast<size_t>(-1);
        support_state state;
    };
}

#endif
