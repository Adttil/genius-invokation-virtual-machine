#ifndef GIVM_TABLE_ENTITIES_STATUS_ENTITY_HPP
#define GIVM_TABLE_ENTITIES_STATUS_ENTITY_HPP

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "../entity_id.hpp"
#include "../entity_storage.hpp"
#include "../table_accessor.hpp"
#include "../../utils/debug.hpp"
#include "../../utils/maybe_const.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    namespace detail
    {
        constexpr card_data& card_data_of(table_storage& storage, hand_card_id id) noexcept
        {
            return storage.player_datas[id.player_id.index].hand_card_datas[id.index];
        }

        constexpr card_data& card_data_of(table_storage& storage, deck_card_id id) noexcept
        {
            return storage.player_datas[id.player_id.index].deck_card_datas[id.index];
        }

        template<class TStorage>
        constexpr size_t add_status(
            TStorage& storage,
            card_data& card,
            definition_id<status_definition> definition_id,
            const status_state& state
        )
        {
            GIVM_ASSERT(card.definition_id.is_valid());
            storage.status_slots.push_back({
                .data = { .definition_id = definition_id, .state = state },
                .next = invalid_status_index
            });
            const size_t index = storage.status_slots.size() - 1;
            if(card.last_status == invalid_status_index)
            {
                GIVM_ASSERT(card.first_status == invalid_status_index);
                card.first_status = index;
            }
            else
            {
                storage.status_slots[card.last_status].next = index;
            }
            card.last_status = index;
            return index;
        }

        template<class TCardId>
        constexpr void erase_status(table_storage& storage, TCardId owner, size_t index)
        {
            auto& data = card_data_of(storage, owner);
            GIVM_ASSERT(data.definition_id.is_valid());

            size_t previous = invalid_status_index;
            size_t current = data.first_status;
            while(current != index)
            {
                GIVM_ASSERT(current != invalid_status_index);
                previous = current;
                current = storage.status_slots[current].next;
            }

            auto& slot = storage.status_slots[index];
            const size_t next = slot.next;
            if(previous == invalid_status_index)
            {
                data.first_status = next;
            }
            else
            {
                storage.status_slots[previous].next = next;
            }
            if(data.last_status == index)
            {
                data.last_status = previous;
            }
            slot.data.definition_id.set_invalid();
            slot.next = invalid_status_index;
        }

        constexpr void erase_statuses(table_storage& storage, card_data& card) noexcept
        {
            size_t current = card.first_status;
            while(current != invalid_status_index)
            {
                auto& slot = storage.status_slots[current];
                const size_t next = slot.next;
                slot.data.definition_id.set_invalid();
                slot.next = invalid_status_index;
                current = next;
            }
            card.first_status = invalid_status_index;
            card.last_status = invalid_status_index;
        }
    }

    template<class TStorage, class TStatusEntity, class TOwnerId>
    class card_status_range
    {
    public:
        class iterator
        {
        public:
            constexpr TStatusEntity operator*() const
            {
                GIVM_ASSERT(index_ < table_->status_slots.size());
                auto result = detail::table_accessor::make_uninitialized<TStatusEntity>();
                detail::table_accessor::storage_of(result) = {
                    .table = table_,
                    .owner = owner_,
                    .slot = index_,
                    .data = &table_->status_slots[index_]
                };
                return result;
            }

            constexpr iterator& operator++()
            {
                GIVM_ASSERT(index_ < table_->status_slots.size());
                index_ = table_->status_slots[index_].next;
                return *this;
            }

            friend constexpr bool operator==(const iterator&, const iterator&) = default;

        private:
            friend card_status_range;

            constexpr iterator(TStorage& table, TOwnerId owner, size_t index) noexcept
            : table_{ &table }, owner_{ owner }, index_{ index }
            {}

            TStorage* table_;
            TOwnerId owner_;
            size_t index_;
        };

        constexpr iterator begin() const noexcept
        {
            return { *table_, owner_, first_ };
        }

        constexpr iterator end() const noexcept
        {
            return { *table_, owner_, invalid_status_index };
        }

    private:
        template<class>
        friend class hand_card_entity;
        template<class>
        friend class deck_card_entity;

        constexpr card_status_range(TStorage& table, TOwnerId owner, size_t first) noexcept
        : table_{ &table }, owner_{ owner }, first_{ first }
        {}

        TStorage* table_;
        TOwnerId owner_;
        size_t first_;
    };

    template<class TStorage>
    class hand_card_status_entity
    {
        friend detail::table_accessor;

    public:
        static constexpr bool is_mutable = not std::is_const_v<TStorage>;
        using data_type = maybe_mutable<is_mutable, status_data>;
        using slot_type = maybe_mutable<is_mutable, status_slot>;
        using storage_type = detail::card_status_entity_storage<TStorage, hand_card_id>;

        constexpr operator hand_card_status_entity<const detail::table_storage>() const noexcept requires is_mutable
        {
            auto result =
                detail::table_accessor::make_uninitialized<hand_card_status_entity<const detail::table_storage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .owner = storage_.owner,
                .slot = storage_.slot,
                .data = storage_.data
            };
            return result;
        }

        constexpr bool is_valid() const
        {
            const auto& card = storage_.table->player_datas[storage_.owner.player_id.index]
                .hand_card_datas[storage_.owner.index];
            return card.definition_id.is_valid() && storage_.data->data.definition_id.is_valid();
        }

        constexpr explicit operator bool() const
        {
            return is_valid();
        }

        constexpr size_t size() const
        {
            return is_valid() ? 1uz : 0uz;
        }

        constexpr auto begin() const
        {
            return this;
        }

        constexpr auto end() const
        {
            return this + size();
        }

        constexpr auto card() const
        {
            auto result = detail::table_accessor::make_uninitialized<hand_card_entity<TStorage>>();
            auto& player = storage_.table->player_datas[storage_.owner.player_id.index];
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = &player,
                .data = &player.hand_card_datas[storage_.owner.index]
            };
            return result;
        }

        constexpr auto player() const
        {
            return card().player();
        }

        constexpr hand_card_status_id id() const
        {
            GIVM_ASSERT(is_valid());
            return { storage_.owner, storage_.slot };
        }

        constexpr auto definition() const
        {
            GIVM_ASSERT(is_valid());
            return (*storage_.table->definition_library_)[storage_.data->data.definition_id];
        }

        template<class TEvent>
        bool can_handle() const
        {
            return is_valid() && definition().template can_handle<TEvent, hand_card_status_view>();
        }

        constexpr auto& state() const
        {
            GIVM_ASSERT(is_valid());
            return storage_.data->data.state;
        }

        constexpr void erase() const requires is_mutable
        {
            detail::erase_status(*storage_.table, storage_.owner, storage_.slot);
        }

    private:
        constexpr hand_card_status_entity(detail::uninitialized_entity_t) noexcept {}

        storage_type storage_;
    };

    template<class TStorage>
    class deck_card_status_entity
    {
        friend detail::table_accessor;

    public:
        static constexpr bool is_mutable = not std::is_const_v<TStorage>;
        using data_type = maybe_mutable<is_mutable, status_data>;
        using slot_type = maybe_mutable<is_mutable, status_slot>;
        using storage_type = detail::card_status_entity_storage<TStorage, deck_card_id>;

        constexpr operator deck_card_status_entity<const detail::table_storage>() const noexcept requires is_mutable
        {
            auto result =
                detail::table_accessor::make_uninitialized<deck_card_status_entity<const detail::table_storage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .owner = storage_.owner,
                .slot = storage_.slot,
                .data = storage_.data
            };
            return result;
        }

        constexpr bool is_valid() const
        {
            const auto& card = storage_.table->player_datas[storage_.owner.player_id.index]
                .deck_card_datas[storage_.owner.index];
            return card.definition_id.is_valid() && storage_.data->data.definition_id.is_valid();
        }

        constexpr explicit operator bool() const
        {
            return is_valid();
        }

        constexpr size_t size() const
        {
            return is_valid() ? 1uz : 0uz;
        }

        constexpr auto begin() const
        {
            return this;
        }

        constexpr auto end() const
        {
            return this + size();
        }

        constexpr auto card() const
        {
            auto result = detail::table_accessor::make_uninitialized<deck_card_entity<TStorage>>();
            auto& player = storage_.table->player_datas[storage_.owner.player_id.index];
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = &player,
                .slot = storage_.owner.index,
                .data = &player.deck_card_datas[storage_.owner.index]
            };
            return result;
        }

        constexpr auto player() const
        {
            return card().player();
        }

        constexpr deck_card_status_id id() const
        {
            GIVM_ASSERT(is_valid());
            return { storage_.owner, storage_.slot };
        }

        constexpr auto definition() const
        {
            GIVM_ASSERT(is_valid());
            return (*storage_.table->definition_library_)[storage_.data->data.definition_id];
        }

        template<class TEvent>
        bool can_handle() const
        {
            return is_valid() && definition().template can_handle<TEvent, deck_card_status_view>();
        }

        constexpr auto& state() const
        {
            GIVM_ASSERT(is_valid());
            return storage_.data->data.state;
        }

        constexpr void erase() const requires is_mutable
        {
            detail::erase_status(*storage_.table, storage_.owner, storage_.slot);
        }

    private:
        constexpr deck_card_status_entity(detail::uninitialized_entity_t) noexcept {}

        storage_type storage_;
    };
}

#include "../../macro_undef.hpp"
#endif
