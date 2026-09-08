#ifndef GIVM_EXECUTOR_INSTRUCTIONS_REPLACE_CARDS_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_REPLACE_CARDS_HPP

#include "../executor.hpp"

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <functional>
#include <span>
#include <utility>
#include <vector>

#include "../broadcast.hpp"
#include "../events.hpp"
#include "../selector.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    namespace detail
    {
        template<class TNextRandom, class TOnDrawn>
        inline void replace_cards(
            card_table& table,
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
                    blacklist.push_back(card.definition().id().value());
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

    }

    struct replace_cards
    {
        using context_type = void;

        enum class stage_type : stage_t
        {
            prepare_selection,
            wait_selection,
            prepare_card_drawn,
            broadcast_card_drawn
        };

        player_id player;

        bool execute(card_table& table, execution_context& context, random_fn& random) const
        {
            const auto stage = static_cast<stage_type>(context.current_stage());
            if(stage == stage_type::prepare_selection)
            {
                context.stack().push(
                    selector{ .player = player },
                    static_cast<stage_t>(stage_type::wait_selection)
                );
                return context.yield();
            }

            if(stage == stage_type::prepare_card_drawn)
            {
                auto&& [drawn_cards, cursor, stored_stage] =
                    context.stack().top<hand_card_id[], stack_count_t, stage_t>();
                (void)stored_stage;
                if(cursor == drawn_cards.size())
                {
                    context.stack().pop<hand_card_id[], stack_count_t, stage_t>();
                    return context.enter_next();
                }

                detail::prepare_broadcast(card_drawn{ .card = drawn_cards[cursor++] }, table, context.stack());
                context.current_stage() = static_cast<stage_t>(stage_type::broadcast_card_drawn);
                return true;
            }

            if(stage == stage_type::broadcast_card_drawn)
            {
                if(not detail::continue_broadcast<card_drawn>(table, context, random))
                {
                    return true;
                }

                detail::pop_broadcast<card_drawn>(context);
                return true;
            }

            auto&& [input, stored_stage] = context.stack().top<selector, stage_t>();
            (void)stored_stage;
            GIVM_ASSERT(input.player == player);

            const auto selected = input.selected;
            auto next_random = [&random]{ return random(); };
            std::vector<hand_card_id> drawn_cards;
            drawn_cards.reserve(selected.count());
            auto on_drawn = [&](hand_card_id card)
            {
                drawn_cards.push_back(card);
            };
            detail::replace_cards(table, player, selected, next_random, on_drawn);
            context.stack().pop<selector, stage_t>();
            if(drawn_cards.empty())
            {
                return context.enter_next();
            }

            context.stack().push(
                dynamic_array<hand_card_id>(drawn_cards),
                stack_count_t{},
                static_cast<stage_t>(stage_type::prepare_card_drawn)
            );
            return true;
        }
    };
}

#include "../../macro_undef.hpp"
#endif
