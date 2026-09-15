#ifndef GIVM_EXECUTOR_INSTRUCTIONS_DRAW_CARDS_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_DRAW_CARDS_HPP

#include "../executor.hpp"
#include "../../definition/commands.hpp"

#include <cstdint>
#include <vector>

#include "../broadcast.hpp"
#include "../../definition/events.hpp"

namespace givm::detail
{
    // A batch has a private [hand_card_id[], stack_count_t] frame and one
    // active broadcast. This continuation does not depend on adjacent opcodes.
    inline execution_state broadcast_drawn_card(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        while(true)
        {
            if(not continue_broadcast<card_drawn>(library, table, context, random))
                return continue_execution;
            pop_broadcast<card_drawn>(context);
            auto&& [cards, cursor] = context.stack().top<hand_card_id[], stack_count_t>();
            if(cursor == cards.size())
            {
                context.stack().pop<hand_card_id[], stack_count_t>();
                return context.enter_next();
            }
            prepare_broadcast(library, card_drawn{ .card = cards[cursor++] }, table, context.stack());
        }
    }

    inline void prepare_drawn_cards(
        const definition_library& library, const unrestricted_table& table,
        execution_context& context, const std::vector<hand_card_id>& cards)
    {
        context.stack().push(dynamic_array<hand_card_id>(cards), stack_count_t{ 1 });
        prepare_broadcast(library, card_drawn{ .card = cards.front() }, table, context.stack());
    }

    inline execution_state draw_cards_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        const auto& command = context.instruction_data<1, givm::draw_cards>(library);
        const auto target_player = command.player == relative_player::current
            ? table.state().active_player
            : other_player(table.state().active_player);
        auto player_entity = table[target_player];
        std::vector<hand_card_id> drawn_cards;
        drawn_cards.reserve(command.count);

        for(std::uint32_t index = 0; index < command.count; ++index)
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
            drawn_cards.push_back(player_entity.add_hand_card(std::move(card)).id());
        }

        if(drawn_cards.empty())
        {
            return context.advance(instruction_extent<1, givm::draw_cards> + sizeof(execute_fn));
        }

        prepare_drawn_cards(library, table, context, drawn_cards);
        return context.advance(instruction_extent<1, givm::draw_cards>);
    }

    inline void compile(program_writer& writer, const givm::draw_cards& command, compile_mode)
    {
        writer.write(execute_fn{ &draw_cards_execute });
        writer.write(command);
        writer.write(execute_fn{ &broadcast_drawn_card });
    }
}

#endif
