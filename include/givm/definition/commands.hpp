#ifndef GIVM_DEFINITION_COMMANDS_HPP
#define GIVM_DEFINITION_COMMANDS_HPP

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>
#include <variant>

#include "../table.hpp"
#include "../utils/type_list.hpp"
#include "events.hpp"
#include "../enums/damage_flags.hpp"
#include "../enums/damage_type.hpp"
#include "../enums/element_application_cause.hpp"
#include "../enums/game_result.hpp"

namespace givm
{
    struct insert_deck_card
    {
        player_id player;
        definition_id<card_definition> definition;
        std::int32_t position = -1;
    };

    struct enter_character
    {
        player_id player;
        definition_id<character_view> definition;
    };

    struct shuffle_deck
    {
        player_id player;
    };

    using set_active_character_input = active_character_changed;

    struct set_active_character
    {
        using input_type = set_active_character_input;

        relative_character_target target{ {}, std::numeric_limits<std::int32_t>::max() };
    };

    struct select_active_character_both
    {
    };

    struct draw_cards
    {
        std::uint32_t count;
        relative_player player = relative_player::self;
    };

    struct create_hand_card_input
    {
        player_id player;
        definition_id<card_definition> definition;
    };

    struct create_hand_card
    {
        using input_type = create_hand_card_input;

        relative_player player = relative_player::self;
        definition_id<card_definition> definition{};
    };

    using discard_hand_card_input = hand_card_discard_effect;

    struct discard_hand_card
    {
        using input_type = discard_hand_card_input;

        relative_player player = relative_player::self;
        definition_id<card_definition> definition{};
    };

    struct discard_deck_cards_input
    {
        player_id player;
        std::uint32_t count;
    };

    struct discard_deck_cards
    {
        using input_type = discard_deck_cards_input;

        std::uint32_t count = std::numeric_limits<std::uint32_t>::max();
        relative_player player = relative_player::self;
    };

