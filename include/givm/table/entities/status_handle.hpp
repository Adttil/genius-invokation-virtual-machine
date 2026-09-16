#ifndef GIVM_TABLE_ENTITIES_STATUS_HANDLE_HPP
#define GIVM_TABLE_ENTITIES_STATUS_HANDLE_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "../entity_id.hpp"
#include "../entity_storage.hpp"
#include "../table_accessor.hpp"
#include "../../utils/debug.hpp"
#include "../../utils/maybe_const.hpp"

#include "../../macro_define.hpp"

namespace givm::detail
{
    template<class TStorage>
    constexpr size_t add_status(
        TStorage& storage,
        card_data& card,
        givm::definition_id<status_definition> definition_id,
        const status_state& state
    )
    {
        storage.status_slots.push_back({
            .data = { .definition_and_flags = definition_id.value(), .state = state },
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

    constexpr void erase_statuses(table_storage& storage, card_data& card) noexcept
    {
        constexpr size_t erased_mask = size_t{ 1 } << (std::numeric_limits<size_t>::digits - 1);
        size_t current = card.first_status;
        while(current != invalid_status_index)
        {
            auto& slot = storage.status_slots[current];
            slot.data.definition_and_flags |= erased_mask;
            current = slot.next;
        }
    }

    template<class TStorage, class TStatusHandle, class TOwnerId>
    class card_status_range
    {
    public:
        class iterator
        {
        public:
            constexpr TStatusHandle operator*() const
            {
                GIVM_ASSERT(index_ < table_->status_slots.size());
                auto result = detail::table_accessor::make_uninitialized<TStatusHandle>();
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
                skip_erased();
                return *this;
            }

            friend constexpr bool operator==(const iterator&, const iterator&) = default;

        private:
            friend card_status_range;

            constexpr iterator(TStorage& table, TOwnerId owner, size_t index) noexcept
            : table_{ &table }, owner_{ owner }, index_{ index }
            {
                skip_erased();
            }

            constexpr void skip_erased() noexcept
            {
                constexpr size_t erased_mask = size_t{ 1 } << (std::numeric_limits<size_t>::digits - 1);
                while(index_ != invalid_status_index
                    && (table_->status_slots[index_].data.definition_and_flags & erased_mask) != 0)
                {
                    index_ = table_->status_slots[index_].next;
                }
            }

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
        friend class basic_hand_card_handle;
        template<class>
        friend class basic_deck_card_handle;

        constexpr card_status_range(TStorage& table, TOwnerId owner, size_t first) noexcept
        : table_{ &table }, owner_{ owner }, first_{ first }
        {}

        TStorage* table_;
        TOwnerId owner_;
        size_t first_;
    };

    template<class TStorage>
    class basic_hand_card_status_handle
    {
        friend detail::table_accessor;
        friend class ::givm::hand_card_status_view;

    public:
        static constexpr bool is_mutable = not std::is_const_v<TStorage>;
        using data_type = maybe_mutable<is_mutable, status_data>;
        using slot_type = maybe_mutable<is_mutable, status_slot>;
        using storage_type = detail::card_status_entity_storage<TStorage, hand_card_id>;

        constexpr operator hand_card_status_handle<const TStorage>() const noexcept requires is_mutable
        {
            auto result =
                detail::table_accessor::make_uninitialized<hand_card_status_handle<const TStorage>>();
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
            return card().is_valid() && (storage_.data->data.definition_and_flags & erased_mask) == 0;
        }

        constexpr explicit operator bool() const
        {
            return is_valid();
        }

        constexpr size_t size() const
        {
            return is_valid() ? 1uz : 0uz;
        }

        constexpr auto begin(this const auto& self)
        {
            return &self;
        }

        constexpr auto end(this const auto& self)
        {
            return &self + self.size();
        }

        constexpr auto card() const
        {
            auto result = detail::table_accessor::make_uninitialized<hand_card_handle<TStorage>>();
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
            GIVM_ASSERT(storage_.data->data.definition_and_flags != static_cast<size_t>(-1));
            return { storage_.owner, storage_.slot };
        }

        constexpr auto definition_id() const
        {
            GIVM_ASSERT(storage_.data->data.definition_and_flags != static_cast<size_t>(-1));
            return table_accessor::make_issued_id<status_definition>(
                storage_.data->data.definition_and_flags & ~erased_mask);
        }

        constexpr auto& state() const
        {
            GIVM_ASSERT(storage_.data->data.definition_and_flags != static_cast<size_t>(-1));
            return storage_.data->data.state;
        }

        constexpr void erase() const requires is_mutable
        {
            GIVM_ASSERT(card().is_valid());
            storage_.data->data.definition_and_flags |= erased_mask;
        }

    private:
        static constexpr size_t erased_mask = size_t{ 1 } << (std::numeric_limits<size_t>::digits - 1);

        constexpr basic_hand_card_status_handle(detail::uninitialized_entity_t) noexcept {}

        storage_type storage_;
    };

    template<class TStorage>
    class basic_deck_card_status_handle
    {
        friend detail::table_accessor;
        friend class ::givm::deck_card_status_view;

    public:
        static constexpr bool is_mutable = not std::is_const_v<TStorage>;
        using data_type = maybe_mutable<is_mutable, status_data>;
        using slot_type = maybe_mutable<is_mutable, status_slot>;
        using storage_type = detail::card_status_entity_storage<TStorage, deck_card_id>;

        constexpr operator deck_card_status_handle<const TStorage>() const noexcept requires is_mutable
        {
            auto result =
                detail::table_accessor::make_uninitialized<deck_card_status_handle<const TStorage>>();
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
            return card().is_valid() && (storage_.data->data.definition_and_flags & erased_mask) == 0;
        }

        constexpr explicit operator bool() const
        {
            return is_valid();
        }

        constexpr size_t size() const
        {
            return is_valid() ? 1uz : 0uz;
        }

        constexpr auto begin(this const auto& self)
        {
            return &self;
        }

        constexpr auto end(this const auto& self)
        {
            return &self + self.size();
        }

        constexpr auto card() const
        {
            auto result = detail::table_accessor::make_uninitialized<deck_card_handle<TStorage>>();
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
            GIVM_ASSERT(storage_.data->data.definition_and_flags != static_cast<size_t>(-1));
            return { storage_.owner, storage_.slot };
        }

        constexpr auto definition_id() const
        {
            GIVM_ASSERT(storage_.data->data.definition_and_flags != static_cast<size_t>(-1));
            return table_accessor::make_issued_id<status_definition>(
                storage_.data->data.definition_and_flags & ~erased_mask);
        }

        constexpr auto& state() const
        {
            GIVM_ASSERT(storage_.data->data.definition_and_flags != static_cast<size_t>(-1));
            return storage_.data->data.state;
        }

        constexpr void erase() const requires is_mutable
        {
            GIVM_ASSERT(card().is_valid());
            storage_.data->data.definition_and_flags |= erased_mask;
        }

    private:
        static constexpr size_t erased_mask = size_t{ 1 } << (std::numeric_limits<size_t>::digits - 1);

        constexpr basic_deck_card_status_handle(detail::uninitialized_entity_t) noexcept {}

        storage_type storage_;
    };
}

namespace givm
{
    class hand_card_status_view : private detail::basic_hand_card_status_handle<const detail::table_storage>
    {
        friend detail::table_accessor;

        using base_type = detail::basic_hand_card_status_handle<const detail::table_storage>;

    public:
        using base_type::is_valid;
        using base_type::operator bool;
        using base_type::size;
        using base_type::begin;
        using base_type::end;
        using base_type::player;
        using base_type::card;
        using base_type::id;
        using base_type::definition_id;
        using base_type::state;

    private:
        constexpr hand_card_status_view(detail::uninitialized_entity_t tag) noexcept : base_type{ tag } {}
    };

    class deck_card_status_view : private detail::basic_deck_card_status_handle<const detail::table_storage>
    {
        friend detail::table_accessor;

        using base_type = detail::basic_deck_card_status_handle<const detail::table_storage>;

    public:
        using base_type::is_valid;
        using base_type::operator bool;
        using base_type::size;
        using base_type::begin;
        using base_type::end;
        using base_type::player;
        using base_type::card;
        using base_type::id;
        using base_type::definition_id;
        using base_type::state;

    private:
        constexpr deck_card_status_view(detail::uninitialized_entity_t tag) noexcept : base_type{ tag } {}
    };
}

#include "../../macro_undef.hpp"
#endif
