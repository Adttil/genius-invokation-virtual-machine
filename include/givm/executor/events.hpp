#ifndef GIVM_EXECUTOR_EVENTS_HPP
#define GIVM_EXECUTOR_EVENTS_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <variant>

#include "../table.hpp"
#include "../enums/action_speed.hpp"
#include "../enums/damage_flags.hpp"
#include "../enums/damage_type.hpp"
#include "../enums/element.hpp"
#include "../enums/element_application_cause.hpp"
#include "../enums/element_aura.hpp"
#include "../enums/elemental_dice.hpp"
#include "../enums/elemental_reaction.hpp"
#include "../enums/skill_kind.hpp"
#include "../definition.hpp"

namespace givm
{
    // For bug in Clang22锛歨ttps://github.com/llvm/llvm-project/issues/59624
#define GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(type) type& operator=(const type&) = delete

    struct test_event
    {
    };

    // Round-flow events.
    struct action_phase_started
    {
    };

    struct battle_started
    {
    };

    struct before_action
    {
    };

    struct round_end_declared
    {
    };

    struct round_ended
    {
    };

    // Dice and fixed-resource events.
    struct dice_roll_preparation
    {
        const std::uint32_t count;
        std::array<dice_counts, 2> fixed_dice{};
        std::array<std::uint32_t, 2> reroll_count{ 1, 1 };
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(dice_roll_preparation);
    };

    struct dice_added
    {
        const player_id player;
        const dice_counts dice;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(dice_added);
    };

    struct dice_removed
    {
        const player_id player;
        const dice_counts dice;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(dice_removed);
    };

    struct dice_converted
    {
        const player_id player;
        const elemental_dice from;
        const elemental_dice to;
        const std::uint8_t count;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(dice_converted);
    };

    struct changing_secret_points
    {
        const player_id player;
        std::int32_t delta;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(changing_secret_points);
    };

    struct secret_points_changed
    {
        const player_id player;
        const std::uint32_t previous;
        const std::uint32_t current;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(secret_points_changed);
    };

    struct changing_energy
    {
        const character_id target;
        std::int32_t delta;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(changing_energy);
    };

    struct energy_changed
    {
        const character_id target;
        const std::uint32_t previous;
        const std::uint32_t current;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(energy_changed);
    };

    // Payment events. The mutable payment data lives in table slots.
    struct calculating_card_payment
    {
        const hand_card_id card;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(calculating_card_payment);
    };

    struct calculating_skill_payment
    {
        const skill_id skill;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(calculating_skill_payment);
    };

    struct calculating_switch_payment
    {
        const character_id target;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(calculating_switch_payment);
    };

    struct elemental_dice_requirement
    {
        dice_counts fixed;
        std::uint8_t same = 0;
        std::uint8_t any = 0;
    };

    struct action_cost_requirement
    {
        elemental_dice_requirement dice_requirement;
        action_speed speed = action_speed::combat;
    };

    template<class TCostEvent>
    struct cost_effect_argument
    {
        elemental_dice_requirement reduced_dice;
    };

    template<class TCostEvent>
    struct onpay_item
    {
        program_entry<onpay_context<TCostEvent>> entry;
        cost_effect_argument<TCostEvent> argument;
    };

    struct cost_of_switch
    {
        character_id target;
        action_cost_requirement requirement;
        cost_effect_argument<cost_of_switch> effect_argument;
    };

    // Card-zone and candidate events.
    struct hand_card_created
    {
        const hand_card_id card;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(hand_card_created);
    };

    struct card_drawn
    {
        const hand_card_id card;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(card_drawn);
    };

    using card_id = std::variant<hand_card_id, deck_card_id>;

    struct card_discarded
    {
        const card_id card;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(card_discarded);
    };

    struct card_candidate_chosen
    {
        const player_id player;
        const definition_id<card_definition> definition_id;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(card_candidate_chosen);
    };

    // Elemental tuning events.
    struct elemental_tuning_will_apply
    {
        const hand_card_id card;
        const elemental_dice from;
        const elemental_dice to;
        bool cancelled = false;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(elemental_tuning_will_apply);
    };

    struct elemental_tuning_applied
    {
        const hand_card_id card;
        const elemental_dice from;
        const elemental_dice to;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(elemental_tuning_applied);
    };

