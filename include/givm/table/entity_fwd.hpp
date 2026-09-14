#ifndef GIVM_TABLE_ENTITY_FWD_HPP
#define GIVM_TABLE_ENTITY_FWD_HPP

#include <type_traits>

namespace givm
{
    class player_view;
    class hand_card_view;
    class deck_card_view;
    class hand_card_status_view;
    class deck_card_status_view;
    class support_view;
    class summon_view;
    class combat_status_view;
    class character_view;
    class skill_view;
    class attachment_view;

    namespace detail
    {
        struct table_storage;
        class unrestricted_table;

        template<class TStorage>
        class basic_player_handle;

        template<class TStorage>
        using player_handle = std::conditional_t<
            std::is_const_v<TStorage>, player_view, basic_player_handle<TStorage>
        >;

        template<class TStorage>
        class basic_hand_card_handle;

        template<class TStorage>
        using hand_card_handle = std::conditional_t<
            std::is_const_v<TStorage>, hand_card_view, basic_hand_card_handle<TStorage>
        >;

        template<class TStorage>
        class basic_deck_card_handle;

        template<class TStorage>
        using deck_card_handle = std::conditional_t<
            std::is_const_v<TStorage>, deck_card_view, basic_deck_card_handle<TStorage>
        >;

        template<class TStorage>
        class basic_hand_card_status_handle;

        template<class TStorage>
        using hand_card_status_handle = std::conditional_t<
            std::is_const_v<TStorage>, hand_card_status_view, basic_hand_card_status_handle<TStorage>
        >;

        template<class TStorage>
        class basic_deck_card_status_handle;

        template<class TStorage>
        using deck_card_status_handle = std::conditional_t<
            std::is_const_v<TStorage>, deck_card_status_view, basic_deck_card_status_handle<TStorage>
        >;

        template<class TStorage>
        class basic_support_handle;

        template<class TStorage>
        using support_handle = std::conditional_t<
            std::is_const_v<TStorage>, support_view, basic_support_handle<TStorage>
        >;

        template<class TStorage>
        class basic_summon_handle;

        template<class TStorage>
        using summon_handle = std::conditional_t<
            std::is_const_v<TStorage>, summon_view, basic_summon_handle<TStorage>
        >;

        template<class TStorage>
        class basic_combat_status_handle;

        template<class TStorage>
        using combat_status_handle = std::conditional_t<
            std::is_const_v<TStorage>, combat_status_view, basic_combat_status_handle<TStorage>
        >;

        template<class TStorage>
        class basic_character_handle;

        template<class TStorage>
        using character_handle = std::conditional_t<
            std::is_const_v<TStorage>, character_view, basic_character_handle<TStorage>
        >;

        template<class TStorage>
        class basic_skill_handle;

        template<class TStorage>
        using skill_handle = std::conditional_t<
            std::is_const_v<TStorage>, skill_view, basic_skill_handle<TStorage>
        >;

        template<class TStorage>
        class basic_attachment_handle;

        template<class TStorage>
        using attachment_handle = std::conditional_t<
            std::is_const_v<TStorage>, attachment_view, basic_attachment_handle<TStorage>
        >;
    }
}

#endif
