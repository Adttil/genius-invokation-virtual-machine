#ifndef GIVM_EXECUTOR_INSTRUCTIONS_INITIALIZE_CHARACTERS_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_INITIALIZE_CHARACTERS_HPP

#include <utility>

#include "../events.hpp"
#include "../executor.hpp"

namespace givm
{
    struct initialize_characters
    {
        using context_type = void;

        player_id player;

        bool execute(card_table& table, execution_context& context, random_fn& random) const
        {
            for(const auto character : table[player].characters())
            {
                character_initialization event{};
                const auto entity = std::as_const(table)[character.id()];
                (void)entity.definition().template handle<character_initialization>(
                    entity,
                    event,
                    std::as_const(table),
                    random
                );
                character.state() = event.state;
            }
            return context.enter_next();
        }
    };
}

#endif
