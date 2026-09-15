#ifndef GIVM_DEFINITION_COMMANDS_HPP
#define GIVM_DEFINITION_COMMANDS_HPP

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
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
    template<class TCostEvent> struct onpay_context {};

    struct insert_deck_card
    {
        using context_type = void;

        player_id player;
        definition_id<card_definition> definition;
        std::int32_t position = -1;
    };

    struct enter_character
    {
        using context_type = void;

        player_id player;
        definition_id<character_view> definition;
    };

    struct shuffle_deck
    {
        using context_type = void;

        player_id player;
    };

    struct initialize_characters
    {
        using context_type = void;

        player_id player;
    };

    struct set_active_character
    {
        using context_type = void;

        character_id target;
    };

    struct select_active_character_both
    {
        using context_type = void;

    };

    enum class relative_player : std::uint8_t
    {
        current,
        other
    };

    struct draw_cards
    {
        using context_type = void;

        std::uint32_t count;
        relative_player player = relative_player::current;
    };

    struct replace_cards
    {
        using context_type = void;

        player_id player;
    };

    struct replace_cards_both
    {
        using context_type = void;

    };

    struct start_round
    {
        using context_type = void;

        std::uint32_t max_rounds = 14;
    };

    struct begin_action
    {
        using context_type = void;

    };

    struct end_round
    {
        using context_type = void;
    };

    struct end_game
    {
        using context_type = void;

        game_result result;
    };

    struct start_dice_roll_phase
    {
        using context_type = void;

        std::uint32_t count = 8;
        std::array<std::uint32_t, 2> reroll_count{ 1, 1 };

    };

    struct start_battle
    {
        using context_type = void;
    };

    struct absorb_damage_by_count
    {
        using context_type = damage_effect;

        std::uint32_t maximum_count = std::numeric_limits<std::uint32_t>::max();
    };

    struct deal_damage
    {
        using context_type = void;

        damage_source_id source;
        character_id target;
        std::uint32_t value;
        std::uint16_t multiplier_numerator = 1;
        std::uint16_t multiplier_denominator = 1;
        damage_type type;
        damage_flags flags;
    };

    struct apply_element
    {
        using context_type = void;

        element_application_source_id source;
        character_id target;
        element element;
        element_application_cause cause = element_application_cause::effect;
    };

    struct set_element_aura
    {
        using context_type = void;

        character_id target;
        element_aura aura;
    };

    struct test_command
    {
        using context_type = void;
    };
}

namespace givm::detail
{
    using command_types = type_list<
        insert_deck_card,
        enter_character,
        shuffle_deck,
        initialize_characters,
        set_active_character,
        select_active_character_both,
        draw_cards,
        replace_cards,
        replace_cards_both,
        start_round,
        begin_action,
        end_round,
        end_game,
        start_dice_roll_phase,
        start_battle,
        absorb_damage_by_count,
        deal_damage,
        apply_element,
        set_element_aura,
        test_command>;

    template<class T, class Context>
    struct compatible_command : std::bool_constant<requires {
        typename T::context_type;
        requires (std::same_as<typename T::context_type, void>
            || std::same_as<typename T::context_type, Context>);
    }> {};

    template<class... T, class Context>
    struct compatible_command<std::variant<T...>, Context>
        : std::bool_constant<(compatible_command<T, Context>::value && ...)> {};

    template<class Context>
    using contextual_commands = decltype([]<class... T>(type_list<T...>)
    {
        using types = type_list_cat<
            std::conditional_t<compatible_command<T, Context>::value, type_list<T>, type_list<>>...
        >;
        return std::type_identity<typename types::template apply<std::variant>>{};
    }(command_types{}));
}

namespace givm
{
    template<class TCommand, class TContext>
    concept command_compatible_with = detail::compatible_command<
        std::remove_cvref_t<TCommand>, std::remove_cvref_t<TContext>
    >::value;

    template<class TContext>
    using any_command_for = typename detail::contextual_commands<TContext>::type;
}

#endif
