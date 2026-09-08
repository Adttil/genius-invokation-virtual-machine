#ifndef GIVM_EXECUTOR_INSTRUCTIONS_SET_ELEMENT_AURA_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_SET_ELEMENT_AURA_HPP

#include "../executor.hpp"

#include "../../table.hpp"

namespace givm
{
    struct set_element_aura
    {
        using context_type = void;

        character_id target;
        element_aura aura;

        bool execute(card_table& table, execution_context& context, random_fn&) const
        {
            table[target].state().aura = aura;
            return context.enter_next();
        }
    };

}

#endif
