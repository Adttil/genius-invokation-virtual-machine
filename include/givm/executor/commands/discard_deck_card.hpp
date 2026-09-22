#ifndef GIVM_EXECUTOR_COMMANDS_DISCARD_DECK_CARD_HPP
#define GIVM_EXECUTOR_COMMANDS_DISCARD_DECK_CARD_HPP

#include "../broadcast.hpp"
#include "../../definition.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state broadcast_deck_card_discard(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<deck_card_discarded>(library, table, context, random))
            return continue_execution;
        pop_broadcast<deck_card_discarded>(context);
        return context.enter_next();
    }

    inline execution_state finish_deck_card_discard_effect(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        const auto event = get<0>(context.stack().top<deck_card_discarded, execution_position>());
        context.stack().pop<deck_card_discarded, execution_position>();
        context.enter_next();
        prepare_broadcast(library, event, table, context.stack(), context.position());
        return broadcast_deck_card_discard(library, table, context, random);
    }

    template<bool Fixed>
    inline execution_state prepare_deck_card_discard(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        deck_card_id card;
        if constexpr(Fixed)
        {
            card = context.instruction_data<1, discard_deck_card>(library).card;
            context.advance(instruction_extent<1, discard_deck_card>);
        }
        else
        {
            card = get<0>(context.stack().top<deck_card_discarded>()).card;
            context.stack().pop<deck_card_discarded>();
            context.enter_next();
        }
        const bool valid = static_cast<bool>(table[card]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        table[card].erase();
        const auto self = std::as_const(table)[card];
        const auto definition = library[self.definition_id()];
        if(not definition.can_handle<deck_card_discarded, deck_card_view>())
        {
            context.enter_next();
            prepare_broadcast(library, deck_card_discarded{ card }, table, context.stack(), context.position());
            return broadcast_deck_card_discard(library, table, context, random);
        }
        context.stack().push(deck_card_discarded{ card }, context.position());
        auto& event = get<0>(context.stack().top<deck_card_discarded, execution_position>());
        auto response = context.make_handle_context(table, random);
        const auto entry = definition.handle<deck_card_discarded>(self, event, response);
        if(entry) return context.enter(entry);
        return finish_deck_card_discard_effect(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::discard_deck_card& command, compile_mode)
    {
        if(command.card.index != std::numeric_limits<size_t>::max())
        {
            writer.write(execute_fn{ prepare_deck_card_discard<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ prepare_deck_card_discard<false> });
        writer.write(execute_fn{ finish_deck_card_discard_effect });
        writer.write(execute_fn{ broadcast_deck_card_discard });
    }
}

#include "../../macro_undef.hpp"
#endif
