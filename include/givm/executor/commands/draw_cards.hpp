#ifndef GIVM_EXECUTOR_COMMANDS_DRAW_CARDS_HPP
#define GIVM_EXECUTOR_COMMANDS_DRAW_CARDS_HPP

#include "../executor.hpp"
#include "../../definition/commands.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>

#include "../broadcast.hpp"
#include "../../definition.hpp"

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
            prepare_broadcast(library, card_drawn{ .card = cards[cursor++] }, table, context.stack(), context.position());
        }
    }

    inline void prepare_drawn_cards(
        const definition_library& library, const unrestricted_table& table,
        execution_context& context, execution_position return_position)
    {
        const auto first_card = get<0>(context.stack().top<hand_card_id[], stack_count_t>()).front();
        prepare_broadcast(library, card_drawn{ .card = first_card }, table, context.stack(), return_position);
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
        const auto count = std::min<size_t>(command.count, player_entity.deck_card_count());
        if(count == 0)
        {
            return context.advance(instruction_extent<1, givm::draw_cards> + sizeof(execute_fn));
        }

        const auto hand_count = player_entity.hand_card_count();
        const auto hand_limit = player_entity.state().hand_limit;
        const auto drawn_count = hand_count < hand_limit
            ? std::min<size_t>(count, hand_limit - hand_count) : size_t{ 0 };

        // No responses run until the entire batch, including overflow discards, is complete.
        if(drawn_count != 0)
        {
            auto drawn_cards = get<0>(context.stack().push(
                dynamic_array<hand_card_id>(drawn_count), stack_count_t{ 1 }));
            for(auto& id : drawn_cards)
            {
                auto card = player_entity.take_top_deck_card();
                std::construct_at(&id, player_entity.add_hand_card(std::move(card)).id());
            }
        }
        for(size_t index = drawn_count; index < count; ++index)
        {
            player_entity.discard_top_deck_card();
        }

        if(drawn_count == 0)
        {
            return context.advance(instruction_extent<1, givm::draw_cards> + sizeof(execute_fn));
        }

        prepare_drawn_cards(library, table, context, context.position() + instruction_extent<1, givm::draw_cards>);
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
