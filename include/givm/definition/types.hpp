#ifndef GIVM_DEFINITION_TYPES_HPP
#define GIVM_DEFINITION_TYPES_HPP

#include <any>

#include "../utils/type_list.hpp"

namespace givm
{
    namespace detail
    {
        struct table_storage;
    }

    template<class TStorage>
    class player_entity;
    using player_view = player_entity<const detail::table_storage>;

    struct card_definition{};
    struct status_definition{};

    template<class TStorage>
    class hand_card_entity;
    using hand_card_view = hand_card_entity<const detail::table_storage>;

    template<class TStorage>
    class deck_card_entity;
    using deck_card_view = deck_card_entity<const detail::table_storage>;

    template<class TStorage>
    class hand_card_status_entity;
    using hand_card_status_view = hand_card_status_entity<const detail::table_storage>;

    template<class TStorage>
    class deck_card_status_entity;
    using deck_card_status_view = deck_card_status_entity<const detail::table_storage>;

    template<class TStorage, class TStatusEntity, class TOwnerId>
    class card_status_range;

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

    template<class TContext>
    class program_entry;

    template<class TCostEvent>
    struct onpay_context;

    struct cost_of_switch;

    template<class TEvent>
    struct handler_program_context
    {
        using type = TEvent;
    };

    template<>
    struct handler_program_context<cost_of_switch>
    {
        using type = onpay_context<cost_of_switch>;
    };

    template<class TEvent>
    using handler_program_context_t = typename handler_program_context<TEvent>::type;

    template<class TEvent>
    using handler_program_entry_t = program_entry<handler_program_context_t<TEvent>>;

    using definition_data = std::any;

    template<class TEntity, class TEvent>
    using handle_fn_t = handler_program_entry_t<TEvent> (*)(
        const definition_data&,
        const TEntity&,
        TEvent&,
        const class card_table&,
        class random_fn&
    );

    template<class EntityView>
    struct subscribed_events;

    template<class TDefinition>
    struct views_of_definition : type_list<TDefinition>{};

    template<>
    struct views_of_definition<card_definition> : type_list<
        hand_card_view,
        deck_card_view
    >{};

    template<>
    struct views_of_definition<status_definition> : type_list<
        hand_card_status_view,
        deck_card_status_view
    >{};

    using definition_types = type_list<
        card_definition,
        status_definition,
        support_view,
        summon_view,
        combat_status_view,
        character_view,
        skill_view,
        attachment_view
    >;
}

#endif
