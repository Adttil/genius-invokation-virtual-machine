#ifndef GIVM_EXECUTOR_COMMANDS_DISCARD_HAND_CARD_HPP
#define GIVM_EXECUTOR_COMMANDS_DISCARD_HAND_CARD_HPP

#include <algorithm>
#include <memory>

#include "../broadcast.hpp"
#include "../../definition.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    struct hand_card_discard_progress
    {
        std::size_t current = 0;
        std::size_t count;
    };

    inline void prepare_hand_card_discard_notification(
        const definition_library& library, const unrestricted_table& table,
        execution_context& context, hand_card_id card)
    {
        context.enter_next();
        prepare_broadcast(library, hand_card_discarded{ card }, table, context.stack(), context.position());
    }

    inline void finish_hand_card_discard_effect_frame(
        const definition_library& library, const unrestricted_table& table, execution_context& context)
    {
        const auto card = get<0>(context.stack().top<hand_card_discard_effect, response_return>()).card;
        context.stack().pop<hand_card_discard_effect, response_return>();
        prepare_hand_card_discard_notification(library, table, context, card);
    }

    inline bool enter_hand_card_discard_effect(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        const auto [cards, progress] = context.stack().top<frame<hand_card_id[]>, frame<hand_card_discard_progress>>();
        const auto card = get<0>(cards)[get<0>(progress).current];
        const auto self = std::as_const(table)[card];
        const auto definition = library[self.definition_id()];
        if(not definition.can_handle<hand_card_discard_effect, hand_card_view>())
        {
            prepare_hand_card_discard_notification(library, table, context, card);
            return false;
        }
        context.stack().push(hand_card_discard_effect{ card }, response_return{ table.state().self_player, context.position() });
        auto& event = get<0>(context.stack().top<hand_card_discard_effect, response_return>());
        auto response = context.make_handle_context(table, random);
        const auto entry = definition.handle<hand_card_discard_effect>(self, event, response);
        if(entry)
        {
            table.state().self_player = card.player_id;
            context.enter(entry);
            return true;
        }
        finish_hand_card_discard_effect_frame(library, table, context);
        return false;
    }

    inline execution_state broadcast_hand_card_discard(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        for(;;)
        {
            if(not continue_broadcast<hand_card_discarded>(library, table, context, random))
                return continue_execution;
            pop_broadcast<hand_card_discarded>(context);
            auto& progress = get<0>(context.stack().top<hand_card_discard_progress>());
            if(++progress.current == progress.count)
            {
                context.stack().pop<hand_card_discard_progress>();
                context.stack().pop<hand_card_id[]>();
                return context.enter_next();
            }
            context.jump(context.position() - sizeof(execute_fn));
            if(enter_hand_card_discard_effect(library, table, context, random))
                return continue_execution;
        }
    }

    inline execution_state finish_hand_card_discard_effect(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        finish_hand_card_discard_effect_frame(library, table, context);
        return broadcast_hand_card_discard(library, table, context, random);
    }

    template<bool Fixed>
    inline execution_state prepare_hand_card_discard(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        std::size_t count;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, discard_hand_card>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            const auto capacity = std::min<std::size_t>(command.count, table[player].hand_cards<false>().size());
            const auto cards = get<0>(context.stack().push(dynamic_array<hand_card_id>(capacity)));
            count = 0;
            if(capacity != 0)
            {
                for(const auto card : table[player].hand_cards())
                {
                    if(card.definition_id() != command.definition) continue;
                    std::construct_at(cards.data() + count++, card.id());
                    if(count == capacity) break;
                }
            }
            context.advance(instruction_extent<1, discard_hand_card>);
        }
        else
        {
            count = get<0>(context.stack().top<hand_card_id[]>()).size();
            context.enter_next();
        }
        if(count == 0)
        {
            context.stack().pop<hand_card_id[]>();
            return context.advance(2 * sizeof(execute_fn));
        }
        const auto cards = get<0>(context.stack().top<hand_card_id[]>());
        for(const auto card : cards.first(count))
        {
            GIVM_ASSERT(static_cast<bool>(table[card]));
            table[card].erase();
        }
        context.stack().push(hand_card_discard_progress{ .count = count });
        if(enter_hand_card_discard_effect(library, table, context, random))
            return continue_execution;
        return broadcast_hand_card_discard(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::discard_hand_card& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ prepare_hand_card_discard<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ prepare_hand_card_discard<false> });
        writer.write(execute_fn{ finish_hand_card_discard_effect });
        writer.write(execute_fn{ broadcast_hand_card_discard });
    }
}

#include "../../macro_undef.hpp"
#endif
