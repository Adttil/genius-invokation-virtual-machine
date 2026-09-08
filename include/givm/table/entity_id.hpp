#ifndef GIVM_TABLE_ENTITY_ID_HPP
#define GIVM_TABLE_ENTITY_ID_HPP

#include <cstddef>

namespace givm
{
    struct player_id
    {
        size_t index;

        friend constexpr bool operator==(player_id, player_id) = default;
    };

    constexpr player_id other_player(player_id player) noexcept
    {
        return { player.index ^ 1uz };
    }

    struct hand_card_id
    {
        player_id player_id;
        size_t index;

        friend constexpr bool operator==(hand_card_id, hand_card_id) = default;
    };

    struct deck_card_id
    {
        player_id player_id;
        size_t index;

        friend constexpr bool operator==(deck_card_id, deck_card_id) = default;
    };

    struct hand_card_status_id
    {
        hand_card_id card_id;
        size_t index;

        friend constexpr bool operator==(hand_card_status_id, hand_card_status_id) = default;
    };

    struct deck_card_status_id
    {
        deck_card_id card_id;
        size_t index;

        friend constexpr bool operator==(deck_card_status_id, deck_card_status_id) = default;
    };

    struct support_id
    {
        player_id player_id;
        size_t index;

        friend constexpr bool operator==(support_id, support_id) = default;
    };

    struct summon_id
    {
        player_id player_id;
        size_t index;

        friend constexpr bool operator==(summon_id, summon_id) = default;
    };

    struct character_id
    {
        player_id player_id;
        size_t index;

        friend constexpr bool operator==(character_id, character_id) = default;
    };

    struct attachment_id
    {
        character_id character_id;
        size_t index;

        friend constexpr bool operator==(attachment_id, attachment_id) = default;
    };

    struct combat_status_id
    {
        player_id player_id;
        size_t index;

        friend constexpr bool operator==(combat_status_id, combat_status_id) = default;
    };

    struct skill_id
    {
        character_id character_id;
        size_t index;

        friend constexpr bool operator==(skill_id, skill_id) = default;
    };

}

#endif
