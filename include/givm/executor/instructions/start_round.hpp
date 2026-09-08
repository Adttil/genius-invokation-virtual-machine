#ifndef GIVM_EXECUTOR_INSTRUCTIONS_START_ROUND_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_START_ROUND_HPP

#include "../executor.hpp"

#include <cstdint>

namespace givm
{
    struct start_round
    {
        using context_type = void;

        std::uint32_t max_rounds = 14;

        bool execute(card_table& table, execution_context& context, random_fn&) const
        {
            auto& state = table.state();
            const auto next_round = state.round_number + 1;
            if(next_round > max_rounds)
            {
                return context.end_game(game_result::both_loss);
            }
            state.round_number = next_round;

            for(auto player : table.players())
            {
                player.state().dice = {};
            }

            return context.enter_next();
        }
    };
}

#endif
