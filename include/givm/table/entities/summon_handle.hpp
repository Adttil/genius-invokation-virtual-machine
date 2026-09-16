#ifndef GIVM_TABLE_ENTITIES_SUMMON_HANDLE_HPP
#define GIVM_TABLE_ENTITIES_SUMMON_HANDLE_HPP

#include <limits>
#include <type_traits>

#include "../entity_id.hpp"
#include "../entity_storage.hpp"
#include "../table_accessor.hpp"
#include "../../utils/maybe_const.hpp"
#include "../../utils/debug.hpp"

#include "../../macro_define.hpp"

namespace givm::detail
{
    template<class TStorage>
    class basic_summon_handle
    {
        friend detail::table_accessor;
        friend class ::givm::summon_view;

    public:
        static constexpr bool is_mutable = not std::is_const_v<TStorage>;

        using table_type = TStorage;
        using player_type = player_handle<TStorage>;
        using data_type = maybe_mutable<is_mutable, summon_data>;
        using storage_type = detail::summon_entity_storage<TStorage>;

        constexpr operator summon_handle<const TStorage>() const noexcept requires is_mutable
        {
            auto result = detail::table_accessor::make_uninitialized<summon_handle<const TStorage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = storage_.player,
                .data = storage_.data
            };
            return result;
        }

        constexpr bool is_valid() const
        {
            return (storage_.data->definition_and_flags & erased_mask) == 0;
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

        constexpr summon_id id() const
        {
            GIVM_ASSERT(storage_.data->definition_and_flags != static_cast<size_t>(-1));
            return {
                player().id(),
                static_cast<size_t>(storage_.data - storage_.player->summon_datas.data())
            };
        }

        constexpr auto definition_id() const
        {
            GIVM_ASSERT(storage_.data->definition_and_flags != static_cast<size_t>(-1));
            return detail::table_accessor::make_issued_id<summon_view>(
                storage_.data->definition_and_flags & ~erased_mask);
        }

        constexpr auto& state() const
        {
            GIVM_ASSERT(storage_.data->definition_and_flags != static_cast<size_t>(-1));
            return storage_.data->state;
        }

        constexpr void erase() const requires is_mutable
        {
            storage_.data->definition_and_flags |= erased_mask;
        }

    private:
        static constexpr size_t erased_mask =
            size_t{ 1 } << (std::numeric_limits<size_t>::digits - 1);

        constexpr basic_summon_handle(detail::uninitialized_entity_t) noexcept {}

        storage_type storage_;
    };
}

namespace givm
{
    class summon_view : private detail::basic_summon_handle<const detail::table_storage>
    {
        friend detail::table_accessor;

        using base_type = detail::basic_summon_handle<const detail::table_storage>;

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

    private:
        constexpr summon_view(detail::uninitialized_entity_t tag) noexcept : base_type{ tag } {}
    };
}

#include "../../macro_undef.hpp"
#endif
