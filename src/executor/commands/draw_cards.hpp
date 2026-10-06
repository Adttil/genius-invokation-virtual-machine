#ifndef GIVM_EXECUTOR_COMMANDS_DRAW_CARDS_HPP
#define GIVM_EXECUTOR_COMMANDS_DRAW_CARDS_HPP

#include "../broadcast.hpp"
#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <givm/macro_define.hpp>

namespace givm::detail
{
    inline void add_drawn_card(execution_context& context, player_handle<unrestricted_table> player, card_data data)
    {
        const auto card = player.add_hand_card(std::move(data));
        const auto id = card.id();
        const bool overflow = player.hand_card_count() > player.state().hand_limit;
        if(overflow) card.erase();
        record_hand_entry(context, id, hand_entry_kind::drawn, overflow);
    }

    inline execution_state draw_fixed_cards_execute(const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        const auto command = context.instruction_data<1, draw_cards>(library);
        const auto player = table[command.player == relative_player::self
            ? table.state().self_player : other_player(table.state().self_player)];
        context.advance(instruction_extent<1, draw_cards>);
        if(command.position == 0)
        {
            const auto count = std::min(command.count, player.deck_card_count());
            for(std::size_t index = 0; index != count; ++index)
                add_drawn_card(context, player, player.take_top_deck_card());
        }
        else for(std::size_t index = 0; index != command.count; ++index)
        {
            const auto deck = player.template deck_cards<false>();
            if(command.position >= deck.size()) break;
            const auto card = deck[deck.size() - 1 - command.position].id();
            add_drawn_card(context, player, player.take_deck_card(card));
            player.compact_deck_card_order();
        }
        return continue_execution;
    }

    inline execution_state draw_selected_cards_execute(const definition_library&, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        const auto cards = get<0>(context.stack().top<deck_card_id[]>());
#ifndef NDEBUG
        debug_validate_unique(std::span<const deck_card_id>{ cards }, "draw_cards", "cards");
        for(const auto card : cards) debug_validate_entity(table, card, "draw_cards", "cards");
#endif
        std::array<bool, 2> changed{};
        for(const auto card : cards)
        {
            const auto player = table[card.player_id];
            add_drawn_card(context, player, player.take_deck_card(card));
            changed[card.player_id.index] = true;
        }
        for(std::size_t index = 0; index != changed.size(); ++index)
            if(changed[index]) table[player_id{ index }].compact_deck_card_order();
        context.stack().pop<deck_card_id[]>();
        return context.enter_next();
    }

    inline void compile(program_writer& writer, const draw_cards& command, compile_mode)
    {
        if(command.position == std::numeric_limits<std::size_t>::max())
            writer.write(execute_fn{ draw_selected_cards_execute });
        else
        {
            writer.write(execute_fn{ draw_fixed_cards_execute });
            writer.write(command);
        }
    }
}

namespace givm::detail
{
    inline std::vector<draw_cards::error_type> check(const draw_cards& command, const definition_compile_context&, program_kind kind)
    {
        using reason = draw_cards::error_type::reason;
        std::vector<draw_cards::error_type> errors;
        if(command.position == std::numeric_limits<std::size_t>::max())
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

#include <givm/macro_undef.hpp>

namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const draw_cards& command) noexcept
    {
        return command.position == std::numeric_limits<std::size_t>::max() ? TInputTypes::template index_of<draw_cards::input_type>() : std::size_t(-1);
    }
}

#endif