    // Playing-card events.
    using card_target_id = std::variant<std::monostate, character_id, support_id, summon_id>;

    struct card_will_be_played
    {
        const hand_card_id card;
        const definition_id<card_definition> definition_id;
        card_target_id target;
        action_speed speed;
        bool effect_cancelled = false;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(card_will_be_played);
    };

    struct card_played
    {
        const hand_card_id card;
        const definition_id<card_definition> definition_id;
        const card_target_id target;
        const action_speed speed;
        const bool effect_cancelled;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(card_played);
    };

    struct active_character_changed
    {
        const character_id current;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(active_character_changed);
    };

    // Skill events.
    struct skill_will_be_used
    {
        const skill_id skill;
        const skill_kind kind;
        action_speed speed;
        bool effect_cancelled = false;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(skill_will_be_used);
    };

    struct skill_used
    {
        const skill_id skill;
        const skill_kind kind;
        const action_speed speed;
        const bool effect_cancelled;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(skill_used);
    };

    // Damage events.
    using damage_source_id =
        std::variant<hand_card_id, hand_card_status_id, deck_card_status_id, support_id, summon_id,
                     combat_status_id, character_id, skill_id, attachment_id>;

    struct damage_calculation
    {
        damage_source_id source;
        character_id target;
        std::uint32_t value;
        std::uint16_t multiplier_numerator = 1;
        std::uint16_t multiplier_denominator = 1;
        damage_type type;
        damage_flags flags;
        bool already_handled_reaction = false;
    };

    struct damage_effect
    {
        const damage_source_id source;
        const character_id target;
        std::uint32_t value;
        const damage_type type;
        const damage_flags flags;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(damage_effect);
    };

    struct after_damage
    {
        const damage_source_id source;
        const character_id target;
        const std::uint32_t value;
        const damage_type type;
        const damage_flags flags;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(after_damage);
    };

    // Healing events.
    using effect_source_id = std::variant<hand_card_id, deck_card_id, hand_card_status_id, deck_card_status_id,
                                          support_id, summon_id, combat_status_id, character_id, skill_id,
                                          attachment_id>;

    struct healing
    {
        const effect_source_id source;
        const character_id target;
        std::uint32_t value;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(healing);
    };

    struct healed
    {
        const effect_source_id source;
        const character_id target;
        const std::uint32_t value;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(healed);
    };

    // Element application events.
    using element_application_source_id =
        std::variant<hand_card_id, hand_card_status_id, deck_card_status_id, support_id, summon_id,
                     combat_status_id, character_id, skill_id, attachment_id>;

    struct elemental_reaction_will_occur
    {
        const element_application_source_id source;
        const character_id target;
        const element incoming_element;
        const element_aura reacted_aura;
        const elemental_reaction reaction;
        const element_application_cause cause = element_application_cause::effect;
        bool already_handled = false;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(elemental_reaction_will_occur);
    };

    struct after_elemental_reaction
    {
        const element_application_source_id source;
        const character_id target;
        const element incoming_element;
        const element_aura reacted_aura;
        const elemental_reaction reaction;
        const element_application_cause cause = element_application_cause::effect;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(after_elemental_reaction);
    };

    static_assert(sizeof(after_elemental_reaction) <= 64);

    // Defeat events.
    struct character_will_be_defeated
    {
        const character_id target;
        bool prevented = false;
        std::uint32_t revive_health = 0;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(character_will_be_defeated);
    };

    struct character_defeated
    {
        const character_id target;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(character_defeated);
    };

    // Entity events.
    using entity_id = std::variant<hand_card_id, deck_card_id, hand_card_status_id, deck_card_status_id, support_id,
                                   summon_id, combat_status_id, character_id, skill_id, attachment_id>;

    struct entity_will_leave
    {
        const entity_id entity;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(entity_will_leave);
    };

    struct entity_left
    {
        const entity_id entity;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(entity_left);
    };

    struct character_initialization
    {
        character_state state{};
    };

    using counted_entity_id = std::variant<hand_card_status_id, deck_card_status_id, support_id, summon_id,
                                           combat_status_id, attachment_id>;

    struct entity_count_changed
    {
        const counted_entity_id entity;
        const std::uint32_t previous;
        const std::uint32_t current;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(entity_count_changed);
    };

#undef GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND

}

#endif
