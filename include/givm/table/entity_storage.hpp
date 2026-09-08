#ifndef GIVM_TABLE_ENTITY_STORAGE_HPP
#define GIVM_TABLE_ENTITY_STORAGE_HPP

#include <cstddef>
#include <type_traits>

#include "../utils/maybe_const.hpp"
#include "table_storage.hpp"

namespace givm::detail
{
    template<class TStorage>
    struct player_entity_storage
    {
        TStorage* table;
        maybe_mutable<not std::is_const_v<TStorage>, player_data>* data;
    };

    template<class TStorage>
    struct hand_card_entity_storage
    {
        TStorage* table;
        maybe_mutable<not std::is_const_v<TStorage>, player_data>* player;
        maybe_mutable<not std::is_const_v<TStorage>, card_data>* data;
    };

    template<class TStorage>
    struct deck_card_entity_storage
    {
        TStorage* table;
        maybe_mutable<not std::is_const_v<TStorage>, player_data>* player;
        size_t slot;
        maybe_mutable<not std::is_const_v<TStorage>, card_data>* data;
    };

    template<class TStorage, class TOwnerId>
    struct card_status_entity_storage
    {
        TStorage* table;
        TOwnerId owner;
        size_t slot;
        maybe_mutable<not std::is_const_v<TStorage>, status_slot>* data;
    };

    template<class TStorage>
    struct support_entity_storage
    {
        TStorage* table;
        maybe_mutable<not std::is_const_v<TStorage>, player_data>* player;
        maybe_mutable<not std::is_const_v<TStorage>, support_data>* data;
    };

    template<class TStorage>
    struct summon_entity_storage
    {
        TStorage* table;
        maybe_mutable<not std::is_const_v<TStorage>, player_data>* player;
        maybe_mutable<not std::is_const_v<TStorage>, summon_data>* data;
    };

    template<class TStorage>
    struct combat_status_entity_storage
    {
        TStorage* table;
        maybe_mutable<not std::is_const_v<TStorage>, player_data>* player;
        maybe_mutable<not std::is_const_v<TStorage>, combat_status_data>* data;
    };

    template<class TStorage>
    struct character_entity_storage
    {
        TStorage* table;
        maybe_mutable<not std::is_const_v<TStorage>, player_data>* player;
        maybe_mutable<not std::is_const_v<TStorage>, character_data>* data;
    };

    template<class TStorage>
    struct skill_entity_storage
    {
        TStorage* table;
        maybe_mutable<not std::is_const_v<TStorage>, player_data>* player;
        maybe_mutable<not std::is_const_v<TStorage>, character_data>* character;
        maybe_mutable<not std::is_const_v<TStorage>, skill_data>* data;
    };

    template<class TStorage>
    struct attachment_entity_storage
    {
        TStorage* table;
        maybe_mutable<not std::is_const_v<TStorage>, player_data>* player;
        maybe_mutable<not std::is_const_v<TStorage>, character_data>* character;
        maybe_mutable<not std::is_const_v<TStorage>, attachment_data>* data;
    };
}

#endif
