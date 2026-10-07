#ifndef GIVM_TABLE_DATA_ATTACHMENT_DATA_HPP
#define GIVM_TABLE_DATA_ATTACHMENT_DATA_HPP

#include <cstddef>

#include "../definition_id.hpp"
#include <cstdint>

namespace givm
{
    struct attachment_state
    {
        std::uint32_t count;
        std::uint32_t round_usages;
    };
}

namespace givm::detail
{
    struct attachment_data
    {
        std::uint64_t definition_and_flags = static_cast<std::uint64_t>(-1);
        attachment_state state;
    };
}

#endif
