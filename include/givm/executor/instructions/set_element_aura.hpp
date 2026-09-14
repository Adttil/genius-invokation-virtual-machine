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
    };

    template<>
    struct detail::instruction_implementation<set_element_aura>
    {
        template<bool Observed>
        static execution_state execute(
            const givm::set_element_aura& instruction, const definition_library&,
            unrestricted_table& table, execution_context& context, random_fn&
        )
        {
            table[instruction.target].state().aura = instruction.aura;
            return context.enter_next();
        }
    };

}

#endif
