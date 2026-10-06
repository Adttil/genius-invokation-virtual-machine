#ifndef GIVM_TABLE_ENTITIES_REACTION_HANDLE_HPP
#define GIVM_TABLE_ENTITIES_REACTION_HANDLE_HPP

#include "../entity_storage.hpp"
#include "../table_accessor.hpp"
#include "../../utils/debug.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<class TStorage>
    class basic_reaction_handle
    {
        friend detail::table_accessor;
        friend class ::givm::reaction_view;

    public:
        using storage_type = reaction_entity_storage<TStorage>;

        constexpr const givm::table& table() const noexcept { return *storage_.table; }

        constexpr bool is_valid() const noexcept
        {
            return storage_.slot != elemental_reaction::none && static_cast<bool>(definition_id());
        }

        constexpr explicit operator bool() const noexcept { return is_valid(); }

        constexpr player_handle<TStorage> player() const
        {
            auto result = table_accessor::make_uninitialized<player_handle<TStorage>>();
            table_accessor::storage_of(result) = { storage_.table, storage_.player };
            return result;
        }

        constexpr reaction_id id() const { return { player().id(), storage_.slot }; }
        constexpr elemental_reaction slot() const noexcept { return storage_.slot; }

        constexpr givm::definition_id<reaction_view> definition_id() const noexcept
        {
            GIVM_ASSERT(storage_.slot != elemental_reaction::none);
            return storage_.player->reactions[static_cast<std::size_t>(storage_.slot) - 1];
        }

    private:
        constexpr basic_reaction_handle(uninitialized_entity_t) noexcept {}
        storage_type storage_;
    };
}

namespace givm
{
    class reaction_view : private detail::basic_reaction_handle<const detail::unrestricted_table>
    {
        friend detail::table_accessor;
        using base_type = detail::basic_reaction_handle<const detail::unrestricted_table>;

    public:
        using base_type::table;
        using base_type::is_valid;
        using base_type::operator bool;
        using base_type::player;
        using base_type::id;
        using base_type::slot;
        using base_type::definition_id;

    private:
        constexpr reaction_view(detail::uninitialized_entity_t tag) noexcept : base_type{ tag } {}
    };
}

#include "../../macro_undef.hpp"
#endif
