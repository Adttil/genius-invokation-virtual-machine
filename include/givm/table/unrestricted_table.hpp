#ifndef GIVM_TABLE_UNRESTRICTED_TABLE_HPP
#define GIVM_TABLE_UNRESTRICTED_TABLE_HPP

#include <concepts>
#include <cstring>
#include <functional>
#include <limits>
#include <optional>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "entity_id.hpp"
#include "table_accessor.hpp"
#include "entities/status_handle.hpp"
#include "entities/player_handle.hpp"
#include "linked_deck.hpp"
#include "table_storage.hpp"
#include "../utils/debug.hpp"

#include "../macro_define.hpp"

namespace givm
{
    class table;
}

namespace givm::detail
{
    class unrestricted_table
    {
        friend detail::table_accessor;

    public:
        using game_state = table_state;

        // Conversion requires this object to be the base subobject of a table.
        template<std::same_as<table> T>
        constexpr operator T&() & noexcept
        {
            return static_cast<T&>(*this);
        }

        template<std::same_as<table> T>
        constexpr operator const T&() const & noexcept
        {
            return static_cast<const T&>(*this);
        }

        constexpr unrestricted_table(table_state state = {}, player_state player0 = {}, player_state player1 = {})
        : storage_{ .state = state, .player_datas = { { .state = player0 }, { .state = player1 } } }
        {}

        constexpr unrestricted_table(const unrestricted_table&) = default;
        constexpr unrestricted_table(unrestricted_table&&) noexcept = default;
        constexpr unrestricted_table& operator=(unrestricted_table&&) noexcept = default;

        constexpr unrestricted_table& operator=(const unrestricted_table& other)
        {
            if(this == &other) return *this;
            storage_.state = other.storage_.state;
            storage_.player_datas[0] = other.storage_.player_datas[0];
            storage_.player_datas[1] = other.storage_.player_datas[1];
            storage_.status_slots = other.storage_.status_slots;
            auto& history = storage_.history_summaries;
            const auto& source = other.storage_.history_summaries;
            history.resize(source.size());
            // memcpy starts the copied field lifetimes even when the layouts differ.
            if(not history.empty()) std::memcpy(history.data(), source.data(), history.size());
            return *this;
        }

        template<class Self>
        constexpr auto& state(this Self& self) noexcept
        {
            return detail::table_accessor::storage_of(self).state;
        }

        void reset_history(std::size_t size)
        {
            auto& history = storage_.history_summaries;
            history = std::remove_reference_t<decltype(history)>(size);
        }

        history_summary_state history_summary(std::size_t offset, std::size_t size) noexcept
        {
            auto& history = storage_.history_summaries;
            GIVM_ASSERT(offset <= history.size() && size <= history.size() - offset);
            auto* begin = history.data();
            if(begin) begin += offset;
            return history_summary_state{ begin, size };
        }

        template<class Self, class T>
        decltype(auto) operator[](this Self& self, history_value_key<T> key) noexcept
        {
            auto& history = detail::table_accessor::storage_of(self).history_summaries;
            return detail::access_history_value<T>(history.data(), history.size(), key.offset(), key.count());
        }

        template<class Self>
        constexpr auto players(this Self& self)
        {
            auto& storage = detail::table_accessor::storage_of(self);
            using table_type = std::remove_reference_t<decltype(self)>;
            return storage.player_datas
                | std::views::transform([&](auto& data)
                {
                    auto result = detail::table_accessor::make_uninitialized<player_handle<table_type>>();
                    detail::table_accessor::storage_of(result) = {
                        .table = &self,
                        .data = &data
                    };
                    return result;
                });
        }

        constexpr void load_deck(player_id player, const linked_deck& deck,
            const std::array<definition_id<definition_category::reaction>, elemental_reaction_count>& reactions = {})
        {
            auto& data = storage_.player_datas[player.index()];
            GIVM_ASSERT(data.deck_card_datas.empty());
            GIVM_ASSERT(data.deck_card_order.empty());
            GIVM_ASSERT(data.character_datas.empty());
            std::ranges::copy(reactions, data.reactions.begin());

            data.deck_card_datas.reserve(deck.cards.size());
            data.deck_card_order.reserve(deck.cards.size());
            for(definition_id<definition_category::card> definition : deck.cards)
            {
                data.deck_card_datas.emplace_back(definition.value(), card_state{});
                data.deck_card_order.push_back(data.deck_card_datas.size() - 1);
            }

            data.character_datas.reserve(deck.characters.size());
            for(definition_id<definition_category::character> definition : deck.characters)
            {
                data.character_datas.emplace_back(definition.value(), character_state{});
            }
        }

        template<class Self>
        constexpr reaction_view operator[](this Self& self, reaction_id id)
        {
            return self[id.player_id()].reaction(id.slot());
        }

        template<class Self>
        constexpr auto operator[](this Self& self, player_id player_id)
        {
            auto& storage = detail::table_accessor::storage_of(self);
            using table_type = std::remove_reference_t<decltype(self)>;
            auto result = detail::table_accessor::make_uninitialized<player_handle<table_type>>();
            detail::table_accessor::storage_of(result) = {
                .table = &self,
                .data = &storage.player_datas[player_id.index()]
            };
            return result;
        }

        template<class Self>
        constexpr auto operator[](this Self& self, support_id support_id)
        {
            const auto player = self[support_id.player_id()];
            return player.template supports<false>()[support_id.index()];
        }

        template<class Self>
        constexpr auto operator[](this Self& self, summon_id summon_id)
        {
            const auto player = self[summon_id.player_id()];
            return player.template summons<false>()[summon_id.index()];
        }

