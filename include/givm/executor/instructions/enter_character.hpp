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
    };

    namespace detail
    {
        template<>
        struct instruction_implementation<enter_character>
        {
            template<bool Observed>
            static execution_state execute(
                const givm::enter_character& instruction, const definition_library& library,
                unrestricted_table& table, execution_context& context, random_fn& random
            )
            {
                const auto character = table[instruction.player].add(instruction.definition, character_state{}).id();
                character_initialization event{};
                const auto character_entity = std::as_const(table)[character];
                (void)library[character_entity.definition_id()].template handle<character_initialization>(
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
}

#endif
