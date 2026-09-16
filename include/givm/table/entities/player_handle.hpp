#ifndef GIVM_TABLE_ENTITIES_PLAYER_HANDLE_HPP
#define GIVM_TABLE_ENTITIES_PLAYER_HANDLE_HPP

#include <algorithm>
#include <functional>
#include <ranges>
#include <optional>
#include <span>
#include <type_traits>
#include <vector>

#include "../../utils/debug.hpp"
#include "../../enums/elemental_dice.hpp"
#include "../entity_storage.hpp"
#include "../table_accessor.hpp"
#include "deck_card_handle.hpp"
#include "hand_card_handle.hpp"
#include "summon_handle.hpp"
#include "support_handle.hpp"
#include "combat_status_handle.hpp"
#include "character_handle.hpp"

#include "../../macro_define.hpp"

namespace givm::detail
{
    template<class TStorage>
    class basic_player_handle
    {
        friend detail::table_accessor;
        friend class ::givm::player_view;

    public:
        static constexpr bool is_mutable = not std::is_const_v<TStorage>;
        using table_type = TStorage;
        using data_type = maybe_mutable<is_mutable, player_data>;
        using storage_type = detail::player_entity_storage<TStorage>;

        constexpr operator player_handle<const TStorage>() const noexcept requires is_mutable
        {
            auto result = detail::table_accessor::make_uninitialized<player_handle<const TStorage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .data = storage_.data
            };
            return result;
        }

        constexpr player_id id() const
        {
            return { static_cast<size_t>(storage_.data - storage_.table->player_datas) };
        }

        constexpr auto& state() const
        {
            return storage_.data->state;
        }

        template<bool SkipErased = true>
        constexpr auto hand_cards() const
        {
            if constexpr(SkipErased)
            {
                return hand_cards<false>() | std::views::filter([](auto&& card){ return card.is_valid(); });
            }
            else
            {
                return storage_.data->hand_card_datas
                    | std::views::transform([player = *this](auto& data)
                    {
                        auto result = detail::table_accessor::make_uninitialized<hand_card_handle<TStorage>>();
                        detail::table_accessor::storage_of(result) = {
                            .table = player.storage_.table,
                            .player = player.storage_.data,
                            .data = &data
                        };
                        return result;
                    });
            }
        }

        template<bool SkipErased = true>
        constexpr auto deck_cards() const
        {
            if constexpr(SkipErased)
            {
                return deck_cards<false>() | std::views::filter([](auto&& card){ return card.is_valid(); });
            }
            else
            {
                return storage_.data->deck_card_order
                    | std::views::transform([player = *this](size_t index)
                    {
                        auto result = detail::table_accessor::make_uninitialized<deck_card_handle<TStorage>>();
                        detail::table_accessor::storage_of(result) = {
                            .table = player.storage_.table,
                            .player = player.storage_.data,
                            .slot = index,
                            .data = &player.storage_.data->deck_card_datas[index]
                        };
                        return result;
                    });
            }
        }

        template<bool SkipErased = true>
        constexpr auto supports() const
        {
            if constexpr(SkipErased)
            {
                return supports<false>() | std::views::filter([](auto&& support){ return support.is_valid(); });
            }
            else
            {
                return storage_.data->support_datas
                    | std::views::transform([player = *this](auto& data)
                    {
                        auto result = detail::table_accessor::make_uninitialized<support_handle<TStorage>>();
                        detail::table_accessor::storage_of(result) = {
                            .table = player.storage_.table,
                            .player = player.storage_.data,
                            .data = &data
                        };
                        return result;
                    });
            }
        }

        template<bool SkipErased = true>
        constexpr auto summons() const
        {
            if constexpr(SkipErased)
            {
                return summons<false>() | std::views::filter([](auto&& support){ return support.is_valid(); });
            }
            else
            {
                return storage_.data->summon_datas
                    | std::views::transform([player = *this](auto& data)
                    {
                        auto result = detail::table_accessor::make_uninitialized<summon_handle<TStorage>>();
                        detail::table_accessor::storage_of(result) = {
                            .table = player.storage_.table,
                            .player = player.storage_.data,
                            .data = &data
                        };
                        return result;
                    });
            }
        }

