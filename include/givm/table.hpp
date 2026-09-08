#ifndef GIVM_TABLE_HPP
#define GIVM_TABLE_HPP

#include <functional>
#include <limits>
#include <optional>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "table/entity_id.hpp"
#include "table/table_accessor.hpp"
#include "table/entities/status_entity.hpp"
#include "table/entities/player_entity.hpp"
#include "definition.hpp"
#include "deck.hpp"
#include "table/table_storage.hpp"
#include "utils/debug.hpp"

#include "macro_define.hpp"

namespace givm
{
    class card_table
    {
        friend detail::table_accessor;

    public:
        using definition_library_type = definition_library;
        using game_state = table_state;

        constexpr card_table(
            const definition_library_type& definition_library,
            game_parameters parameters = {}
        )
        : storage_{
            .definition_library_ = &definition_library,
            .parameters = parameters
        }
        {}

        constexpr const definition_library_type& definition_library() const
        {
            return *storage_.definition_library_;
        }

        template<class Self>
        constexpr auto& parameters(this Self& self) noexcept
        {
            return detail::table_accessor::storage_of(self).parameters;
        }

        template<class Self>
        constexpr auto& state(this Self& self) noexcept
        {
            return detail::table_accessor::storage_of(self).state;
        }

        template<class Self>
        constexpr auto players(this Self& self)
        {
            auto& storage = detail::table_accessor::storage_of(self);
            using storage_type = std::remove_reference_t<decltype(storage)>;
            return storage.player_datas
                | std::views::transform([&](auto& data)
                {
                    auto result = detail::table_accessor::make_uninitialized<player_entity<storage_type>>();
                    detail::table_accessor::storage_of(result) = {
                        .table = &storage,
                        .data = &data
                    };
                    return result;
                });
        }

        constexpr void load_deck(player_id player, const linked_deck& deck)
        {
            auto& data = storage_.player_datas[player.index];
            GIVM_ASSERT(data.deck_card_datas.empty());
            GIVM_ASSERT(data.deck_card_order.empty());
            GIVM_ASSERT(data.character_datas.empty());

            data.deck_card_datas.reserve(deck.cards.size());
            data.deck_card_order.reserve(deck.cards.size());
            for(definition_id<card_definition> definition : deck.cards)
            {
                data.deck_card_datas.emplace_back(definition, card_state{});
                data.deck_card_order.push_back(data.deck_card_datas.size() - 1);
            }

            data.character_datas.reserve(deck.characters.size());
            for(definition_id<character_view> definition : deck.characters)
            {
                data.character_datas.emplace_back(definition, character_state{});
            }
        }

        template<class Self>
        constexpr auto operator[](this Self& self, player_id player_id)
        {
            auto& storage = detail::table_accessor::storage_of(self);
            using storage_type = std::remove_reference_t<decltype(storage)>;
            auto result = detail::table_accessor::make_uninitialized<player_entity<storage_type>>();
            detail::table_accessor::storage_of(result) = {
                .table = &storage,
                .data = &storage.player_datas[player_id.index]
            };
            return result;
        }

        template<class Self>
        constexpr auto operator[](this Self& self, support_id support_id)
        {
            const auto player = self[support_id.player_id];
            return player.template supports<false>()[support_id.index];
        }

        template<class Self>
        constexpr auto operator[](this Self& self, summon_id summon_id)
        {
            const auto player = self[summon_id.player_id];
            return player.template summons<false>()[summon_id.index];
        }

        template<class Self>
        constexpr auto operator[](this Self& self, combat_status_id combat_status_id)
        {
            const auto player = self[combat_status_id.player_id];
            return player.template combat_statuses<false>()[combat_status_id.index];
        }

        template<class Self>
        constexpr auto operator[](this Self& self, hand_card_id hand_card_id)
        {
            const auto player = self[hand_card_id.player_id];
            return player.template hand_cards<false>()[hand_card_id.index];
        }

