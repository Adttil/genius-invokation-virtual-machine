#ifndef GIVM_TABLE_ENTITIES_DECK_CARD_ENTITY_HPP
#define GIVM_TABLE_ENTITIES_DECK_CARD_ENTITY_HPP

#include <algorithm>
#include <type_traits>

#include "../entity_id.hpp"
#include "../entity_storage.hpp"
#include "../table_accessor.hpp"
#include "../../utils/debug.hpp"
#include "../../utils/maybe_const.hpp"
#include "status_entity.hpp"

#include "../../macro_define.hpp"

namespace givm
{
template<class TStorage>
    class deck_card_entity
    {
        friend detail::table_accessor;

    public:
        static constexpr bool is_mutable = not std::is_const_v<TStorage>;

        using table_type = TStorage;
        using player_type = player_entity<TStorage>;
        using data_type = maybe_mutable<is_mutable, card_data>;
        using storage_type = detail::deck_card_entity_storage<TStorage>;

        constexpr operator deck_card_entity<const detail::table_storage>() const noexcept requires is_mutable
        {
            auto result = detail::table_accessor::make_uninitialized<deck_card_entity<const detail::table_storage>>();
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

        constexpr auto begin() const
        {
            return this;
        }

        constexpr auto end() const
        {
            return this + size();
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

        constexpr auto definition() const
        {
            GIVM_ASSERT(is_valid());
            return (*storage_.table->definition_library_)[storage_.data->definition_id];
        }

        template<class TEvent>
        bool can_handle() const
        {
            return is_valid() && definition().template can_handle<TEvent, deck_card_view>();
        }

        constexpr auto& state() const
        {
            GIVM_ASSERT(is_valid());
            return storage_.data->state;
        }

        constexpr auto statuses() const
        {
            GIVM_ASSERT(is_valid());
            using status_entity_type = deck_card_status_entity<TStorage>;
            return card_status_range<TStorage, status_entity_type, deck_card_id>{
                *storage_.table, id(), storage_.data->first_status
            };
        }

        constexpr deck_card_status_entity<TStorage> add(
            definition_id<status_definition> definition_id,
            const status_state& state
        ) const requires is_mutable
        {
            GIVM_ASSERT(is_valid());
            const size_t index = detail::add_status(*storage_.table, *storage_.data, definition_id, state);
            auto result = detail::table_accessor::make_uninitialized<deck_card_status_entity<TStorage>>();
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
        constexpr deck_card_entity(detail::uninitialized_entity_t) noexcept {}

        storage_type storage_;
    };
}

#include "../../macro_undef.hpp"
#endif
