#ifndef GIVM_TABLE_DATA_CARD_DATA_HPP
#define GIVM_TABLE_DATA_CARD_DATA_HPP

#include <cstddef>

#include "status_data.hpp"

namespace givm
{
    struct card_state
    {};
}

namespace givm::detail
{
    struct card_data
    {
        size_t definition_and_flags = static_cast<size_t>(-1);
        card_state state;
        size_t first_status = invalid_status_index;
        size_t last_status = invalid_status_index;
    };
}

#endif
