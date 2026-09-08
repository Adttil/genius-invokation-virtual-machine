#ifndef GIVM_EXECUTOR_INSTRUCTIONS_ENTER_CHARACTER_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_ENTER_CHARACTER_HPP

#include "../executor.hpp"

#include "../events.hpp"

#include <utility>

namespace givm
{
    struct enter_character
    {
        using context_type = void;

        player_id player;
        definition_id<character_view> definition;

        bool execute(card_table& table, execution_context& context, random_fn& random) const
        {
            const auto character = table[player].add(definition, character_state{}).id();
            character_initialization event{};
            const auto character_entity = std::as_const(table)[character];
            (void)character_entity.definition().template handle<character_initialization>(
                character_entity,
                event,
                std::as_const(table),
                random
            );
            table[character].state() = event.state;
            return context.enter_next();
        }
    };
}

#endif
