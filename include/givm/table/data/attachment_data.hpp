#ifndef GIVM_TABLE_DATA_ATTACHMENT_DATA_HPP
#define GIVM_TABLE_DATA_ATTACHMENT_DATA_HPP

#include <cstdint>

#include "../entity_fwd.hpp"
#include "../issued_id.hpp"

namespace givm
{
    struct attachment_state
    {
        std::uint32_t count;
    };

    struct attachment_data
    {
        definition_id<attachment_view> definition_id;
        attachment_state state;
    };
}

#endif
