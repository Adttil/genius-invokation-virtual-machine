#ifndef GIVM_DEFINITION_DEFINITION_CATEGORIES_HPP
#define GIVM_DEFINITION_DEFINITION_CATEGORIES_HPP

#include "../table.hpp"
#include "../utils/type_list.hpp"

namespace givm
{
    template<class TDefinition>
    struct views_of_definition : type_list<TDefinition>{};

    template<>
    struct views_of_definition<card_definition> : type_list<
        hand_card_view,
        deck_card_view
    >{};

    template<>
    struct views_of_definition<status_definition> : type_list<
        hand_card_status_view,
        deck_card_status_view
    >{};

    using definition_types = type_list<
        card_definition,
        status_definition,
        support_view,
        summon_view,
        combat_status_view,
        character_view,
        skill_view,
        attachment_view
    >;
}

#endif
