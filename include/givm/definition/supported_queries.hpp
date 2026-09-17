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
        card_initial_cost,
        card_target_validation
    >{};

    template<>
    struct supported_queries<character_view> : type_list<character_initial_state>{};
}

#endif
