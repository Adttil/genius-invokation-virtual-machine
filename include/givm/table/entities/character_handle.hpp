#ifndef GIVM_TABLE_ENTITIES_CHARACTER_HANDLE_HPP
#define GIVM_TABLE_ENTITIES_CHARACTER_HANDLE_HPP

#include <type_traits>
#include <ranges>

#include "../../enums/element_aura.hpp"
#include "../entity_id.hpp"
#include "../entity_storage.hpp"
#include "../table_accessor.hpp"
#include "../../utils/maybe_const.hpp"
#include "../../utils/debug.hpp"
#include "attachment_handle.hpp"
#include "skill_handle.hpp"

#include "../../macro_define.hpp"

namespace givm::detail
{
    template<class TStorage>
    class basic_character_handle
    {
        friend detail::table_accessor;
        friend class ::givm::character_view;

    public:
        static constexpr bool is_mutable = not std::is_const_v<TStorage>;

        using table_type = TStorage;
        using player_type = player_handle<TStorage>;
        using data_type = maybe_mutable<is_mutable, character_data>;
        using storage_type = detail::character_entity_storage<TStorage>;

        constexpr operator character_handle<const TStorage>() const noexcept requires is_mutable
        {
            auto result = detail::table_accessor::make_uninitialized<character_handle<const TStorage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = storage_.player,
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

        constexpr character_id id() const
        {
            GIVM_ASSERT(is_valid());
            return {
                player().id(),
                static_cast<size_t>(storage_.data - storage_.player->character_datas.data())
            };
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

        constexpr void erase() const requires is_mutable
        {
            GIVM_ASSERT(is_valid());
            for(auto&& skill : skills())
            {
                skill.erase();
            }
            for(auto&& attachment : attachments())
            {
                attachment.erase();
            }
            storage_.data->definition_id.set_invalid();
        }

        template<bool SkipErased = true>
        constexpr auto skills() const
        {
            if constexpr(SkipErased)
            {
                GIVM_ASSERT(is_valid());
                return skills<false>() | std::views::filter([](auto&& skill){ return skill.is_valid(); });
            }
            else
            {
                return storage_.data->skill_datas
                    | std::views::transform([character = *this](auto& data)
                    {
                        auto result = detail::table_accessor::make_uninitialized<skill_handle<TStorage>>();
                        detail::table_accessor::storage_of(result) = {
                            .table = character.storage_.table,
                            .player = character.storage_.player,
                            .character = character.storage_.data,
                            .data = &data
                        };
                        return result;
                    });
            }
        }

        template<bool SkipErased = true>
        constexpr auto attachments() const
        {
            if constexpr(SkipErased)
            {
                GIVM_ASSERT(is_valid());
                return attachments<false>() | std::views::filter([](auto&& attachment){ return attachment.is_valid(); });
            }
            else
            {
                return storage_.data->attachment_datas
                    | std::views::transform([character = *this](auto& data)
                    {
                        auto result = detail::table_accessor::make_uninitialized<attachment_handle<TStorage>>();
                        detail::table_accessor::storage_of(result) = {
                            .table = character.storage_.table,
                            .player = character.storage_.player,
                            .character = character.storage_.data,
                            .data = &data
                        };
                        return result;
                    });
            }
        }

        constexpr skill_handle<TStorage> add(givm::definition_id<skill_view> definition_id, const skill_state& state) const requires is_mutable
        {
            GIVM_ASSERT(is_valid());
            storage_.data->skill_datas.emplace_back(definition_id, state);
            auto result = detail::table_accessor::make_uninitialized<skill_handle<TStorage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = storage_.player,
                .character = storage_.data,
                .data = &storage_.data->skill_datas.back()
            };
            return result;
        }

        constexpr attachment_handle<TStorage> add(givm::definition_id<attachment_view> definition_id, const attachment_state& state) const requires is_mutable
        {
            GIVM_ASSERT(is_valid());
            storage_.data->attachment_datas.emplace_back(definition_id, state);
            auto result = detail::table_accessor::make_uninitialized<attachment_handle<TStorage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = storage_.player,
                .character = storage_.data,
                .data = &storage_.data->attachment_datas.back()
            };
            return result;
        }

        constexpr void clean_up() const requires is_mutable
        {
            GIVM_ASSERT(is_valid());
            storage_.data->clean_up();
        }

    private:
        constexpr basic_character_handle(detail::uninitialized_entity_t) noexcept {}

        storage_type storage_;
    };
}

namespace givm
{
    class character_view : private detail::basic_character_handle<const detail::table_storage>
    {
        friend detail::table_accessor;

        using base_type = detail::basic_character_handle<const detail::table_storage>;

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
        using base_type::skills;
        using base_type::attachments;

    private:
        constexpr character_view(detail::uninitialized_entity_t tag) noexcept : base_type{ tag } {}
    };
}

#include "../../macro_undef.hpp"
#endif
