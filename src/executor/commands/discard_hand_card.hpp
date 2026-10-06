#ifndef GIVM_EXECUTOR_COMMANDS_DISCARD_HAND_CARD_HPP
#define GIVM_EXECUTOR_COMMANDS_DISCARD_HAND_CARD_HPP

#include "../program_writer.hpp"

#include <vector>

#include <algorithm>
#include <memory>

#include "../broadcast.hpp"
#include <givm/definition.hpp>
#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <givm/macro_define.hpp>

namespace givm::detail
{
    template<bool Fixed>
    inline execution_state prepare_hand_card_discard(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        std::size_t count;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, discard_hand_card>(library);
#ifndef NDEBUG
            debug_validate_entity(table, table.state().self_player, "discard_hand_card", "self_player");
#endif
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
#ifndef NDEBUG
            const std::span<const hand_card_id> cards = get<0>(context.stack().top<hand_card_id[]>());
            debug_validate_unique(cards, "discard_hand_card", "cards");
            for(const auto card : cards) debug_validate_entity(table, card, "discard_hand_card", "cards");
#endif
            context.enter_next();
        }
        const auto cards = get<0>(context.stack().top<hand_card_id[]>());
        for(const auto card : cards.first(count))
        {
            table[card].erase();
            append_removal_record<this_hand_card_discard>(context, card, hand_card_discarded{ card });
        }
        context.stack().pop<hand_card_id[]>();
        return continue_execution;
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
    }
}

namespace givm::detail
{
    inline std::vector<discard_hand_card::error_type> check(const discard_hand_card& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = discard_hand_card::error_type::reason;
        std::vector<discard_hand_card::error_type> errors;
        if(not command.definition)
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.player != relative_player::self && command.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_player, .value = static_cast<std::size_t>(command.player) });
        if(command.definition.value() >= context.definition_count<card_definition>())
            errors.push_back({ .cause = reason::invalid_definition, .value = command.definition.value(), .limit = context.definition_count<card_definition>() });
        return errors;
    }
}

#include <givm/macro_undef.hpp>

namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const discard_hand_card& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<discard_hand_card::input_type>() : std::size_t(-1);
    }
}

#endif
