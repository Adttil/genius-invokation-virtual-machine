#ifndef GIVM_EXECUTOR_INSTRUCTIONS_INSERT_DECK_CARD_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_INSERT_DECK_CARD_HPP

#include "../executor.hpp"

#include <cstddef>
#include <cstdint>

#include "../../table.hpp"
#include "../../utils/debug.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    struct insert_deck_card
    {
        using context_type = void;

        player_id player;
        definition_id<card_definition> definition;
        std::int32_t position = -1;

        bool execute(card_table& table, execution_context& context, random_fn&) const
        {
            auto player_entity = table[player];
            const auto size = player_entity.deck_card_count();
            size_t index;
            if(position >= 0)
            {
                index = static_cast<size_t>(position);
                GIVM_ASSERT(index <= size);
            }
            else
            {
                const auto offset_from_top = static_cast<size_t>(-position - 1);
                GIVM_ASSERT(offset_from_top <= size);
                index = size - offset_from_top;
            }
            player_entity.insert_deck_card(index, definition, card_state{});
            return context.enter_next();
        }
    };
}

#include "../../macro_undef.hpp"
#endif
