#ifndef GIVM_TABLE_DATA_SUMMON_DATA_HPP
#define GIVM_TABLE_DATA_SUMMON_DATA_HPP

#include <cstddef>
#include <cstdint>

namespace givm
{
    struct summon_state
    {
        std::uint32_t value;
        std::uint32_t usages;
    };
}

namespace givm::detail
{
    struct summon_data
    {
        size_t definition_and_flags = static_cast<size_t>(-1);
        summon_state state;
    };
}

#endif
