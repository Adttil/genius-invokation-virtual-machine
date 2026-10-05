#ifndef GIVM_EXECUTOR_COMMANDS_DRAW_CARDS_HPP
#define GIVM_EXECUTOR_COMMANDS_DRAW_CARDS_HPP

#include "../program_writer.hpp"

#include <vector>

#include <givm/executor/executor.hpp>
#include <givm/definition.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <span>

#include "../broadcast.hpp"
#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <givm/macro_define.hpp>

namespace givm::detail
{
    // A batch has a private [hand_card_id[], stack_count_t] frame and one
    // active broadcast. This continuation does not depend on adjacent opcodes.
    template<bool Inputs = false>
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
                if constexpr(Inputs) context.stack().pop<deck_card_id[]>();
                return context.advance(response_extent<card_drawn>);
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

    struct fixed_draw_cards_parameters
    {
        relative_player player;
        std::size_t count;
    };

    inline execution_state draw_top_cards_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        const auto& command = context.instruction_data<1, fixed_draw_cards_parameters>(library);
#ifndef NDEBUG
        debug_validate_entity(table, table.state().self_player, "draw_cards", "self_player");
#endif
        const auto target_player = command.player == relative_player::self
            ? table.state().self_player
            : other_player(table.state().self_player);
        auto player_entity = table[target_player];
        const auto count = std::min<size_t>(command.count, player_entity.deck_card_count());
        context.advance(instruction_extent<1, fixed_draw_cards_parameters>);
        if(count == 0)
        {
            return context.advance(response_extent<card_drawn>);
        }

        const auto hand_count = player_entity.hand_card_count();
        const auto hand_limit = player_entity.state().hand_limit;
        const auto drawn_count = hand_count < hand_limit
            ? std::min<size_t>(count, hand_limit - hand_count) : size_t{ 0 };

        // No responses run until the entire batch, including overflow removals, is complete.
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
            return context.advance(response_extent<card_drawn>);
        }

        prepare_drawn_cards(library, table, context, context.position());
        return broadcast_drawn_card(library, table, context, random);
    }

    inline execution_state draw_positioned_cards_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        const auto& command = context.instruction_data<1, fixed_draw_cards_parameters>(library);
#ifndef NDEBUG
        debug_validate_entity(table, table.state().self_player, "draw_cards", "self_player");
#endif
        const auto target_player = command.player == relative_player::self
            ? table.state().self_player : other_player(table.state().self_player);
        context.advance(instruction_extent<1, fixed_draw_cards_parameters>);
        static_assert(padded_size<std::size_t> == sizeof(std::size_t));
        const std::span positions{ &context.instruction_data<0, std::size_t>(library), command.count };
        context.advance(positions.size() * padded_size<std::size_t>);
        auto player_entity = table[target_player];
        const auto deck_count = player_entity.deck_card_count();
        const auto count = static_cast<std::size_t>(std::ranges::count_if(
            positions, [deck_count](std::size_t position) { return position < deck_count; }));
        if(count == 0) return context.advance(response_extent<card_drawn>);

        const auto hand_count = player_entity.hand_card_count();
        const auto hand_limit = player_entity.state().hand_limit;
        const auto drawn_count = hand_count < hand_limit
            ? std::min<std::size_t>(count, hand_limit - hand_count) : std::size_t{ 0 };
        std::span<hand_card_id> drawn_cards;
        if(drawn_count != 0)
            drawn_cards = get<0>(context.stack().push(dynamic_array<hand_card_id>(drawn_count), stack_count_t{ 1 }));

        std::size_t drawn = 0;
        for(const auto position : positions)
        {
            if(position >= deck_count) continue;
            const auto card = player_entity.deck_cards<false>()[deck_count - 1 - position].id();
            if(drawn < drawn_count)
            {
                auto data = player_entity.take_deck_card(card);
                std::construct_at(&drawn_cards[drawn++], player_entity.add_hand_card(std::move(data)).id());
            }
            else
                player_entity.discard_deck_card(card);
        }
        player_entity.compact_deck_card_order();
        if(drawn_count == 0) return context.advance(response_extent<card_drawn>);
        prepare_drawn_cards(library, table, context, context.position());
        return broadcast_drawn_card(library, table, context, random);
    }

    inline execution_state draw_selected_cards_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        std::span<const deck_card_id> cards = get<0>(context.stack().top<deck_card_id[]>());
