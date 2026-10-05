#ifndef GIVM_EXECUTOR_COMMANDS_DISCARD_DECK_CARDS_HPP
#define GIVM_EXECUTOR_COMMANDS_DISCARD_DECK_CARDS_HPP

#include "../program_writer.hpp"

#include <vector>

#include <algorithm>
#include <memory>

#include "../broadcast.hpp"
#include <givm/definition.hpp>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

namespace givm::detail
{
    inline void prepare_deck_card_discard_notification(
        const definition_library& library, const unrestricted_table& table,
        execution_context& context, deck_card_id card)
    {
        context.advance(response_extent<deck_card_discard_effect>);
        prepare_broadcast(library, deck_card_discarded{ card }, table, context.stack(), context.position());
    }

    inline void finish_deck_card_discard_effect_frame(
        const definition_library& library, const unrestricted_table& table, execution_context& context)
    {
        const auto card = get<0>(context.stack().top<deck_card_discard_effect, response_return>()).card;
        pop_single_response<deck_card_discard_effect, deck_card_id>(context);
        prepare_deck_card_discard_notification(library, table, context, card);
    }

    inline bool enter_deck_card_discard_effect(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        const auto [cards, cursor] = context.stack().top<deck_card_id[], stack_count_t>();
        const auto card = cards[cursor];
        const auto self = std::as_const(table)[card];
        const auto definition = library[self.definition_id()];
        if(not definition.can_handle<deck_card_discard_effect, deck_card_view>())
        {
            prepare_deck_card_discard_notification(library, table, context, card);
            return false;
        }
        prepare_single_response(deck_card_discard_effect{ card }, card, table, context, context.position(), true);
        if(not continue_single_response<deck_card_discard_effect, deck_card_id>(library, table, context, random)) return true;
        finish_deck_card_discard_effect_frame(library, table, context);
        return false;
    }

    inline execution_state broadcast_deck_card_discards(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        while(true)
        {
            if(not continue_broadcast<deck_card_discarded>(library, table, context, random))
                return continue_execution;
            pop_broadcast<deck_card_discarded>(context);
            auto&& [cards, cursor] = context.stack().top<deck_card_id[], stack_count_t>();
            if(++cursor == cards.size())
            {
                context.stack().pop<deck_card_id[], stack_count_t>();
                return context.advance(response_extent<deck_card_discarded>);
            }
            context.jump(context.position() - response_extent<deck_card_discard_effect>);
            if(enter_deck_card_discard_effect(library, table, context, random))
                return continue_execution;
        }
    }

    inline execution_state finish_deck_card_discard_effect(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_single_response<deck_card_discard_effect, deck_card_id>(library, table, context, random)) return continue_execution;
        finish_deck_card_discard_effect_frame(library, table, context);
        return broadcast_deck_card_discards(library, table, context, random);
    }

    inline execution_state resume_observed_deck_card_discard(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        context.enter_next();
        if(enter_deck_card_discard_effect(library, table, context, random))
            return continue_execution;
        return broadcast_deck_card_discards(library, table, context, random);
    }

    template<bool Fixed, bool Observed>
    inline execution_state prepare_deck_card_discards(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        player_id player;
        std::uint32_t requested_count;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, discard_deck_cards>(library);
#ifndef NDEBUG
            debug_validate_entity(table, table.state().self_player, "discard_deck_cards", "self_player");
#endif
            player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            requested_count = command.count;
            context.advance(instruction_extent<1, discard_deck_cards>);
        }
        else
        {
            const auto input = get<0>(context.stack().top<discard_deck_cards_input>());
            player = input.player;
            requested_count = input.count;
#ifndef NDEBUG
            debug_validate_entity(table, player, "discard_deck_cards", "player");
#endif
            context.stack().pop<discard_deck_cards_input>();
            context.enter_next();
        }
        auto player_entity = table[player];
        const auto count = std::min<size_t>(requested_count, player_entity.deck_card_count());
        if(count == 0)
            return context.advance(Observed * sizeof(execute_fn)
                + response_extent<deck_card_discard_effect> + response_extent<deck_card_discarded>);

        auto cards = get<0>(context.stack().push(dynamic_array<deck_card_id>(count), stack_count_t{ 0 }));
        for(auto& card : cards)
        {
            std::construct_at(&card, player_entity.deck_cards<false>().back().id());
            player_entity.discard_top_deck_card();
        }
        if constexpr(Observed)
            return execution_state::deck_cards_discarded;
        if(enter_deck_card_discard_effect(library, table, context, random))
            return continue_execution;
        return broadcast_deck_card_discards(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::discard_deck_cards& command, compile_mode mode)
    {
        const bool observed = mode == compile_mode::observed;
        if(command.count != std::numeric_limits<std::uint32_t>::max())
        {
            writer.write(observed ? execute_fn{ prepare_deck_card_discards<true, true> }
                : execute_fn{ prepare_deck_card_discards<true, false> });
            writer.write(command);
        }
        else
            writer.write(observed ? execute_fn{ prepare_deck_card_discards<false, true> }
                : execute_fn{ prepare_deck_card_discards<false, false> });
        if(observed)
            writer.write(execute_fn{ resume_observed_deck_card_discard });
        compile_single_response<deck_card_discard_effect, deck_card_id>(writer, finish_deck_card_discard_effect);
        compile_broadcast<deck_card_discarded>(writer, broadcast_deck_card_discards);
    }
}

namespace givm::detail
{
    inline std::vector<discard_deck_cards::error_type> check(const discard_deck_cards& command,
        const definition_compile_context&, program_kind kind)
    {
        using reason = discard_deck_cards::error_type::reason;
        std::vector<discard_deck_cards::error_type> errors;
        if(command.count == std::numeric_limits<std::uint32_t>::max())
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.player != relative_player::self && command.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_player, .value = static_cast<std::size_t>(command.player) });
        return errors;
    }
}


#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const discard_deck_cards& command) noexcept
    {
        return command.count == std::numeric_limits<std::uint32_t>::max() ? TInputTypes::template index_of<discard_deck_cards::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
