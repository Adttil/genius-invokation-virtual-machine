#ifndef GIVM_TABLE_ENTITY_FWD_HPP
#define GIVM_TABLE_ENTITY_FWD_HPP

namespace givm
{
    namespace detail
    {
        struct table_storage;
    }

    template<class TStorage>
    class player_entity;

    template<class TStorage>
    class hand_card_entity;

    template<class TStorage>
    class deck_card_entity;

    template<class TStorage>
    class hand_card_status_entity;

    template<class TStorage>
    class deck_card_status_entity;

    template<class TStorage>
    class support_entity;
    using support_view = support_entity<const detail::table_storage>;

    template<class TStorage>
    class summon_entity;
    using summon_view = summon_entity<const detail::table_storage>;

    template<class TStorage>
    class combat_status_entity;
    using combat_status_view = combat_status_entity<const detail::table_storage>;

    template<class TStorage>
    class character_entity;
    using character_view = character_entity<const detail::table_storage>;

    template<class TStorage>
    class skill_entity;
    using skill_view = skill_entity<const detail::table_storage>;

    template<class TStorage>
    class attachment_entity;
    using attachment_view = attachment_entity<const detail::table_storage>;
}

#endif