#ifndef NDEBUG
        debug_validate_unique(cards, "draw_cards", "cards");
        for(const auto card : cards) debug_validate_entity(table, card, "draw_cards", "cards");
#endif
        context.enter_next();
        if(cards.empty())
        {
            context.stack().pop<deck_card_id[]>();
            return context.advance(response_extent<card_drawn>);
        }

        std::array<std::size_t, 2> counts{};
        for(const auto card : cards)
        {
            GIVM_ASSERT(card.player_id.index < counts.size());
            ++counts[card.player_id.index];
        }
        std::array<std::size_t, 2> remaining{};
        for(std::size_t index = 0; index < counts.size(); ++index)
        {
            if(counts[index] == 0) continue;
            const auto player = table[player_id{ index }];
            const auto hand_count = player.hand_card_count();
            const auto hand_limit = player.state().hand_limit;
            if(hand_count < hand_limit)
                remaining[index] = std::min<std::size_t>(counts[index], hand_limit - hand_count);
        }
        const auto drawn_count = remaining[0] + remaining[1];
        std::span<hand_card_id> drawn_cards;
        if(drawn_count != 0)
        {
            drawn_cards = get<0>(context.stack().push(dynamic_array<hand_card_id>(drawn_count), stack_count_t{ 1 }));
            const auto frames = context.stack().top<frame<deck_card_id[]>, frame<hand_card_id[], stack_count_t>>();
            cards = get<0>(get<0>(frames));
        }

        std::size_t drawn = 0;
        for(const auto card : cards)
        {
            auto player = table[card.player_id];
            auto& available = remaining[card.player_id.index];
            if(available != 0)
            {
                auto data = player.take_deck_card(card);
                std::construct_at(&drawn_cards[drawn++], player.add_hand_card(std::move(data)).id());
                --available;
            }
            else
                player.discard_deck_card(card);
        }
        for(std::size_t index = 0; index < counts.size(); ++index)
            if(counts[index] != 0) table[player_id{ index }].compact_deck_card_order();

        if(drawn_count == 0)
        {
            context.stack().pop<deck_card_id[]>();
            return context.advance(response_extent<card_drawn>);
        }
        prepare_drawn_cards(library, table, context, context.position());
        return broadcast_drawn_card<true>(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::draw_cards& command, compile_mode)
    {
        if(command.positions.empty())
        {
            writer.write(execute_fn{ draw_selected_cards_execute });
            compile_broadcast<card_drawn>(writer, broadcast_drawn_card<true>);
            return;
        }

        bool from_top = true;
        for(std::size_t index = 0; index < command.positions.size(); ++index)
        {
            GIVM_ASSERT(std::ranges::find(command.positions.first(index), command.positions[index])
                == command.positions.first(index).end());
            from_top = from_top && command.positions[index] == index;
        }
        writer.write(execute_fn{ from_top ? draw_top_cards_execute : draw_positioned_cards_execute });
        writer.write(fixed_draw_cards_parameters{ command.player, command.positions.size() });
        if(not from_top)
            for(const auto position : command.positions) writer.write(position);
        compile_broadcast<card_drawn>(writer, broadcast_drawn_card<false>);
    }
}

namespace givm::detail
{
    inline std::vector<draw_cards::error_type> check(const draw_cards& command, const definition_compile_context&, program_kind kind)
    {
        using reason = draw_cards::error_type::reason;
        std::vector<draw_cards::error_type> errors;
        if(command.positions.empty())
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.player != relative_player::self && command.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_player, .value = static_cast<std::size_t>(command.player) });
        for(std::size_t index = 0; index < command.positions.size(); ++index)
        {
            const auto earlier = std::ranges::find(command.positions.first(index), command.positions[index]);
            if(earlier != command.positions.first(index).end())
                errors.push_back({ .cause = reason::duplicate_position, .value = command.positions[index], .index = index, .first_index = static_cast<std::size_t>(earlier - command.positions.first(index).begin()) });
        }
        return errors;
    }
}

#include <givm/macro_undef.hpp>

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const draw_cards& command) noexcept
    {
        return command.positions.empty() ? TInputTypes::template index_of<draw_cards::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
