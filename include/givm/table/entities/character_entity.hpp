#ifndef GIVM_TABLE_ENTITIES_CHARACTER_ENTITY_HPP
#define GIVM_TABLE_ENTITIES_CHARACTER_ENTITY_HPP

#include <type_traits>
#include <ranges>

#include "../../enums/element_aura.hpp"
#include "../entity_id.hpp"
#include "../entity_storage.hpp"
#include "../table_accessor.hpp"
#include "../../utils/maybe_const.hpp"
#include "../../utils/debug.hpp"
#include "attachment_entity.hpp"
#include "skill_entity.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    template<class TStorage>
    class character_entity
    {
        friend detail::table_accessor;

    public:
        static constexpr bool is_mutable = not std::is_const_v<TStorage>;

        using table_type = TStorage;
        using player_type = player_entity<TStorage>;
        using data_type = maybe_mutable<is_mutable, character_data>;
        using storage_type = detail::character_entity_storage<TStorage>;

        constexpr operator character_entity<const detail::table_storage>() const noexcept requires is_mutable
        {
            auto result = detail::table_accessor::make_uninitialized<character_entity<const detail::table_storage>>();
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

        constexpr character_id id() const
        {
            GIVM_ASSERT(is_valid());
            return {
                player().id(),
                static_cast<size_t>(storage_.data - storage_.player->character_datas.data())
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
            return is_valid() && definition().template can_handle<TEvent, character_view>();
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
                        auto result = detail::table_accessor::make_uninitialized<skill_entity<TStorage>>();
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
                        auto result = detail::table_accessor::make_uninitialized<attachment_entity<TStorage>>();
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

        constexpr skill_entity<TStorage> add(definition_id<skill_view> definition_id, const skill_state& state) const requires is_mutable
        {
            GIVM_ASSERT(is_valid());
            storage_.data->skill_datas.emplace_back(definition_id, state);
            auto result = detail::table_accessor::make_uninitialized<skill_entity<TStorage>>();
            detail::table_accessor::storage_of(result) = {
                .table = storage_.table,
                .player = storage_.player,
                .character = storage_.data,
                .data = &storage_.data->skill_datas.back()
            };
            return result;
        }

        constexpr attachment_entity<TStorage> add(definition_id<attachment_view> definition_id, const attachment_state& state) const requires is_mutable
        {
            GIVM_ASSERT(is_valid());
            storage_.data->attachment_datas.emplace_back(definition_id, state);
            auto result = detail::table_accessor::make_uninitialized<attachment_entity<TStorage>>();
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
        constexpr character_entity(detail::uninitialized_entity_t) noexcept {}

        storage_type storage_;
    };


}



#include "../../macro_undef.hpp"
#endif
