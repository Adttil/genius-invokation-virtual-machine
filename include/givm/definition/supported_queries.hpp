#ifndef GIVM_DEFINITION_SUPPORTED_QUERIES_HPP
#define GIVM_DEFINITION_SUPPORTED_QUERIES_HPP

#include "queries.hpp"
#include "definition_categories.hpp"

namespace givm
{
    template<definition_category TCategory>
    struct supported_queries : type_list<>{};

    template<>
    struct supported_queries<definition_category::card> : type_list<
        card_initial_state,
        card_target_validation,
        card_equipment_target_validation
    >{};

    template<>
    struct supported_queries<definition_category::card_status> : type_list<
        card_state_modification
    >{};

    template<>
    struct supported_queries<definition_category::character> : type_list<
        character_initial_state,
        character_initial_skill,
        character_reaction_override
    >{};

    template<>
    struct supported_queries<definition_category::skill> : type_list<
        skill_initial_cost,
        skill_target_validation
    >{};

    template<>
    struct supported_queries<definition_category::reaction> : type_list<reaction_aura>{};

    template<>
    struct supported_queries<definition_category::summon> : type_list<summon_state_limit>{};

    template<>
    struct supported_queries<definition_category::support> : type_list<support_state_limit>{};

    template<>
    struct supported_queries<definition_category::combat_status> : type_list<combat_status_state_limit>{};

    template<>
    struct supported_queries<definition_category::attachment> : type_list<attachment_state_limit, technique_initial_cost, technique_target_validation>{};
}

#endif
