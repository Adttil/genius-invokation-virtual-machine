#ifndef GIVM_TABLE_LINKED_DECK_HPP
#define GIVM_TABLE_LINKED_DECK_HPP

#include <vector>

#include "entity_fwd.hpp"
#include "tag_id.hpp"

namespace givm
{
    struct linked_deck
    {
        std::vector<definition_id<definition_category::card>> cards;
        std::vector<definition_id<definition_category::character>> characters;
    };
}

#endif
