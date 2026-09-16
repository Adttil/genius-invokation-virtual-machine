#ifndef GIVM_TABLE_DATA_ATTACHMENT_DATA_HPP
#define GIVM_TABLE_DATA_ATTACHMENT_DATA_HPP

#include <cstddef>
#include <cstdint>

namespace givm
{
    struct attachment_state
    {
        std::uint32_t count;
    };
}

namespace givm::detail
{
    struct attachment_data
    {
        size_t definition_and_flags = static_cast<size_t>(-1);
        attachment_state state;
    };
}

#endif
