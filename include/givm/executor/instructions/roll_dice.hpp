#ifndef GIVM_EXECUTOR_INSTRUCTIONS_ROLL_DICE_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_ROLL_DICE_HPP

#include "../executor.hpp"

#include <bitset>
#include <cstddef>

#include "push_selector.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    namespace detail
    {
        inline dice_counts selected_dice_from_mask(
            dice_counts dice,
            std::bitset<selection_capacity> selected_mask
        )
        {
            dice_counts selected_dice;
            size_t selection_index = 0;
            for(size_t dice_index = 0; dice_index < 8; ++dice_index)
            {
                const auto current_dice = static_cast<elemental_dice>(dice_index);
                for(std::uint8_t count = 0; count < dice[current_dice]; ++count)
                {
                    GIVM_ASSERT(selection_index < selection_capacity);
                    if(selected_mask[selection_index])
                    {
                        ++selected_dice[current_dice];
                    }
                    ++selection_index;
                }
            }
            GIVM_ASSERT(selected_mask.count() == selected_dice.total());
            return selected_dice;
        }

        inline void roll_selected_dice(
            card_table& table,
            player_id player,
            std::bitset<selection_capacity> selected_mask,
            random_fn& random
        )
        {
            auto& state = table[player].state();
            const auto selected = selected_dice_from_mask(state.dice, selected_mask);
            const auto selected_count = selected.total();
            state.dice -= selected;
            for(std::uint32_t index = 0; index < selected_count; ++index)
            {
                ++state.dice[elemental_dice_from_random(random())];
            }
        }
    }

    struct roll_dice
    {
        using context_type = void;

        player_id player;

        bool execute(card_table& table, execution_context& context, random_fn& random) const
        {
            auto&& [input] = context.stack().top<selector>();
            GIVM_ASSERT(input.player == player);

            const auto selected_mask = input.selected;
            context.stack().pop<selector>();

            detail::roll_selected_dice(table, player, selected_mask, random);
            return context.enter_next();
        }
    };
}

#include "../../macro_undef.hpp"
#endif
