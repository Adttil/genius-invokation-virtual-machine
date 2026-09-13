#ifndef GIVM_EXECUTOR_INSTRUCTIONS_DRAW_CARDS_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_DRAW_CARDS_HPP

#include "../executor.hpp"

#include <cstdint>
#include <vector>

#include "../broadcast.hpp"
#include "../events.hpp"

namespace givm
{
    enum class relative_player : std::uint8_t
    {
        current,
        other
    };

    struct draw_cards
    {
        using context_type = void;

        std::uint32_t count;
        relative_player player = relative_player::current;
    };

    namespace detail
    {
        template<>
        struct instruction_implementation<draw_cards>
        {
            enum class stage_type : stage_t
            {
                draw,
                prepare_card_drawn,
                broadcast_card_drawn
            };

            template<bool Observed>
            static execution_state execute(
                const givm::draw_cards& instruction, card_table& table, execution_context& context, random_fn& random
            )
            {
                const auto stage = static_cast<stage_type>(context.current_stage());
                if(stage == stage_type::prepare_card_drawn)
                {
                    auto&& [drawn_cards, cursor, stored_stage] =
                        context.stack().top<hand_card_id[], stack_count_t, stage_t>();
                    (void)stored_stage;
                    if(cursor == drawn_cards.size())
                    {
                        context.stack().pop<hand_card_id[], stack_count_t, stage_t>();
                        return context.enter_next();
                    }

                    detail::prepare_broadcast(card_drawn{ .card = drawn_cards[cursor++] }, table, context.stack());
                    context.current_stage() = static_cast<stage_t>(stage_type::broadcast_card_drawn);
                    return continue_execution;
                }

                if(stage == stage_type::broadcast_card_drawn)
                {
                    if(not detail::continue_broadcast<card_drawn>(table, context, random))
                    {
                        return continue_execution;
                    }

                    detail::pop_broadcast<card_drawn>(context);
                    return continue_execution;
                }

                const auto target_player = instruction.player == relative_player::current
                    ? table.state().active_player
                    : other_player(table.state().active_player);
                auto player_entity = table[target_player];
                std::vector<hand_card_id> drawn_cards;
                drawn_cards.reserve(instruction.count);

                for(std::uint32_t index = 0; index < instruction.count; ++index)
                {
                    if(player_entity.deck_card_count() == 0)
                    {
                        break;
                    }

                    if(player_entity.hand_card_count() >= table.parameters().hand_limit)
                    {
                        player_entity.discard_top_deck_card();

                        continue;
                    }
                    auto card = player_entity.take_top_deck_card();
                    const auto destination = player_entity.add_hand_card(std::move(card)).id();
                    drawn_cards.push_back(destination);

                }

                if(drawn_cards.empty())
                {
                    return context.enter_next();
                }

                context.stack().push(
                    dynamic_array<hand_card_id>(drawn_cards),
                    stack_count_t{},
                    static_cast<stage_t>(stage_type::prepare_card_drawn)
                );

                return continue_execution;
            }

        };
    }
}

#endif
