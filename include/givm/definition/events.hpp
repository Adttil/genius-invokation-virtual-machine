#ifndef GIVM_DEFINITION_EVENTS_HPP
#define GIVM_DEFINITION_EVENTS_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <variant>

#include "../table.hpp"
#include "../enums/action_speed.hpp"
#include "../enums/damage_flags.hpp"
#include "../enums/skill_flags.hpp"
#include "../enums/damage_type.hpp"
#include "../enums/element.hpp"
#include "../enums/element_application_cause.hpp"
#include "../enums/element_aura.hpp"
#include "../enums/elemental_dice.hpp"
#include "../enums/elemental_reaction.hpp"
#include "../enums/relative_player.hpp"

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

    struct round_started
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

    struct energy_change
    {
        character_id target;
        std::uint32_t value;
    };

    struct energy_modification
    {
        character_id target;
        std::int64_t delta{};
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
        const skill_flags flags{};
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(calculating_skill_payment);
    };

    struct calculating_switch_payment
    {
        const character_id target;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(calculating_switch_payment);
    };

    struct cost_of_switch
    {
        const character_id target;
        action_cost_requirement requirement;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(cost_of_switch);
    };

    struct cost_of_card
    {
        const hand_card_id card;
        action_cost_requirement requirement;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(cost_of_card);
    };

    struct cost_of_skill
    {
        const skill_id skill;
        const skill_flags flags{};
        action_cost_requirement requirement;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(cost_of_skill);
    };

    struct cost_of_technique
    {
        const attachment_id technique;
        action_cost_requirement requirement;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(cost_of_technique);
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

    struct hand_card_discard_effect
    {
        const hand_card_id card;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(hand_card_discard_effect);
    };

    struct deck_card_discard
    {
        const player_id player;
        const std::uint32_t count;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(deck_card_discard);
    };

    struct deck_card_discard_effect
    {
        const deck_card_id card;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(deck_card_discard_effect);
    };

    struct hand_card_discarded
    {
        const hand_card_id card;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(hand_card_discarded);
    };

    struct deck_card_discarded
    {
        const deck_card_id card;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(deck_card_discarded);
    };

    struct card_candidate_chosen
    {
        const player_id player;
        const definition_id<card_definition> definition_id;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(card_candidate_chosen);
    };

    // Elemental tuning events.
    struct elemental_tuning_modification
    {
        const hand_card_id card;
        const elemental_dice from;
        elemental_dice to;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(elemental_tuning_modification);
    };

    struct elemental_tuning_completed
    {
        const hand_card_id card;
        const elemental_dice from;
        const elemental_dice to;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(elemental_tuning_completed);
    };

    // Playing-card events.
    using card_target_id = std::variant<std::monostate, character_id, support_id, summon_id>;

    struct card_effect
    {
        const hand_card_id card;
        const std::array<card_target_id, 2> targets;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(card_effect);
    };

    struct card_will_be_played
    {
        const hand_card_id card;
        const definition_id<card_definition> definition_id;
        const std::array<card_target_id, 2> targets;
        const action_speed speed;
        bool effect_cancelled = false;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(card_will_be_played);
    };

    struct card_played
    {
        const hand_card_id card;
        const definition_id<card_definition> definition_id;
        const std::array<card_target_id, 2> targets;
        const action_speed speed;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(card_played);
    };

    struct active_character_changed
    {
        const character_id current;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(active_character_changed);
    };

    // Skill events.
    using skill_target_id = std::variant<std::monostate, character_id, support_id, summon_id>;

    struct skill_state_change
    {
        skill_id skill;
        skill_state state;
    };

    struct skill_effect
    {
        const skill_id skill;
        const skill_flags flags{};
        const std::array<skill_target_id, 2> targets;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(skill_effect);
    };

    struct skill_will_be_used
    {
        const skill_id skill;
        const skill_flags flags{};
        const std::array<skill_target_id, 2> targets;
        action_speed speed;
        bool effect_cancelled = false;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(skill_will_be_used);
    };

    struct skill_used
    {
        const skill_id skill;
        const skill_flags flags{};
        const std::array<skill_target_id, 2> targets;
        const action_speed speed;
        const bool effect_cancelled;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(skill_used);
    };

    // Technique events.
    using technique_target_id = std::variant<std::monostate, character_id, support_id, summon_id>;

    struct technique_effect
    {
        const attachment_id technique;
        const std::array<technique_target_id, 2> targets;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(technique_effect);
    };

    struct technique_will_be_used
    {
        const attachment_id technique;
        const std::array<technique_target_id, 2> targets;
        action_speed speed;
        bool effect_cancelled = false;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(technique_will_be_used);
    };

    struct technique_used
    {
        const attachment_id technique;
        const std::array<technique_target_id, 2> targets;
        const action_speed speed;
        const bool effect_cancelled;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(technique_used);
    };

    struct prepared_skill_effect
    {
        const attachment_id attachment;
        action_speed speed = action_speed::combat;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(prepared_skill_effect);
    };

    // Damage events.
    using damage_source_id =
        std::variant<hand_card_id, deck_card_id, hand_card_status_id, deck_card_status_id, support_id, summon_id,
                     combat_status_id, character_id, skill_id, attachment_id>;

    enum class character_selection : std::uint8_t
    {
        character,
        others,
        all
    };

    struct relative_character_target
    {
        relative_player player = relative_player::self;
        std::int32_t offset = 0;
        character_selection selection = character_selection::character;
    };

    using damage_target = std::variant<character_id, relative_character_target>;

    struct damage
    {
        damage_source_id source;
        damage_target target;
        character_selection selection = character_selection::character;
        std::uint32_t value;
        std::uint16_t multiplier_numerator = 1;
        std::uint16_t multiplier_denominator = 1;
        damage_type type;
        damage_flags flags;
    };

    struct damage_preparation
    {
        damage_source_id source;
        character_id target;
        const std::uint32_t value;
        const std::uint16_t multiplier_numerator = 1;
        const std::uint16_t multiplier_denominator = 1;
        damage_type type;
        damage_flags flags;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(damage_preparation);
    };

    struct damage_calculation
    {
        const damage_source_id source;
        const character_id target;
        std::uint32_t value;
        std::uint16_t multiplier_numerator = 1;
        std::uint16_t multiplier_denominator = 1;
        const damage_type type;
        const damage_flags flags;
        const elemental_reaction reaction = elemental_reaction::none;
        const element_aura reacted_aura = element_aura::none;
        const tag_id replacement_reaction{};
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(damage_calculation);
    };

    struct damage_effect
    {
        const damage_source_id source;
        const character_id target;
        std::uint32_t value;
        const damage_type type;
        const damage_flags flags;
        const elemental_reaction reaction = elemental_reaction::none;
        const tag_id replacement_reaction{};
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(damage_effect);
    };

    struct after_damage
    {
        const damage_source_id source;
        const character_id target;
        const std::uint32_t value;
        const damage_type type;
        const damage_flags flags;
        const elemental_reaction reaction = elemental_reaction::none;
        const tag_id replacement_reaction{};
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(after_damage);
    };

    // Healing events.
    using effect_source_id = std::variant<hand_card_id, deck_card_id, hand_card_status_id, deck_card_status_id,
                                          support_id, summon_id, combat_status_id, character_id, skill_id,
                                          attachment_id>;

    using healing_target = std::variant<character_id, relative_character_target>;

    struct healing_application
    {
        effect_source_id source;
        healing_target target;
        std::uint32_t value;
    };

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
        std::variant<hand_card_id, deck_card_id, hand_card_status_id, deck_card_status_id, support_id, summon_id,
                     combat_status_id, character_id, skill_id, attachment_id>;

    struct element_application
    {
        element_application_source_id source;
        character_id target;
        element element;
        element_application_cause cause = element_application_cause::effect;
    };

    struct elemental_reaction_will_occur
    {
        const element_application_source_id source;
        const character_id target;
        const element incoming_element;
        const element_aura reacted_aura;
        const elemental_reaction reaction;
        const element_application_cause cause = element_application_cause::effect;
        tag_id replacement_reaction{};
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
        const tag_id replacement_reaction{};
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(after_elemental_reaction);
    };

    static_assert(sizeof(after_elemental_reaction) <= 64);

    // Defeat events.
    struct character_will_be_defeated
    {
        const character_id target;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(character_will_be_defeated);
    };

    struct character_defeated
    {
        const character_id target;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(character_defeated);
    };

    // Entity events.
    struct support_addition
    {
        player_id player;
        definition_id<support_view> definition;
        support_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct support_state_change
    {
        support_id support;
        support_state state;
    };

    struct support_state_modification
    {
        support_id support;
        std::int64_t count{};
        std::int64_t round_usages{};
    };

    struct support_state_changed
    {
        const support_state previous;
        const support_state current;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(support_state_changed);
    };

    struct support_removal
    {
        support_id support;
    };

    struct support_removed
    {
        const support_id support;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(support_removed);
    };

    struct summoning
    {
        player_id player;
        definition_id<summon_view> definition;
        summon_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct summon_addition
    {
        player_id player;
        definition_id<summon_view> definition;
        summon_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct resummoning
    {
        const summon_state state;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(resummoning);
    };

    struct summon_state_change
    {
        summon_id summon;
        summon_state state;
    };

    struct summon_state_modification
    {
        summon_id summon;
        std::int64_t value{};
        std::int64_t usages{};
    };

    struct summon_state_changed
    {
        const summon_state previous;
        const summon_state current;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(summon_state_changed);
    };

    struct summon_removal
    {
        summon_id summon;
    };

    struct summon_removed
    {
        const summon_id summon;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(summon_removed);
    };

    struct combat_status_generation
    {
        player_id player;
        definition_id<combat_status_view> definition;
        combat_status_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct combat_status_addition
    {
        player_id player;
        definition_id<combat_status_view> definition;
        combat_status_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct combat_status_regeneration
    {
        const combat_status_state state;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(combat_status_regeneration);
    };

    struct combat_status_state_change
    {
        combat_status_id status;
        combat_status_state state;
    };

    struct combat_status_state_modification
    {
        combat_status_id status;
        std::int64_t count{};
        std::int64_t round_usages{};
    };

    struct combat_status_state_changed
    {
        const combat_status_state previous;
        const combat_status_state current;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(combat_status_state_changed);
    };

    struct combat_status_removal
    {
        combat_status_id status;
    };

    struct combat_status_removed
    {
        const combat_status_id status;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(combat_status_removed);
    };

    struct attachment_application
    {
        character_id target;
        definition_id<attachment_view> definition;
        attachment_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct attachment_reapplication
    {
        const attachment_state state;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(attachment_reapplication);
    };

    struct attachment_state_change
    {
        attachment_id attachment;
        attachment_state state;
    };

    struct attachment_state_modification
    {
        attachment_id attachment;
        std::int64_t count{};
        std::int64_t round_usages{};
    };

    struct attachment_state_changed
    {
        const attachment_state previous;
        const attachment_state current;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(attachment_state_changed);
    };

    struct attachment_addition
    {
        character_id target;
        definition_id<attachment_view> definition;
        attachment_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct attachment_removal
    {
        attachment_id attachment;
    };

    struct attachment_removed
    {
        const attachment_id attachment;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(attachment_removed);
    };

#undef GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND

}

#endif