        template<class Self>
        constexpr auto operator[](this Self& self, deck_card_id deck_card_id)
        {
            auto& storage = detail::table_accessor::storage_of(self);
            using storage_type = std::remove_reference_t<decltype(storage)>;
            auto& player = storage.player_datas[deck_card_id.player_id.index];
            auto result = detail::table_accessor::make_uninitialized<deck_card_entity<storage_type>>();
            detail::table_accessor::storage_of(result) = {
                .table = &storage,
                .player = &player,
                .slot = deck_card_id.index,
                .data = &player.deck_card_datas[deck_card_id.index]
            };
            return result;
        }

        template<class Self>
        constexpr auto operator[](this Self& self, hand_card_status_id status_id)
        {
            auto& storage = detail::table_accessor::storage_of(self);
            using storage_type = std::remove_reference_t<decltype(storage)>;
            auto result = detail::table_accessor::make_uninitialized<hand_card_status_entity<storage_type>>();
            detail::table_accessor::storage_of(result) = {
                .table = &storage,
                .owner = status_id.card_id,
                .slot = status_id.index,
                .data = &storage.status_slots[status_id.index]
            };
            return result;
        }

        template<class Self>
        constexpr auto operator[](this Self& self, deck_card_status_id status_id)
        {
            auto& storage = detail::table_accessor::storage_of(self);
            using storage_type = std::remove_reference_t<decltype(storage)>;
            auto result = detail::table_accessor::make_uninitialized<deck_card_status_entity<storage_type>>();
            detail::table_accessor::storage_of(result) = {
                .table = &storage,
                .owner = status_id.card_id,
                .slot = status_id.index,
                .data = &storage.status_slots[status_id.index]
            };
            return result;
        }

        template<class Self>
        constexpr auto operator[](this Self& self, character_id character_id)
        {
            const auto player = self[character_id.player_id];
            return player.template characters<false>()[character_id.index];
        }

        template<class Self>
        constexpr auto operator[](this Self& self, skill_id skill_id)
        {
            const auto character = self[skill_id.character_id];
            return character.template skills<false>()[skill_id.index];
        }

        template<class Self>
        constexpr auto operator[](this Self& self, attachment_id attachment_id)
        {
            const auto character = self[attachment_id.character_id];
            return character.template attachments<false>()[attachment_id.index];
        }

        // template<class Self>
        // constexpr auto operator[](this Self& self, summon_id summon_id)
        // {
        //     const auto player = self[summon_id.player_id];
        //     return SummonHandle<Self>{ player, summon_id.index };
        // }

        constexpr void clean_up() noexcept
        {
            for(auto player : players())
            {
                player.clean_up();
            }
            clean_up_statuses();
        }

    private:
        static constexpr size_t moved_status_mask =
            size_t{ 1 } << (std::numeric_limits<size_t>::digits - 1);

        constexpr void clean_up_statuses() noexcept
        {
            size_t front = 0;
            size_t back = storage_.status_slots.size();
            while(true)
            {
                while(front < back && storage_.status_slots[front].data.definition_id.is_valid())
                {
                    ++front;
                }
                while(front < back && not storage_.status_slots[back - 1].data.definition_id.is_valid())
                {
                    --back;
                }
                if(front == back)
                {
                    break;
                }

                const size_t source = --back;
                storage_.status_slots[front] = std::move(storage_.status_slots[source]);
                storage_.status_slots[source].data.definition_id.set_invalid();
                storage_.status_slots[source].next = moved_status_mask | front;
                ++front;
            }

            const size_t live_count = front;
            const auto relocated = [&](size_t index)
            {
                if(index == invalid_status_index || index < live_count)
                {
                    return index;
                }
                GIVM_ASSERT((storage_.status_slots[index].next & moved_status_mask) != 0);
                return storage_.status_slots[index].next & ~moved_status_mask;
            };

            for(size_t index = 0; index < live_count; ++index)
            {
                storage_.status_slots[index].next = relocated(storage_.status_slots[index].next);
            }
            for(auto& player : storage_.player_datas)
            {
                for(auto& card : player.hand_card_datas)
                {
                    card.first_status = relocated(card.first_status);
                    card.last_status = relocated(card.last_status);
                }
                for(auto& card : player.deck_card_datas)
                {
                    card.first_status = relocated(card.first_status);
                    card.last_status = relocated(card.last_status);
                }
            }
            storage_.status_slots.resize(live_count);
        }

        detail::table_storage storage_;
    };

}

#include "macro_undef.hpp"
#endif
