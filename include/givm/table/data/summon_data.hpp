#ifndef GIVM_TABLE_DATA_SUMMON_DATA_HPP
#define GIVM_TABLE_DATA_SUMMON_DATA_HPP

#include <cstddef>

#include "../definition_id.hpp"
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
        std::uint64_t definition_and_flags = static_cast<std::uint64_t>(-1);
        summon_state state;
    };
}

#endif
