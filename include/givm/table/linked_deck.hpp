#ifndef GIVM_TABLE_LINKED_DECK_HPP
#define GIVM_TABLE_LINKED_DECK_HPP

#include <vector>

#include "entity_fwd.hpp"
#include "issued_id.hpp"

namespace givm
{
    struct linked_deck
    {
        std::vector<definition_id<card_definition>> cards;
        std::vector<definition_id<character_view>> characters;
    };
}

#endif
