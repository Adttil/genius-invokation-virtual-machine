#ifndef GIVM_EXECUTOR_COMMANDS_REPLACE_CARDS_BOTH_HPP
#define GIVM_EXECUTOR_COMMANDS_REPLACE_CARDS_BOTH_HPP

#include "../executor.hpp"
#include "../../definition/commands.hpp"

#include <bitset>
#include <cstddef>
#include <cstdint>

#include "replace_cards.hpp"

namespace givm::detail
{
    inline execution_state prepare_initial_card_selection(
        const definition_library&, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        const auto player0_random_count = table[player_id{ 0 }].hand_card_count();
        const auto player1_random_count = table[player_id{ 1 }].hand_card_count();
        auto random_pool = get<0>(context.stack().push(
            dynamic_array<std::uint32_t>(player0_random_count + player1_random_count),
            static_cast<std::uint32_t>(player0_random_count),
            player_id{ 0 },
            std::bitset<selection_capacity>{}
        ));
        for(auto& value : random_pool)
        {
            value = random();
        }
        return context.yield_next(execution_state::initial_card_selection);
    }

    template<bool First>
    inline execution_state apply_initial_card_selection(
        const definition_library&, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        auto&& [random_pool, player0_random_count, input_player, input_selected] =
            context.stack().top<std::uint32_t[], std::uint32_t, player_id, std::bitset<selection_capacity>>();
        const auto player = input_player;
        const auto selected = input_selected;

        size_t random_index = player == player_id{ 0 } ? 0 : static_cast<size_t>(player0_random_count);
        auto next_random = [&]
        {
            return random_pool[random_index++];
        };
        auto on_drawn = [](hand_card_id){};
        detail::replace_cards(table, player, selected, next_random, on_drawn);

        if constexpr(First)
        {
            input_player = other_player(player);
            input_selected.reset();
            return context.yield_next(execution_state::card_selection);
        }
        else
        {
            context.stack().pop<std::uint32_t[], std::uint32_t, player_id, std::bitset<selection_capacity>>();
            return context.enter_next();
        }
    }

    inline void compile(program_writer& writer, const givm::replace_cards_both&, compile_mode)
    {
        writer.write(execute_fn{ &prepare_initial_card_selection });
        writer.write(execute_fn{ &apply_initial_card_selection<true> });
        writer.write(execute_fn{ &apply_initial_card_selection<false> });
    }
}

#endif
