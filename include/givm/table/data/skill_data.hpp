#ifndef GIVM_TABLE_DATA_SKILL_DATA_HPP
#define GIVM_TABLE_DATA_SKILL_DATA_HPP

#include <cstddef>

#include "../definition_id.hpp"
#include <cstdint>

namespace givm
{
    struct skill_state
    {
        std::uint32_t count;
    };
}

namespace givm::detail
{
    struct skill_data
    {
        std::uint64_t definition_and_flags = static_cast<std::uint64_t>(-1);
        skill_state state;
    };
}

#endif