        template<class Self>
        constexpr auto operator[](this Self& self, combat_status_id combat_status_id)
        {
            const auto player = self[combat_status_id.player_id()];
            return player.template combat_statuses<false>()[combat_status_id.index()];
        }

        template<class Self>
        constexpr auto operator[](this Self& self, hand_card_id hand_card_id)
        {
            const auto player = self[hand_card_id.player_id()];
            return player.template hand_cards<false>()[hand_card_id.index()];
        }

        template<class Self>
        constexpr auto operator[](this Self& self, deck_card_id deck_card_id)
        {
            auto& storage = detail::table_accessor::storage_of(self);
            using table_type = std::remove_reference_t<decltype(self)>;
            auto& player = storage.player_datas[deck_card_id.player_id().index()];
            auto result = detail::table_accessor::make_uninitialized<deck_card_handle<table_type>>();
            detail::table_accessor::storage_of(result) = {
                .table = &self,
                .player = &player,
                .slot = deck_card_id.index(),
                .data = &player.deck_card_datas[deck_card_id.index()]
            };
            return result;
        }

        template<class Self>
        constexpr auto operator[](this Self& self, hand_card_status_id status_id)
        {
            auto& storage = detail::table_accessor::storage_of(self);
            using table_type = std::remove_reference_t<decltype(self)>;
            auto result = detail::table_accessor::make_uninitialized<hand_card_status_handle<table_type>>();
            detail::table_accessor::storage_of(result) = {
                .table = &self,
                .owner = status_id.hand_card_id(),
                .slot = status_id.index(),
                .data = &storage.status_slots[status_id.index()]
            };
            return result;
        }

        template<class Self>
        constexpr auto operator[](this Self& self, deck_card_status_id status_id)
        {
            auto& storage = detail::table_accessor::storage_of(self);
            using table_type = std::remove_reference_t<decltype(self)>;
            auto result = detail::table_accessor::make_uninitialized<deck_card_status_handle<table_type>>();
            detail::table_accessor::storage_of(result) = {
                .table = &self,
                .owner = status_id.deck_card_id(),
                .slot = status_id.index(),
                .data = &storage.status_slots[status_id.index()]
            };
            return result;
        }

        template<class Self>
        constexpr auto operator[](this Self& self, character_id character_id)
        {
            const auto player = self[character_id.player_id()];
            return player.template characters<false>()[character_id.index()];
        }

        template<class Self>
        constexpr auto operator[](this Self& self, skill_id skill_id)
        {
            const auto character = self[skill_id.character_id()];
            return character.template skills<false>()[skill_id.index()];
        }

        template<class Self>
        constexpr auto operator[](this Self& self, attachment_id attachment_id)
        {
            const auto character = self[attachment_id.character_id()];
            return character.template attachments<false>()[attachment_id.index()];
        }

#ifndef NDEBUG
        bool debug_entity_in_range(deck_card_id id) const noexcept
        {
            return id.player_id().index() < 2
                && id.index() < storage_.player_datas[id.player_id().index()].deck_card_datas.size();
        }

        bool debug_entity_in_range(hand_card_status_id id) const noexcept
        {
            if(id.hand_card_id().player_id().index() >= 2 || id.index() >= storage_.status_slots.size()) return false;
            const auto& cards = storage_.player_datas[id.hand_card_id().player_id().index()].hand_card_datas;
            if(id.hand_card_id().index() >= cards.size()) return false;
            for(auto index = cards[id.hand_card_id().index()].first_status; index != invalid_status_index;
                index = storage_.status_slots[index].next)
                if(index == id.index()) return true;
            return false;
        }

        bool debug_entity_in_range(deck_card_status_id id) const noexcept
        {
            if(not debug_entity_in_range(id.deck_card_id()) || id.index() >= storage_.status_slots.size()) return false;
            const auto& card = storage_.player_datas[id.deck_card_id().player_id().index()].deck_card_datas[id.deck_card_id().index()];
            for(auto index = card.first_status; index != invalid_status_index;
                index = storage_.status_slots[index].next)
                if(index == id.index()) return true;
            return false;
        }
#endif

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
            constexpr std::uint64_t erased_mask = std::uint64_t{ 1 } << detail::definition_id_bit_width;
            // Unlink erased statuses before compaction reuses their next fields.
            const auto clean_up_chain = [&](card_data& card)
            {
                size_t* link = &card.first_status;
                card.last_status = invalid_status_index;
                while(*link != invalid_status_index)
                {
                    auto& slot = storage_.status_slots[*link];
                    if((slot.data.definition_and_flags & erased_mask) == 0)
                    {
                        card.last_status = *link;
                        link = &slot.next;
                    }
                    else
                    {
                        *link = slot.next;
                    }
                }
            };
            for(auto& player : storage_.player_datas)
            {
                for(auto& card : player.hand_card_datas)
                {
                    clean_up_chain(card);
                }
                for(auto& card : player.deck_card_datas)
                {
                    clean_up_chain(card);
                }
            }

            size_t front = 0;
            size_t back = storage_.status_slots.size();
            while(true)
            {
                while(front < back && (storage_.status_slots[front].data.definition_and_flags & erased_mask) == 0)
                {
                    ++front;
                }
                while(front < back && (storage_.status_slots[back - 1].data.definition_and_flags & erased_mask) != 0)
                {
                    --back;
                }
                if(front == back)
                {
                    break;
                }

                const size_t source = --back;
                storage_.status_slots[front] = std::move(storage_.status_slots[source]);
                storage_.status_slots[source].data.definition_and_flags = static_cast<size_t>(-1);
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

#include "../macro_undef.hpp"
#endif
