#ifndef GIVM_EXECUTOR_INSTRUCTIONS_REPLACE_CARDS_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_REPLACE_CARDS_HPP

#include "../executor.hpp"
#include "../../definition/commands.hpp"

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <functional>
#include <span>
#include <utility>
#include <vector>

#include "draw_cards.hpp"
#include "../selector.hpp"

#include "../../macro_define.hpp"

namespace givm::detail
{
    template<class TNextRandom, class TOnDrawn>
    inline void replace_cards(
        unrestricted_table& table,
        player_id player,
        std::bitset<selection_capacity> selected,
        TNextRandom& next_random,
        TOnDrawn& on_drawn
    )
    {
        auto player_entity = table[player];
        std::vector<hand_card_id> selected_cards;
        std::vector<size_t> blacklist;
        size_t hand_index = 0;
        for(auto card : player_entity.hand_cards<false>())
        {
            if(not card)
            {
                continue;
            }

            if(selected[hand_index])
            {
                selected_cards.push_back(card.id());
                blacklist.push_back(card.definition_id().value());
            }
            ++hand_index;
        }
        std::ranges::sort(blacklist);
        const auto unique_end = std::ranges::unique(blacklist).begin();
        blacklist.erase(unique_end, blacklist.end());
        const auto is_blacklisted = [&](definition_id<card_definition> definition)
        { return std::ranges::binary_search(blacklist, definition.value()); };

        for(hand_card_id card_id : selected_cards)
        {
            auto card = player_entity.take_hand_card(card_id);
            const auto deck_count = player_entity.deck_card_count();
            const auto insertion_index = static_cast<size_t>(
                static_cast<std::uint64_t>(next_random()) * (deck_count + 1) >> 32
            );
            player_entity.insert_deck_card(insertion_index, std::move(card));
        }

        const size_t selected_count = selected_cards.size();
        if(selected_count == 0)
        {
            return;
        }

        std::vector<size_t> non_blacklisted_indices;
        std::vector<size_t> blacklisted_indices;
        non_blacklisted_indices.reserve(selected_count);
        blacklisted_indices.reserve(selected_count);

        bool has_enough_non_blacklisted = false;
        for(size_t index = player_entity.deck_card_count(); index > 0;)
        {
            --index;
            if(is_blacklisted(player_entity.deck_card_definition(index)))
            {
                if(blacklisted_indices.size() < selected_count)
                {
                    blacklisted_indices.push_back(index);
                }
                continue;
            }

            non_blacklisted_indices.push_back(index);
            if(non_blacklisted_indices.size() == selected_count)
            {
                has_enough_non_blacklisted = true;
                break;
            }
        }

        std::vector<size_t> drawn_indices = std::move(non_blacklisted_indices);
        if(not has_enough_non_blacklisted)
        {
            const size_t blacklisted_count = selected_count - drawn_indices.size();
            drawn_indices.insert(drawn_indices.end(), blacklisted_indices.begin(),
                                 blacklisted_indices.begin() + blacklisted_count);
            std::ranges::sort(drawn_indices, std::greater{});
        }

        auto drawn_card_datas = player_entity.take_deck_cards(drawn_indices);
        for(auto& card : drawn_card_datas)
        {
            std::invoke(on_drawn, player_entity.add_hand_card(std::move(card)).id());
        }
    }

    inline execution_state prepare_card_selection(
        const definition_library& library, unrestricted_table&,
        execution_context& context, random_fn&
    )
    {
        const auto& command = context.instruction_data<1, givm::replace_cards>(library);
        context.stack().push(selector{ .player = command.player });
        context.advance(instruction_extent<1, givm::replace_cards>);
        return context.yield(execution_state::card_selection);
    }

    inline execution_state apply_card_selection(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        const auto& input = get<0>(context.stack().top<selector>());
        const auto selected = input.selected;
        const auto player = input.player;
        auto next_random = [&random]{ return random(); };
        std::vector<hand_card_id> drawn_cards;
        drawn_cards.reserve(selected.count());
        auto on_drawn = [&](hand_card_id card)
        {
            drawn_cards.push_back(card);
        };
        detail::replace_cards(table, player, selected, next_random, on_drawn);
        context.stack().pop<selector>();
        if(drawn_cards.empty())
        {
            return context.advance(2 * sizeof(execute_fn));
        }

        prepare_drawn_cards(library, table, context, drawn_cards);
        return context.enter_next();
    }

    inline void compile(program_writer& writer, const givm::replace_cards& command, compile_mode)
    {
        writer.write(execute_fn{ &prepare_card_selection });
        writer.write(command);
        writer.write(execute_fn{ &apply_card_selection });
        writer.write(execute_fn{ &broadcast_drawn_card });
    }
}

#include "../../macro_undef.hpp"
#endif
