#ifndef GIVM_DEFINITION_SUBSCRIBED_EVENTS_HPP
#define GIVM_DEFINITION_SUBSCRIBED_EVENTS_HPP

#include "types.hpp"

namespace givm
{
    template<>
    struct subscribed_events<hand_card_view> : type_list<
        struct test_event,
        struct action_phase_started,
        struct battle_started,
        struct before_action,
        struct round_end_declared,
        struct round_ended,
        struct dice_roll_preparation,
        struct calculating_card_payment,
        struct hand_card_created,
        struct card_drawn,
        struct card_discarded,
        struct elemental_tuning_will_apply,
        struct elemental_tuning_applied,
        struct card_will_be_played,
        struct card_played,
        struct active_character_changed,
        struct entity_will_leave,
        struct entity_left
    >{};

    template<>
    struct subscribed_events<deck_card_view> : type_list<
        struct test_event,
        struct action_phase_started,
        struct battle_started,
        struct before_action,
        struct round_end_declared,
        struct round_ended,
        struct dice_roll_preparation,
        struct card_drawn,
        struct card_discarded,
        struct card_played,
        struct active_character_changed,
        struct entity_will_leave,
        struct entity_left
    >{};

    template<>
    struct subscribed_events<hand_card_status_view> : subscribed_events<hand_card_view>{};

    template<>
    struct subscribed_events<deck_card_status_view> : subscribed_events<deck_card_view>{};

    namespace subscribed_events_detail
    {
        template<class TList, class... TExtra>
        struct append;

        template<class... TEvents, class... TExtra>
        struct append<type_list<TEvents...>, TExtra...> : type_list<TEvents..., TExtra...>{};

    }

    using support_subscribed_events = type_list<
        struct test_event,
        struct action_phase_started,
        struct battle_started,
        struct before_action,
        struct round_end_declared,
        struct round_ended,
        struct dice_roll_preparation,
        struct dice_added,
        struct dice_removed,
        struct dice_converted,
        struct changing_secret_points,
        struct secret_points_changed,
        struct changing_energy,
        struct energy_changed,
        struct calculating_card_payment,
        struct calculating_skill_payment,
        struct calculating_switch_payment,
        struct cost_of_switch,
        struct hand_card_created,
        struct card_drawn,
        struct card_discarded,
        struct card_candidate_chosen,
        struct elemental_tuning_will_apply,
        struct elemental_tuning_applied,
        struct card_will_be_played,
        struct card_played,
        struct active_character_changed,
        struct skill_will_be_used,
        struct skill_used,
        struct damage_calculation,
        struct damage_effect,
        struct after_damage,
        struct healing,
        struct healed,
        struct elemental_reaction_will_occur,
        struct after_elemental_reaction,
        struct character_will_be_defeated,
        struct character_defeated,
        struct entity_will_leave,
        struct entity_left,
        struct entity_count_changed
    >;

    template<>
    struct subscribed_events<support_view> : support_subscribed_events{};

    template<>
    struct subscribed_events<summon_view> : subscribed_events<support_view>{};

    template<>
    struct subscribed_events<combat_status_view> : subscribed_events<support_view>{};

    template<>
    struct subscribed_events<character_view>
    : subscribed_events_detail::append<support_subscribed_events, struct character_initialization>{};

    template<>
    struct subscribed_events<skill_view> : subscribed_events<support_view>{};

    template<>
    struct subscribed_events<attachment_view> : subscribed_events<support_view>{};
}

#endif
