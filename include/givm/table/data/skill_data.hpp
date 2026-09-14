#ifndef GIVM_TABLE_DATA_SKILL_DATA_HPP
#define GIVM_TABLE_DATA_SKILL_DATA_HPP

#include <cstdint>

#include "../entity_fwd.hpp"
#include "../issued_id.hpp"

namespace givm
{
    struct skill_state
    {
        std::uint32_t count;
    };

    struct skill_data
    {
        definition_id<skill_view> definition_id;
        skill_state state;
    };
}

#endif
