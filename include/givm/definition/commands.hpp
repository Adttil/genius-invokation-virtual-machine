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

    struct set_active_character
    {
        character_id target{ {}, std::numeric_limits<size_t>::max() };
    };

    struct select_active_character_both
    {
    };

    enum class relative_player : std::uint8_t
    {
        current,
        other
    };

    struct draw_cards
    {
        std::uint32_t count;
        relative_player player = relative_player::current;
    };

    struct add_attachment
    {
        relative_player player = relative_player::current;
        definition_id<attachment_view> definition{};
        attachment_state state{};
    };

    struct remove_attachment
    {
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
        std::uint32_t max_rounds = 14;
    };

    struct begin_action
    {
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

    struct start_battle
    {
    };

    struct reduce_combat_status_count
    {
    };

    struct deal_damage
    {
        std::span<const damage> damages{};
        std::size_t input_count = 1;
    };

    struct apply_element
    {
        element_application_source_id source;
        character_id target;
        element element;
        element_application_cause cause = element_application_cause::effect;
    };

    struct set_element_aura
    {
        character_id target;
        element_aura aura;
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
        add_attachment,
        remove_attachment,
        replace_cards,
        replace_cards_both,
        start_round,
        begin_action,
        end_round,
        end_game,
        start_dice_roll_phase,
        start_battle,
        reduce_combat_status_count,
        deal_damage,
        apply_element,
        set_element_aura,
        test_command>;

}

namespace givm
{
    using any_command = detail::command_types::apply<std::variant>;
}

#ifndef NDEBUG
namespace givm::detail
{
    constexpr size_t input_size(const insert_deck_card&) noexcept { return 0; }
    constexpr size_t input_size(const enter_character&) noexcept { return 0; }
    constexpr size_t input_size(const shuffle_deck&) noexcept { return 0; }

    constexpr size_t input_size(const set_active_character& command) noexcept
    {
        return command.target.index == std::numeric_limits<size_t>::max() ? sizeof(active_character_changed) : 0;
    }

    constexpr size_t input_size(const select_active_character_both&) noexcept { return 0; }
    constexpr size_t input_size(const draw_cards&) noexcept { return 0; }

    constexpr size_t input_size(const add_attachment& command) noexcept
    {
        return command.definition ? 0 : sizeof(attachment_addition);
    }

    constexpr size_t input_size(const remove_attachment&) noexcept { return sizeof(attachment_removal); }
    constexpr size_t input_size(const replace_cards&) noexcept { return 0; }
    constexpr size_t input_size(const replace_cards_both&) noexcept { return 0; }
    constexpr size_t input_size(const start_round&) noexcept { return 0; }
    constexpr size_t input_size(const begin_action&) noexcept { return 0; }
    constexpr size_t input_size(const end_round&) noexcept { return 0; }
    constexpr size_t input_size(const end_game&) noexcept { return 0; }
    constexpr size_t input_size(const start_dice_roll_phase&) noexcept { return 0; }
    constexpr size_t input_size(const start_battle&) noexcept { return 0; }
    constexpr size_t input_size(const reduce_combat_status_count&) noexcept { return sizeof(combat_status_count_reduction); }
    constexpr size_t input_size(const deal_damage& command) noexcept
    {
        constexpr auto alignment = alignof(std::max_align_t);
        return command.damages.empty()
            ? command.input_count * ((sizeof(damage) + alignment - 1) / alignment * alignment) : 0;
    }
    constexpr size_t input_size(const apply_element&) noexcept { return 0; }
    constexpr size_t input_size(const set_element_aura&) noexcept { return 0; }
    constexpr size_t input_size(const test_command&) noexcept { return 0; }
}
#endif

#endif
