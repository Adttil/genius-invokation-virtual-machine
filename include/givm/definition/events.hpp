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
#include "../enums/event_category.hpp"
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

    struct history_summary_initialization
    {
        static constexpr event_category category = event_category::normal;
    };

    // Round-flow events.
    struct action_phase_started
    {
        static constexpr event_category category = event_category::normal;
    };

    struct battle_started
    {
        static constexpr event_category category = event_category::normal;
    };

    struct round_started
    {
        static constexpr event_category category = event_category::normal;
    };

    struct before_action
    {
        static constexpr event_category category = event_category::normal;
    };

    struct round_end_declared
    {
        static constexpr event_category category = event_category::normal;
    };

    struct round_ended
    {
        static constexpr event_category category = event_category::normal;
    };

    // Dice and fixed-resource events.
    struct dice_roll_preparation
    {
        static constexpr event_category category = event_category::immediate;
        const std::uint32_t count;
        std::array<dice_counts, 2> fixed_dice{};
        std::array<std::uint32_t, 2> reroll_count{ 1, 1 };
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(dice_roll_preparation);
    };

    struct dice_added
    {
        static constexpr event_category category = event_category::normal;
        const player_id player;
        const dice_counts dice;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(dice_added);
    };

    struct dice_removed
    {
        static constexpr event_category category = event_category::normal;
        const player_id player;
        const dice_counts dice;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(dice_removed);
    };

    struct dice_converted
    {
        static constexpr event_category category = event_category::normal;
        const player_id player;
        const elemental_dice from;
        const elemental_dice to;
        const std::uint8_t count;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(dice_converted);
    };

    struct changing_secret_points
    {
        static constexpr event_category category = event_category::immediate;
        const player_id player;
        std::int32_t delta;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(changing_secret_points);
    };

    struct secret_points_changed
    {
        static constexpr event_category category = event_category::normal;
        const player_id player;
        const std::uint32_t previous;
        const std::uint32_t current;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(secret_points_changed);
    };

    struct changing_energy
    {
        static constexpr event_category category = event_category::immediate;
        const character_id target;
        std::int32_t delta;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(changing_energy);
    };

    struct energy_changed
    {
        static constexpr event_category category = event_category::normal;
        const character_id target;
        const std::uint32_t previous;
        const std::uint32_t current;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(energy_changed);
    };

    // Action quotes. Empty target slots hold the null category.
    using card_target_id = variant_entity_id<entity_category::null, entity_category::character, entity_category::support, entity_category::summon>;
    using skill_target_id = variant_entity_id<entity_category::null, entity_category::character, entity_category::support, entity_category::summon>;
    using technique_target_id = variant_entity_id<entity_category::null, entity_category::character, entity_category::support, entity_category::summon>;

    struct cost_of_switch
    {
        static constexpr event_category category = event_category::preview;
        const character_id target;
        action_cost_requirement requirement;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(cost_of_switch);
    };

    struct cost_of_card
    {
        static constexpr event_category category = event_category::preview;
        const hand_card_id card;
        const std::array<card_target_id, 2> targets{};
        action_cost_requirement requirement;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(cost_of_card);
    };

    struct cost_of_skill
    {
        static constexpr event_category category = event_category::preview;
        const skill_id skill;
        const skill_flags flags{};
        const std::array<skill_target_id, 2> targets{};
        action_cost_requirement requirement;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(cost_of_skill);
    };

    struct cost_of_technique
    {
        static constexpr event_category category = event_category::preview;
        const attachment_id technique;
        const std::array<technique_target_id, 2> targets{};
        action_cost_requirement requirement;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(cost_of_technique);
    };

    // Card-zone and candidate events.
    struct hand_card_added
    {
        static constexpr event_category category = event_category::normal;
        const hand_card_id card;
        const bool overflow = false;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(hand_card_added);
    };

    struct card_drawn
    {
        static constexpr event_category category = event_category::normal;
        const hand_card_id card;
        const bool overflow = false;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(card_drawn);
    };

    struct this_hand_card_discard
    {
        static constexpr event_category category = event_category::normal;
    };

    struct this_deck_card_discard
    {
        static constexpr event_category category = event_category::normal;
    };

    struct hand_card_discarded
    {
        static constexpr event_category category = event_category::normal;
        const hand_card_id card;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(hand_card_discarded);
    };

    struct deck_card_discarded
    {
        static constexpr event_category category = event_category::normal;
        const deck_card_id card;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(deck_card_discarded);
    };

    struct card_candidate_chosen
    {
        static constexpr event_category category = event_category::normal;
        const player_id player;
        const definition_id<definition_category::card> definition_id;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(card_candidate_chosen);
    };

    // Elemental tuning events.
    struct elemental_tuning_modification
    {
        static constexpr event_category category = event_category::immediate;
        const hand_card_id card;
        const elemental_dice from;
        elemental_dice to;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(elemental_tuning_modification);
    };

    struct elemental_tuning_completed
    {
        static constexpr event_category category = event_category::normal;
        const hand_card_id card;
        const elemental_dice from;
        const elemental_dice to;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(elemental_tuning_completed);
    };

    // Playing-card events.

    struct this_card_play
    {
        static constexpr event_category category = event_category::normal;
        const hand_card_id card;
        const std::array<card_target_id, 2> targets;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(this_card_play);
    };

    struct card_will_be_played
    {
        static constexpr event_category category = event_category::immediate;
        const hand_card_id card;
        const definition_id<definition_category::card> definition_id;
        const std::array<card_target_id, 2> targets;
        const action_speed speed;
        bool effect_cancelled = false;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(card_will_be_played);
    };

    struct card_played
    {
        static constexpr event_category category = event_category::normal;
        const hand_card_id card;
        const definition_id<definition_category::card> definition_id;
        const std::array<card_target_id, 2> targets;
        const action_speed speed;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(card_played);
    };

    struct active_character_changed
    {
        static constexpr event_category category = event_category::normal;
        const character_id current;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(active_character_changed);
    };

    // Skill events.

    struct this_skill_use
    {
        static constexpr event_category category = event_category::normal;
        const skill_id skill;
        const skill_flags flags{};
        const std::array<skill_target_id, 2> targets;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(this_skill_use);
    };

    struct skill_will_be_used
    {
        static constexpr event_category category = event_category::immediate;
        const skill_id skill;
        const skill_flags flags{};
        const std::array<skill_target_id, 2> targets;
        action_speed speed;
        bool effect_cancelled = false;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(skill_will_be_used);
    };

    struct skill_used
    {
        static constexpr event_category category = event_category::normal;
        const skill_id skill;
        const skill_flags flags{};
        const std::array<skill_target_id, 2> targets;
        const action_speed speed;
        const bool effect_cancelled;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(skill_used);
    };

    // Technique events.

    struct this_technique_use
    {
        static constexpr event_category category = event_category::normal;
        const attachment_id technique;
        const std::array<technique_target_id, 2> targets;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(this_technique_use);
    };

    struct technique_will_be_used
    {
        static constexpr event_category category = event_category::immediate;
        const attachment_id technique;
        const std::array<technique_target_id, 2> targets;
        action_speed speed;
        bool effect_cancelled = false;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(technique_will_be_used);
    };

    struct technique_used
    {
        static constexpr event_category category = event_category::normal;
        const attachment_id technique;
        const std::array<technique_target_id, 2> targets;
        const action_speed speed;
        const bool effect_cancelled;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(technique_used);
    };

    struct this_prepared_skill_use
    {
        static constexpr event_category category = event_category::normal;
        const attachment_id attachment;
        action_speed speed = action_speed::combat;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(this_prepared_skill_use);
    };

    // Damage events.
    using damage_source_id =
        variant_entity_id<entity_category::hand_card, entity_category::deck_card, entity_category::hand_card_status, entity_category::deck_card_status, entity_category::support, entity_category::summon, entity_category::combat_status, entity_category::character, entity_category::skill, entity_category::attachment>;

    enum class character_selection : std::uint8_t
    {
        character,
        others,
        all,
        prioritized
    };

    struct relative_character_target
    {
        relative_player player = relative_player::self;
        std::int32_t offset = 0;
        character_selection selection = character_selection::character;
    };

    using damage_target = std::variant<character_id, relative_character_target>;

    struct damage_preparation
    {
        static constexpr event_category category = event_category::immediate;
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
        static constexpr event_category category = event_category::immediate;
        const damage_source_id source;
        const character_id target;
        std::uint32_t value;
        std::uint16_t multiplier_numerator = 1;
        std::uint16_t multiplier_denominator = 1;
        const damage_type type;
        const damage_flags flags;
        const optional_reaction_id reaction{};
        const element_aura reacted_aura = element_aura::none;
        bool cancel_reaction_bonus = false;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(damage_calculation);
    };

    struct damage_effect
    {
        static constexpr event_category category = event_category::immediate;
        const damage_source_id source;
        const character_id target;
        std::uint32_t value;
        const damage_type type;
        const damage_flags flags;
        const optional_reaction_id reaction{};
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(damage_effect);
    };

    struct after_damage
    {
        static constexpr event_category category = event_category::normal;
        const character_id target;
        const std::uint32_t value;
        const damage_type_mask type;
        const damage_flags flags;
        const elemental_reaction_mask reaction{};
        const bool defeated = false;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(after_damage);
    };

    // Healing events.
    using effect_source_id = variant_entity_id<entity_category::hand_card, entity_category::deck_card, entity_category::hand_card_status, entity_category::deck_card_status, entity_category::support, entity_category::summon, entity_category::combat_status, entity_category::character, entity_category::skill, entity_category::attachment>;

    using healing_target = std::variant<character_id, relative_character_target>;

    enum class healing_kind : std::uint8_t
    {
        normal,
        prevent_defeat,
        revive
    };

    struct healing
    {
        static constexpr event_category category = event_category::immediate;
        const effect_source_id source;
        const character_id target;
        std::uint32_t value;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(healing);
    };

    struct healed
    {
        static constexpr event_category category = event_category::normal;
        const effect_source_id source;
        const character_id target;
        const std::uint32_t value;
        const healing_kind kind = healing_kind::normal;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(healed);
    };

    // Element application events.
    using element_application_source_id =
        variant_entity_id<entity_category::hand_card, entity_category::deck_card, entity_category::hand_card_status, entity_category::deck_card_status, entity_category::support, entity_category::summon, entity_category::combat_status, entity_category::character, entity_category::skill, entity_category::attachment>;

    inline player_id source_player(const element_application_source_id& source) noexcept
    {
        return source.player_id();
    }

    struct elemental_reaction_will_occur
    {
        static constexpr event_category category = event_category::immediate;
        const element_application_source_id source;
        const character_id target;
        const element incoming_element;
        const element_aura reacted_aura;
        const reaction_id reaction;
        const element_application_cause cause = element_application_cause::effect;
        element_aura new_aura = element_aura::none;
        bool cancel_default_effects = false;
        player_id source_player() const noexcept { return givm::source_player(source); }
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(elemental_reaction_will_occur);
    };

    struct after_elemental_reaction
    {
        static constexpr event_category category = event_category::normal;
        const element_application_source_id source;
        const character_id target;
        const element incoming_element;
        const element_aura reacted_aura;
        const reaction_id reaction;
        const element_application_cause cause = element_application_cause::effect;
        player_id source_player() const noexcept { return givm::source_player(source); }
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(after_elemental_reaction);
    };

    // Defeat events.
    struct character_will_be_defeated
    {
        static constexpr event_category category = event_category::immediate;
        const character_id target;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(character_will_be_defeated);
    };

    struct character_revived
    {
        static constexpr event_category category = event_category::normal;
        const character_id target;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(character_revived);
    };

    // Entity events.
    struct this_support_state_change
    {
        static constexpr event_category category = event_category::normal;
        const support_state previous;
        const support_state current;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(this_support_state_change);
    };

    struct this_support_remove
    {
        static constexpr event_category category = event_category::normal;
    };

    struct support_removed
    {
        static constexpr event_category category = event_category::normal;
        const support_id support;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(support_removed);
    };

    struct this_summon_resummon
    {
        static constexpr event_category category = event_category::normal;
        const summon_state state;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(this_summon_resummon);
    };

    struct this_summon_remove
    {
        static constexpr event_category category = event_category::normal;
    };

    struct summon_removed
    {
        static constexpr event_category category = event_category::normal;
        const summon_id summon;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(summon_removed);
    };

    struct this_combat_status_regenerate
    {
        static constexpr event_category category = event_category::normal;
        const combat_status_state state;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(this_combat_status_regenerate);
    };

    struct this_combat_status_state_change
    {
        static constexpr event_category category = event_category::normal;
        const combat_status_state previous;
        const combat_status_state current;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(this_combat_status_state_change);
    };

    struct this_combat_status_remove
    {
        static constexpr event_category category = event_category::normal;
    };

    struct combat_status_removed
    {
        static constexpr event_category category = event_category::normal;
        const combat_status_id status;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(combat_status_removed);
    };

    struct this_attachment_reapply
    {
        static constexpr event_category category = event_category::normal;
        const attachment_state state;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(this_attachment_reapply);
    };

    struct this_attachment_state_change
    {
        static constexpr event_category category = event_category::normal;
        const attachment_state previous;
        const attachment_state current;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(this_attachment_state_change);
    };

    struct this_attachment_remove
    {
        static constexpr event_category category = event_category::normal;
    };

    struct attachment_removed
    {
        static constexpr event_category category = event_category::normal;
        const attachment_id attachment;
        GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND(attachment_removed);
    };

#undef GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND

}

#endif