    struct add_support_input
    {
        player_id player;
        definition_id<support_view> definition;
        support_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct add_support
    {
        using input_type = add_support_input;

        relative_player player = relative_player::self;
        definition_id<support_view> definition{};
        support_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct set_support_state_input
    {
        support_id support;
        support_state state;
    };

    struct set_support_state
    {
        using input_type = set_support_state_input;

        relative_player player = relative_player::self;
        definition_id<support_view> definition{};
        support_state state{};
    };

    struct modify_support_state_input
    {
        support_id support;
        std::int64_t count{};
        std::int64_t round_usages{};
    };

    struct modify_support_state
    {
        using input_type = modify_support_state_input;

        relative_player player = relative_player::self;
        definition_id<support_view> definition{};
        std::int64_t count{};
        std::int64_t round_usages{};
    };

    struct remove_support_input
    {
        support_id support;
    };

    struct remove_support
    {
        using input_type = remove_support_input;

        relative_player player = relative_player::self;
        definition_id<support_view> definition{};
    };

    struct summon_input
    {
        player_id player;
        definition_id<summon_view> definition;
        summon_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct summon
    {
        using input_type = summon_input;

        relative_player player = relative_player::self;
        definition_id<summon_view> definition{};
        summon_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct add_summon_input
    {
        player_id player;
        definition_id<summon_view> definition;
        summon_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct add_summon
    {
        using input_type = add_summon_input;

        relative_player player = relative_player::self;
        definition_id<summon_view> definition{};
        summon_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct set_summon_state_input
    {
        summon_id summon;
        summon_state state;
    };

    struct set_summon_state
    {
        using input_type = set_summon_state_input;

        relative_player player = relative_player::self;
        definition_id<summon_view> definition{};
        summon_state state{};
    };

    struct modify_summon_state_input
    {
        summon_id summon;
        std::int64_t value{};
        std::int64_t usages{};
    };

    struct modify_summon_state
    {
        using input_type = modify_summon_state_input;

        relative_player player = relative_player::self;
        definition_id<summon_view> definition{};
        std::int64_t value{};
        std::int64_t usages{};
    };

    struct remove_summon_input
    {
        summon_id summon;
    };

    struct remove_summon
    {
        using input_type = remove_summon_input;

        relative_player player = relative_player::self;
        definition_id<summon_view> definition{};
    };

    struct generate_combat_status_input
    {
        player_id player;
        definition_id<combat_status_view> definition;
        combat_status_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct generate_combat_status
    {
        using input_type = generate_combat_status_input;

        relative_player player = relative_player::self;
        definition_id<combat_status_view> definition{};
        combat_status_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct add_combat_status_input
    {
        player_id player;
        definition_id<combat_status_view> definition;
        combat_status_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct add_combat_status
    {
        using input_type = add_combat_status_input;

        relative_player player = relative_player::self;
        definition_id<combat_status_view> definition{};
        combat_status_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct set_combat_status_state_input
    {
        combat_status_id status;
        combat_status_state state;
    };

    struct set_combat_status_state
    {
        using input_type = set_combat_status_state_input;

        relative_player player = relative_player::self;
        definition_id<combat_status_view> definition{};
        combat_status_state state{};
    };

    struct modify_combat_status_state_input
    {
        combat_status_id status;
        std::int64_t count{};
        std::int64_t round_usages{};
    };

    struct modify_combat_status_state
    {
        using input_type = modify_combat_status_state_input;

        relative_player player = relative_player::self;
        definition_id<combat_status_view> definition{};
        std::int64_t count{};
        std::int64_t round_usages{};
    };

    struct remove_combat_status_input
    {
        combat_status_id status;
    };

    struct remove_combat_status
    {
        using input_type = remove_combat_status_input;

        relative_player player = relative_player::self;
        definition_id<combat_status_view> definition{};
    };

    struct attach_input
    {
        character_id target;
        definition_id<attachment_view> definition;
        attachment_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct attach
    {
        using input_type = attach_input;

        relative_player player = relative_player::self;
        definition_id<attachment_view> definition{};
        attachment_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct set_attachment_state_input
    {
        attachment_id attachment;
        attachment_state state;
    };

    struct set_attachment_state
    {
        using input_type = set_attachment_state_input;

        relative_player player = relative_player::self;
        definition_id<attachment_view> definition{};
        attachment_state state{};
    };

    struct modify_attachment_state_input
    {
        attachment_id attachment;
        std::int64_t count{};
        std::int64_t round_usages{};
    };

    struct modify_attachment_state
    {
        using input_type = modify_attachment_state_input;

        relative_player player = relative_player::self;
        definition_id<attachment_view> definition{};
        std::int64_t count{};
        std::int64_t round_usages{};
    };

    struct add_attachment_input
    {
        character_id target;
        definition_id<attachment_view> definition;
        attachment_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct add_attachment
    {
        using input_type = add_attachment_input;

        relative_player player = relative_player::self;
        definition_id<attachment_view> definition{};
        attachment_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct remove_attachment_input
    {
        attachment_id attachment;
    };

    struct remove_attachment
    {
        using input_type = remove_attachment_input;

        relative_player player = relative_player::self;
        definition_id<attachment_view> definition{};
    };

    struct replace_cards
    {
        player_id player;
    };

    struct replace_cards_both
    {
    };

    struct start_round
    {
    };

    struct begin_action
    {
    };

    using use_skill_input = skill_effect;

    struct use_skill
    {
        using input_type = use_skill_input;

        relative_player player = relative_player::self;
        definition_id<skill_view> definition{};
    };

    struct set_skill_state_input
    {
        skill_id skill;
        skill_state state;
    };

    struct set_skill_state
    {
        using input_type = set_skill_state_input;

        relative_character_target character{};
        definition_id<skill_view> definition{};
        skill_state state{};
    };

    struct set_energy_input
    {
        character_id target;
        std::uint32_t value;
    };

    struct set_energy
    {
        using input_type = set_energy_input;

        relative_character_target target{ {}, std::numeric_limits<std::int32_t>::max() };
        std::uint32_t value{};
    };

    struct modify_energy_input
    {
        character_id target;
        std::int64_t delta{};
    };

    struct modify_energy
    {
        using input_type = modify_energy_input;

        relative_character_target target{ {}, std::numeric_limits<std::int32_t>::max() };
        std::int64_t delta{};
    };

    struct end_round
    {
    };

    struct end_game
    {
        game_result result;
    };

    struct start_dice_roll_phase
    {
        std::uint32_t count = 8;
        std::array<std::uint32_t, 2> reroll_count{ 1, 1 };

    };

    using add_dice_input = dice_added;

    struct add_dice
    {
        using input_type = add_dice_input;

        relative_player player = static_cast<relative_player>(-1);
        dice_counts dice{};
    };

    struct start_battle
    {
    };

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

    struct fixed_damage
    {
        relative_character_target source;
        relative_character_target target{ relative_player::opponent };
        std::uint32_t value;
        std::uint16_t multiplier_numerator = 1;
        std::uint16_t multiplier_denominator = 1;
        damage_type type;
        damage_flags flags;
    };

    struct deal_damage_input
    {
        std::span<const damage> damages;
    };

    struct deal_damage
    {
        using input_type = deal_damage_input;

        std::span<const fixed_damage> damages{};
    };

    struct apply_element_input
    {
        element_application_source_id source;
        character_id target;
        element element;
        element_application_cause cause = element_application_cause::effect;
    };

    struct apply_element
    {
        using input_type = apply_element_input;

        relative_character_target source{};
        relative_character_target target{ {}, std::numeric_limits<std::int32_t>::max() };
        element element;
        element_application_cause cause = element_application_cause::effect;
    };

    struct heal_input
    {
        effect_source_id source;
        healing_target target;
        std::uint32_t value;
    };

    struct heal
    {
        using input_type = heal_input;

        relative_character_target source{};
        relative_character_target target{ {}, std::numeric_limits<std::int32_t>::max() };
        std::uint32_t value{};
    };

    using increase_max_health_input = healing;

    struct increase_max_health
    {
        using input_type = increase_max_health_input;

        relative_character_target source{};
        relative_character_target target{ {}, std::numeric_limits<std::int32_t>::max() };
        std::uint32_t value{};
    };

    struct test_command
    {
    };
}

namespace givm::detail
{
    using command_types = type_list<
        insert_deck_card,
        enter_character,
        shuffle_deck,
        set_active_character,
        select_active_character_both,
        draw_cards,
        create_hand_card,
        discard_hand_card,
        discard_deck_cards,
        add_support,
        set_support_state,
        modify_support_state,
        remove_support,
        summon,
        add_summon,
        set_summon_state,
        modify_summon_state,
        remove_summon,
        generate_combat_status,
        add_combat_status,
        set_combat_status_state,
        modify_combat_status_state,
        remove_combat_status,
        attach,
        set_attachment_state,
        modify_attachment_state,
        add_attachment,
        remove_attachment,
        replace_cards,
        replace_cards_both,
        start_round,
        begin_action,
        use_skill,
        set_skill_state,
        set_energy,
        modify_energy,
        end_round,
        end_game,
        start_dice_roll_phase,
        add_dice,
        start_battle,
        deal_damage,
        apply_element,
        heal,
        increase_max_health,
        test_command>;

    using command_input_types = decltype([]<class... T>(type_list<T...>)
    {
        return type_list_cat<decltype([]
        {
            if constexpr(requires { typename T::input_type; })
                return type_list<typename T::input_type>{};
            else
                return type_list<>{};
        }())...>{};
    }(command_types{}));
}

namespace givm
{
    using any_command = detail::command_types::apply<std::variant>;
    using any_command_input = detail::command_input_types::apply<std::variant>;
}

#ifndef NDEBUG
namespace givm::detail
{
    template<class T>
    requires (not requires { typename T::input_type; })
    constexpr size_t input_marker(const T&) noexcept { return size_t(-1); }

    constexpr size_t input_marker(const set_active_character& command) noexcept
    {
        return command.target.offset == std::numeric_limits<std::int32_t>::max() ? command_input_types::index_of<set_active_character::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const create_hand_card& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<create_hand_card::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const discard_hand_card& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<discard_hand_card::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const discard_deck_cards& command) noexcept
    {
        return command.count == std::numeric_limits<std::uint32_t>::max() ? command_input_types::index_of<discard_deck_cards::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const add_support& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<add_support::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const set_support_state& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<set_support_state::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const modify_support_state& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<modify_support_state::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const remove_support& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<remove_support::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const add_attachment& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<add_attachment::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const summon& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<summon::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const add_summon& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<add_summon::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const set_summon_state& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<set_summon_state::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const modify_summon_state& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<modify_summon_state::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const remove_summon& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<remove_summon::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const generate_combat_status& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<generate_combat_status::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const add_combat_status& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<add_combat_status::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const set_combat_status_state& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<set_combat_status_state::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const modify_combat_status_state& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<modify_combat_status_state::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const remove_combat_status& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<remove_combat_status::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const attach& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<attach::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const set_attachment_state& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<set_attachment_state::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const modify_attachment_state& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<modify_attachment_state::input_type>() : size_t(-1);
    }

    constexpr size_t input_marker(const remove_attachment& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<remove_attachment::input_type>() : size_t(-1);
    }
    constexpr size_t input_marker(const use_skill& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<use_skill::input_type>() : size_t(-1);
    }
    constexpr size_t input_marker(const set_skill_state& command) noexcept
    {
        return not command.definition ? command_input_types::index_of<set_skill_state::input_type>() : size_t(-1);
    }
    constexpr size_t input_marker(const set_energy& command) noexcept
    {
        return command.target.offset == std::numeric_limits<std::int32_t>::max() ? command_input_types::index_of<set_energy::input_type>() : size_t(-1);
    }
    constexpr size_t input_marker(const modify_energy& command) noexcept
    {
        return command.target.offset == std::numeric_limits<std::int32_t>::max() ? command_input_types::index_of<modify_energy::input_type>() : size_t(-1);
    }
    constexpr size_t input_marker(const add_dice& command) noexcept
    {
        return command.player == static_cast<relative_player>(-1) ? command_input_types::index_of<add_dice::input_type>() : size_t(-1);
    }
    constexpr size_t input_marker(const deal_damage& command) noexcept
    {
        return command.damages.empty() ? command_input_types::index_of<deal_damage::input_type>() : size_t(-1);
    }
    constexpr size_t input_marker(const apply_element& command) noexcept
    {
        return command.target.offset == std::numeric_limits<std::int32_t>::max() ? command_input_types::index_of<apply_element::input_type>() : size_t(-1);
    }
    constexpr size_t input_marker(const heal& command) noexcept
    {
        return command.target.offset == std::numeric_limits<std::int32_t>::max() ? command_input_types::index_of<heal::input_type>() : size_t(-1);
    }
    constexpr size_t input_marker(const increase_max_health& command) noexcept
    {
        return command.target.offset == std::numeric_limits<std::int32_t>::max() ? command_input_types::index_of<increase_max_health::input_type>() : size_t(-1);
    }
}
#endif

#endif
