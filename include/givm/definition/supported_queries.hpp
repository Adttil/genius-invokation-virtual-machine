#ifndef GIVM_DEFINITION_SUPPORTED_QUERIES_HPP
#define GIVM_DEFINITION_SUPPORTED_QUERIES_HPP

#include "queries.hpp"
#include "definition_categories.hpp"

namespace givm
{
    template<class TCategory>
    struct supported_queries : type_list<>{};

    template<>
    struct supported_queries<card_definition> : type_list<
        card_initial_state,
        card_target_validation
    >{};

    template<>
    struct supported_queries<status_definition> : type_list<
        card_state_modification
    >{};

    template<>
    struct supported_queries<character_view> : type_list<
        character_initial_state,
        character_initial_skill
    >{};

    template<>
    struct supported_queries<skill_view> : type_list<
        skill_initial_cost,
        skill_target_validation
    >{};
}

#endif
