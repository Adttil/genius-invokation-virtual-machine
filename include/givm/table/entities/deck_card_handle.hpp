#ifndef GIVM_TABLE_ENTITIES_DECK_CARD_HANDLE_HPP
#define GIVM_TABLE_ENTITIES_DECK_CARD_HANDLE_HPP

#include <algorithm>
#include <type_traits>

#include "../entity_id.hpp"
#include "../entity_storage.hpp"
#include "../table_accessor.hpp"
#include "../../utils/debug.hpp"
#include "../../utils/maybe_const.hpp"
#include "status_handle.hpp"

#include "../../macro_define.hpp"

namespace givm::detail
{
    template<class TStorage>
    class basic_deck_card_handle
    {
        friend detail::table_accessor;
        friend class ::givm::deck_card_view;

    public:
        static constexpr bool is_mutable = not std::is_const_v<TStorage>;

        using table_type = TStorage;
        using player_type = player_handle<TStorage>;
        using data_type = maybe_mutable<is_mutable, card_data>;
        using storage_type = detail::deck_card_entity_storage<TStorage>;

        constexpr operator deck_card_handle<const TStorage>() const noexcept requires is_mutable
        {
            auto result = detail::table_accessor::make_uninitialized<deck_card_handle<const TStorage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = storage_.player,
                .slot = storage_.slot,
                .data = storage_.data
            };
            return result;
        }

        constexpr bool is_valid() const
        {
            return storage_.data->definition_id.is_valid();
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

        constexpr player_type player() const
        {
            auto result = detail::table_accessor::make_uninitialized<player_type>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .data = storage_.player
            };
            return result;
        }

        constexpr deck_card_id id() const
        {
            GIVM_ASSERT(is_valid());
            return { player().id(), storage_.slot };
        }

        constexpr auto definition_id() const
        {
            GIVM_ASSERT(is_valid());
            return storage_.data->definition_id;
        }

        constexpr auto& state() const
        {
            GIVM_ASSERT(is_valid());
            return storage_.data->state;
        }

        constexpr auto statuses() const
        {
            GIVM_ASSERT(is_valid());
            using status_entity_type = deck_card_status_handle<TStorage>;
            return card_status_range<TStorage, status_entity_type, deck_card_id>{
                *storage_.table, id(), storage_.data->first_status
            };
        }

        constexpr deck_card_status_handle<TStorage> add(
            givm::definition_id<status_definition> definition_id,
            const status_state& state
        ) const requires is_mutable
        {
            GIVM_ASSERT(is_valid());
            const size_t index = detail::add_status(*storage_.table, *storage_.data, definition_id, state);
            auto result = detail::table_accessor::make_uninitialized<deck_card_status_handle<TStorage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .owner = id(),
                .slot = index,
                .data = &storage_.table->status_slots[index]
            };
            return result;
        }

        constexpr void erase() const requires is_mutable
        {
            detail::erase_statuses(*storage_.table, *storage_.data);
            storage_.data->definition_id.set_invalid();
            const auto order = std::ranges::find(storage_.player->deck_card_order, storage_.slot);
            GIVM_ASSERT(order != storage_.player->deck_card_order.end());
            storage_.player->deck_card_order.erase(order);
        }

    private:
        constexpr basic_deck_card_handle(detail::uninitialized_entity_t) noexcept {}

        storage_type storage_;
    };
}

namespace givm
{
    class deck_card_view : private detail::basic_deck_card_handle<const detail::table_storage>
    {
        friend detail::table_accessor;

        using base_type = detail::basic_deck_card_handle<const detail::table_storage>;

    public:
        using base_type::is_valid;
        using base_type::operator bool;
        using base_type::size;
        using base_type::begin;
        using base_type::end;
        using base_type::player;
        using base_type::id;
        using base_type::definition_id;
        using base_type::state;
        using base_type::statuses;

    private:
        constexpr deck_card_view(detail::uninitialized_entity_t tag) noexcept : base_type{ tag } {}
    };
}

#include "../../macro_undef.hpp"
#endif
