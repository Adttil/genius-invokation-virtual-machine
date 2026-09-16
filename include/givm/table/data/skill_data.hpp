#ifndef GIVM_TABLE_DATA_SKILL_DATA_HPP
#define GIVM_TABLE_DATA_SKILL_DATA_HPP

#include <cstddef>
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
        size_t definition_and_flags = static_cast<size_t>(-1);
        skill_state state;
    };
}

#endif
