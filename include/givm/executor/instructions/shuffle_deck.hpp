#ifndef GIVM_EXECUTOR_INSTRUCTIONS_SHUFFLE_DECK_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_SHUFFLE_DECK_HPP

#include <cstddef>
#include <cstdint>

#include "../executor.hpp"

namespace givm
{
    struct shuffle_deck
    {
        using context_type = void;

        player_id player;

        bool execute(card_table& table, execution_context& context, random_fn& random) const
        {
            const auto target = table[player];
            for(size_t remaining = target.deck_card_count(); remaining > 1; --remaining)
            {
                const size_t selected = static_cast<size_t>(
                    static_cast<std::uint64_t>(random()) * remaining >> 32
                );
                target.swap_deck_cards(remaining - 1, selected);
            }
            return context.enter_next();
        }
    };
}

#endif
