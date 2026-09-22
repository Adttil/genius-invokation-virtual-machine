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

    template<>
    struct supported_queries<summon_view> : type_list<summon_state_limit>{};

    template<>
    struct supported_queries<support_view> : type_list<support_state_limit>{};

    template<>
    struct supported_queries<combat_status_view> : type_list<combat_status_state_limit>{};

    template<>
    struct supported_queries<attachment_view> : type_list<attachment_state_limit, technique_initial_cost, technique_target_validation>{};
}

#endif