        template<bool SkipErased = true>
        constexpr auto combat_statuses() const
        {
            if constexpr(SkipErased)
            {
                return combat_statuses<false>() | std::views::filter([](auto&& support){ return support.is_valid(); });
            }
            else
            {
                return storage_.data->combat_status_datas
                    | std::views::transform([player = *this](auto& data)
                    {
                        auto result = detail::table_accessor::make_uninitialized<combat_status_handle<TStorage>>();
                        detail::table_accessor::storage_of(result) = {
                            .table = player.storage_.table,
                            .player = player.storage_.data,
                            .data = &data
                        };
                        return result;
                    });
            }
        }

        template<bool SkipErased = true>
        constexpr auto characters() const
        {
            if constexpr(SkipErased)
            {
                return characters<false>() | std::views::filter([](auto&& support){ return support.is_valid(); });
            }
            else
            {
                return storage_.data->character_datas
                    | std::views::transform([player = *this](auto& data)
                    {
                        auto result = detail::table_accessor::make_uninitialized<character_handle<TStorage>>();
                        detail::table_accessor::storage_of(result) = {
                            .table = player.storage_.table,
                            .player = player.storage_.data,
                            .data = &data
                        };
                        return result;
                    });
            }
        }

        constexpr hand_card_handle<TStorage> add_hand_card(definition_id<card_definition> definition_id, const card_state& state) const requires is_mutable
        {
            storage_.data->hand_card_datas.emplace_back(definition_id, state);
            auto result = detail::table_accessor::make_uninitialized<hand_card_handle<TStorage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = storage_.data,
                .data = &storage_.data->hand_card_datas.back()
            };
            return result;
        }

        constexpr deck_card_handle<TStorage> add_deck_card(definition_id<card_definition> definition_id, const card_state& state) const requires is_mutable
        {
            storage_.data->deck_card_datas.emplace_back(definition_id, state);
            storage_.data->deck_card_order.push_back(storage_.data->deck_card_datas.size() - 1);
            return deck_card({ id(), storage_.data->deck_card_datas.size() - 1 });
        }

        constexpr deck_card_handle<TStorage> insert_deck_card(
            size_t index,
            definition_id<card_definition> definition_id,
            const card_state& state
        ) const requires is_mutable
        {
            GIVM_ASSERT(index <= storage_.data->deck_card_order.size());
            storage_.data->deck_card_datas.emplace_back(definition_id, state);
            const size_t slot = storage_.data->deck_card_datas.size() - 1;
            storage_.data->deck_card_order.insert(storage_.data->deck_card_order.begin() + index, slot);
            return deck_card({ id(), slot });
        }

        constexpr deck_card_handle<TStorage> insert_deck_card(
            size_t index,
            card_data data
        ) const requires is_mutable
        {
            GIVM_ASSERT(index <= storage_.data->deck_card_order.size());
            storage_.data->deck_card_datas.push_back(std::move(data));
            const size_t slot = storage_.data->deck_card_datas.size() - 1;
            storage_.data->deck_card_order.insert(storage_.data->deck_card_order.begin() + index, slot);
            return deck_card({ id(), slot });
        }

        constexpr size_t deck_card_count() const noexcept
        {
            return storage_.data->deck_card_order.size();
        }

        constexpr void swap_deck_cards(size_t first, size_t second) const requires is_mutable
        {
            GIVM_ASSERT(first < storage_.data->deck_card_order.size());
            GIVM_ASSERT(second < storage_.data->deck_card_order.size());
            std::ranges::swap(
                storage_.data->deck_card_order[first],
                storage_.data->deck_card_order[second]
            );
        }

        constexpr size_t hand_card_count() const noexcept
        {
            return static_cast<size_t>(std::ranges::count_if(
                storage_.data->hand_card_datas,
                [](const card_data& data){ return data.definition_id.is_valid(); }
            ));
        }

        constexpr definition_id<card_definition> deck_card_definition(size_t index) const
        {
            GIVM_ASSERT(index < storage_.data->deck_card_order.size());
            return storage_.data->deck_card_datas[storage_.data->deck_card_order[index]].definition_id;
        }

        constexpr card_data take_top_deck_card() const requires is_mutable
        {
            GIVM_ASSERT(not storage_.data->deck_card_order.empty());
            const size_t slot = storage_.data->deck_card_order.back();
            storage_.data->deck_card_order.pop_back();
            auto& source = storage_.data->deck_card_datas[slot];
            card_data result = std::move(source);
            source.definition_id.set_invalid();
            source.first_status = invalid_status_index;
            source.last_status = invalid_status_index;
            return result;
        }

        template<class TConsume>
        constexpr void take_deck_cards(
            std::span<const size_t> descending_indices, TConsume&& consume
        ) const requires is_mutable
        {
            if(descending_indices.empty())
            {
                return;
            }

            // The internal consumer must not alter the deck or run event responses.
            // All removals finish before the caller broadcasts any resulting events.
            size_t previous_index = storage_.data->deck_card_order.size();
            for(size_t index : descending_indices)
            {
                GIVM_ASSERT(index < previous_index);
                auto& source = storage_.data->deck_card_datas[storage_.data->deck_card_order[index]];
                std::invoke(consume, std::move(source));
                source.definition_id.set_invalid();
                source.first_status = invalid_status_index;
                source.last_status = invalid_status_index;
                previous_index = index;
            }

            auto selected = descending_indices.rbegin();
            size_t write_index = descending_indices.back();
            for(size_t read_index = write_index; read_index < storage_.data->deck_card_order.size(); ++read_index)
            {
                if(selected != descending_indices.rend() && *selected == read_index)
                {
                    ++selected;
                    continue;
                }

                if(write_index != read_index)
                {
                    storage_.data->deck_card_order[write_index] = storage_.data->deck_card_order[read_index];
                }
                ++write_index;
            }
            storage_.data->deck_card_order.resize(write_index);
        }

        constexpr card_data take_hand_card(hand_card_id card_id) const requires is_mutable
        {
            GIVM_ASSERT(card_id.player_id == id());
            GIVM_ASSERT(card_id.index < storage_.data->hand_card_datas.size());
            auto& source = storage_.data->hand_card_datas[card_id.index];
            GIVM_ASSERT(source.definition_id.is_valid());
            card_data result = std::move(source);
            source.definition_id.set_invalid();
            source.first_status = invalid_status_index;
            source.last_status = invalid_status_index;
            return result;
        }

        constexpr void discard_top_deck_card() const requires is_mutable
        {
            GIVM_ASSERT(not storage_.data->deck_card_order.empty());
            const size_t slot = storage_.data->deck_card_order.back();
            storage_.data->deck_card_order.pop_back();
            auto& source = storage_.data->deck_card_datas[slot];
            detail::erase_statuses(*storage_.table, source);
            source.definition_id.set_invalid();
            source.first_status = invalid_status_index;
            source.last_status = invalid_status_index;
        }

        constexpr hand_card_handle<TStorage> add_hand_card(card_data data) const requires is_mutable
        {
            storage_.data->hand_card_datas.push_back(std::move(data));
            auto result = detail::table_accessor::make_uninitialized<hand_card_handle<TStorage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = storage_.data,
                .data = &storage_.data->hand_card_datas.back()
            };
            return result;
        }

        constexpr support_handle<TStorage> add(definition_id<support_view> definition_id, const support_state& state) const requires is_mutable
        {
            storage_.data->support_datas.emplace_back(definition_id, state);
            auto result = detail::table_accessor::make_uninitialized<support_handle<TStorage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = storage_.data,
                .data = &storage_.data->support_datas.back()
            };
            return result;
        }

        constexpr summon_handle<TStorage> add(definition_id<summon_view> definition_id, const summon_state& state) const requires is_mutable
        {
            storage_.data->summon_datas.emplace_back(definition_id, state);
            auto result = detail::table_accessor::make_uninitialized<summon_handle<TStorage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = storage_.data,
                .data = &storage_.data->summon_datas.back()
            };
            return result;
        }

        constexpr combat_status_handle<TStorage> add(definition_id<combat_status_view> definition_id, const combat_status_state& state) const requires is_mutable
        {
            storage_.data->combat_status_datas.emplace_back(definition_id, state);
            auto result = detail::table_accessor::make_uninitialized<combat_status_handle<TStorage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = storage_.data,
                .data = &storage_.data->combat_status_datas.back()
            };
            return result;
        }

        constexpr character_handle<TStorage> add(definition_id<character_view> definition_id, const character_state& state) const requires is_mutable
        {
            storage_.data->character_datas.emplace_back(definition_id, state);
            auto result = detail::table_accessor::make_uninitialized<character_handle<TStorage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = storage_.data,
                .data = &storage_.data->character_datas.back()
            };
            return result;
        }

        constexpr void clean_up() const requires is_mutable
        {
            storage_.data->clean_up();
        }

    private:
        constexpr basic_player_handle(detail::uninitialized_entity_t) noexcept {}

        storage_type storage_;

        constexpr size_t hand_card_slot_index(const card_data& data) const noexcept
        {
            GIVM_ASSERT(not storage_.data->hand_card_datas.empty());
            return static_cast<size_t>(&data - storage_.data->hand_card_datas.data());
        }

        constexpr size_t deck_card_slot_index(const card_data& data) const noexcept
        {
            GIVM_ASSERT(not storage_.data->deck_card_datas.empty());
            return static_cast<size_t>(&data - storage_.data->deck_card_datas.data());
        }

        constexpr deck_card_handle<TStorage> deck_card(deck_card_id card_id) const
        {
            GIVM_ASSERT(card_id.player_id == id());
            GIVM_ASSERT(card_id.index < storage_.data->deck_card_datas.size());
            auto result = detail::table_accessor::make_uninitialized<deck_card_handle<TStorage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = storage_.data,
                .slot = card_id.index,
                .data = &storage_.data->deck_card_datas[card_id.index]
            };
            return result;
        }

        constexpr void erase_deck_card(deck_card_id card_id) const requires is_mutable
        {
            GIVM_ASSERT(card_id.player_id == id());
            GIVM_ASSERT(card_id.index < storage_.data->deck_card_datas.size());
            auto& source = storage_.data->deck_card_datas[card_id.index];
            GIVM_ASSERT(source.definition_id.is_valid());
            detail::erase_statuses(*storage_.table, source);
            source.definition_id.set_invalid();
            const auto order = std::ranges::find(storage_.data->deck_card_order, card_id.index);
            GIVM_ASSERT(order != storage_.data->deck_card_order.end());
            storage_.data->deck_card_order.erase(order);
        }
    };
}

namespace givm
{
    class player_view : private detail::basic_player_handle<const detail::table_storage>
    {
        friend detail::table_accessor;

        using base_type = detail::basic_player_handle<const detail::table_storage>;

    public:
        using base_type::id;
        using base_type::state;
        using base_type::hand_cards;
        using base_type::deck_cards;
        using base_type::supports;
        using base_type::summons;
        using base_type::combat_statuses;
        using base_type::characters;
        using base_type::deck_card_count;
        using base_type::hand_card_count;
        using base_type::deck_card_definition;

    private:
        constexpr player_view(detail::uninitialized_entity_t tag) noexcept : base_type{ tag } {}
    };
}

#include "../../macro_undef.hpp"
#endif
