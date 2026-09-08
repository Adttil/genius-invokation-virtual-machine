#ifndef GIVM_EXECUTOR_INSTRUCTIONS_REPLACE_CARDS_BOTH_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_REPLACE_CARDS_BOTH_HPP

#include "../executor.hpp"

#include <cstddef>
#include <cstdint>

#include "replace_cards.hpp"

namespace givm
{
    struct replace_cards_both
    {
        using context_type = void;

        enum class stage_type : stage_t
        {
            prepare,
            first_selection,
            second_selection
        };
        bool execute(card_table& table, execution_context& context, random_fn& random) const
        {
            const auto stage = static_cast<stage_type>(context.current_stage());
            if(stage == stage_type::prepare)
            {
                const auto player0_random_count = table[player_id{ 0 }].hand_card_count();
                const auto player1_random_count = table[player_id{ 1 }].hand_card_count();
                auto&& [random_pool, player0_random_count_slot, input, stored_stage] = context.stack().push(
                    dynamic_array<std::uint32_t>(player0_random_count + player1_random_count),
                    static_cast<std::uint32_t>(player0_random_count),
                    selector{ .player = player_id{ 0 } },
                    static_cast<stage_t>(stage_type::first_selection)
                );
                (void)player0_random_count_slot;
                (void)input;
                (void)stored_stage;
                for(auto& value : random_pool)
                {
                    value = random();
                }

                return context.yield();
            }

            auto&& [random_pool, player0_random_count, input, stored_stage] =
                context.stack().top<std::uint32_t[], std::uint32_t, selector, stage_t>();
            const auto player = input.player;
            const auto selected = input.selected;

            size_t random_index = player == player_id{ 0 } ? 0 : static_cast<size_t>(player0_random_count);
            auto next_random = [&]
            {
                return random_pool[random_index++];
            };
            auto on_drawn = [](hand_card_id){};
            detail::replace_cards(table, player, selected, next_random, on_drawn);

            if(stage == stage_type::first_selection)
            {
                input.player = other_player(player);
                input.selected.reset();
                stored_stage = static_cast<stage_t>(stage_type::second_selection);
                return context.yield();
            }

            context.stack().pop<std::uint32_t[], std::uint32_t, selector, stage_t>();
            return context.enter_next();
        }
    };
}

#endif
