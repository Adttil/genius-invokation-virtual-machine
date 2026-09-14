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
    };

    namespace detail
    {
        template<>
        struct instruction_implementation<start_round>
        {
            enum class stage_type : stage_t
            {
                prepare,
                finish
            };

            template<bool Observed>
            static execution_state execute(
                const givm::start_round& instruction, const definition_library&,
                card_table& table, execution_context& context, random_fn&
            )
            {
                auto& state = table.state();
                if constexpr(Observed)
                {
                    if(static_cast<stage_type>(context.current_stage()) == stage_type::prepare)
                    {
                        ++state.round_number;
                        context.current_stage() = static_cast<stage_t>(stage_type::finish);
                        return execution_state::round_started;
                    }
                }
                else
                {
                    ++state.round_number;
                }

                if(state.round_number > instruction.max_rounds) [[unlikely]]
                {
                    return context.end_game(game_result::both_loss);
                }

                for(auto player : table.players())
                {
                    player.state().dice = {};
                }

                return context.enter_next();
            }

        };
    }
}

#endif
