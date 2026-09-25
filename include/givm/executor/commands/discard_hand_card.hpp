#ifndef GIVM_EXECUTOR_COMMANDS_DISCARD_HAND_CARD_HPP
#define GIVM_EXECUTOR_COMMANDS_DISCARD_HAND_CARD_HPP

#include <algorithm>

#include "../broadcast.hpp"
#include "../../definition.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state broadcast_hand_card_discard(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<hand_card_discarded>(library, table, context, random))
            return continue_execution;
        pop_broadcast<hand_card_discarded>(context);
        return context.enter_next();
    }

    inline execution_state finish_hand_card_discard_effect(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        const auto card = get<0>(context.stack().top<hand_card_discard_effect, response_return>()).card;
        context.stack().pop<hand_card_discard_effect, response_return>();
        context.enter_next();
        prepare_broadcast(library, hand_card_discarded{ card }, table, context.stack(), context.position());
        return broadcast_hand_card_discard(library, table, context, random);
    }

    template<bool Fixed>
    inline execution_state prepare_hand_card_discard(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        hand_card_id card;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, discard_hand_card>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            auto cards = table[player].hand_cards();
            const auto found = std::ranges::find_if(cards,
                [&](const auto entity) { return entity.definition_id() == command.definition; });
            const bool exists = found != cards.end();
            GIVM_ASSERT(exists);
            [[assume(exists)]];
            card = (*found).id();
            context.advance(instruction_extent<1, discard_hand_card>);
        }
        else
        {
            card = get<0>(context.stack().top<discard_hand_card_input>()).card;
            context.stack().pop<discard_hand_card_input>();
            context.enter_next();
        }
        const bool valid = static_cast<bool>(table[card]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        table[card].erase();
        const auto self = std::as_const(table)[card];
        const auto definition = library[self.definition_id()];
        if(not definition.can_handle<hand_card_discard_effect, hand_card_view>())
        {
            context.enter_next();
            prepare_broadcast(library, hand_card_discarded{ card }, table, context.stack(), context.position());
            return broadcast_hand_card_discard(library, table, context, random);
        }
        context.stack().push(hand_card_discard_effect{ card }, response_return{ table.state().self_player, context.position() });
        auto& event = get<0>(context.stack().top<hand_card_discard_effect, response_return>());
        auto response = context.make_handle_context(table, random);
        const auto entry = definition.handle<hand_card_discard_effect>(self, event, response);
        if(entry)
        {
            table.state().self_player = self.player().id();
            return context.enter(entry);
        }
        return finish_hand_card_discard_effect(library, table, context, random);
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
