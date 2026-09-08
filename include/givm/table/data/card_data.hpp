#ifndef GIVM_TABLE_DATA_CARD_DATA_HPP
#define GIVM_TABLE_DATA_CARD_DATA_HPP

#include "../../definition.hpp"
#include "status_data.hpp"

namespace givm
{
    struct card_state
    {};

    struct card_data
    {
        definition_id<card_definition> definition_id;
        card_state state;
        size_t first_status = invalid_status_index;
        size_t last_status = invalid_status_index;
    };
}

#endif
