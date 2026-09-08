#ifndef GIVM_TABLE_ENTITIES_SKILL_ENTITY_HPP
#define GIVM_TABLE_ENTITIES_SKILL_ENTITY_HPP

#include <type_traits>

#include "../entity_id.hpp"
#include "../entity_storage.hpp"
#include "../table_accessor.hpp"
#include "../../utils/maybe_const.hpp"
#include "../../utils/debug.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    template<class TStorage>
    class skill_entity
    {
        friend detail::table_accessor;

    public:
        static constexpr bool is_mutable = not std::is_const_v<TStorage>;

        using table_type = TStorage;
        using character_type = character_entity<TStorage>;
        using data_type = maybe_mutable<is_mutable, skill_data>;
        using player_type = player_entity<TStorage>;
        using storage_type = detail::skill_entity_storage<TStorage>;

        constexpr operator skill_entity<const detail::table_storage>() const noexcept requires is_mutable
        {
            auto result = detail::table_accessor::make_uninitialized<skill_entity<const detail::table_storage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = storage_.player,
                .character = storage_.character,
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

        constexpr character_type character() const
        {
            auto result = detail::table_accessor::make_uninitialized<character_type>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = storage_.player,
                .data = storage_.character
            };
            return result;
        }

        constexpr skill_id id() const
        {
            GIVM_ASSERT(is_valid());
            return {
                character().id(),
                static_cast<size_t>(storage_.data - storage_.character->skill_datas.data())
            };
        }

        constexpr auto definition() const
        {
            GIVM_ASSERT(is_valid());
            return (*storage_.table->definition_library_)[storage_.data->definition_id];
        }

        template<class TEvent>
        bool can_handle() const
        {
            return is_valid() && definition().template can_handle<TEvent, skill_view>();
        }

        constexpr auto& state() const
        {
            GIVM_ASSERT(is_valid());
            return storage_.data->state;
        }


        constexpr void erase() const requires is_mutable
        {
            storage_.data->definition_id.set_invalid();
        }

    private:
        constexpr skill_entity(detail::uninitialized_entity_t) noexcept {}

        storage_type storage_;
    };
}

#include "../../macro_undef.hpp"
#endif
