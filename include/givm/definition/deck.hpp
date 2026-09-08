#ifndef GIVM_DEFINITION_DECK_HPP
#define GIVM_DEFINITION_DECK_HPP

#include <stdexcept>
#include <string_view>
#include <vector>

#include "issued_id.hpp"
#include "issued_id_map.hpp"
#include "types.hpp"

namespace givm
{
    struct linked_deck
    {
        std::vector<definition_id<card_definition>> cards;
        std::vector<definition_id<character_view>> characters;
    };

    template<class TCardNames, class TCharacterNames>
    linked_deck link_deck(
        const issued_id_map& id_map,
        TCardNames&& card_names,
        TCharacterNames&& character_names
    )
    {
        linked_deck result;
        for(auto&& item : card_names)
        {
            const std::string_view name{ item };
            if(not id_map.has<card_definition>(name))
            {
                throw std::invalid_argument{ "deck references an unavailable card definition" };
            }
            result.cards.push_back(id_map.get_id<card_definition>(name));
        }
        for(auto&& item : character_names)
        {
            const std::string_view name{ item };
            if(not id_map.has<character_view>(name))
            {
                throw std::invalid_argument{ "deck references an unavailable character definition" };
            }
            result.characters.push_back(id_map.get_id<character_view>(name));
        }
        return result;
    }
}

#endif
