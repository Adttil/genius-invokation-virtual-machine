#ifndef GIVM_TABLE_PERSPECTIVE_TABLE_VIEW_HPP
#define GIVM_TABLE_PERSPECTIVE_TABLE_VIEW_HPP

#include <ranges>
#include <tuple>
#include "../enums/role.hpp"
#include "entities/hidden_hand_card_view.hpp"
#include "../utils/debug.hpp"
#include "../macro_define.hpp"

namespace givm
{
    template<role Role> class perspective_table_view;

    template<unsigned Player, role Role>
    class perspective_player_view
    {
        static_assert(Player < 2);

    public:
        static constexpr entity_category category = entity_category::player;
        static constexpr givm::role access_role = Role;

        static constexpr player_id id() noexcept { return player_id{ Player }; }
        constexpr auto active_character_id() const { return player_.state().active_character; }
        constexpr auto dice_count() const { return player_.state().dice.total(); }
        constexpr auto hand_limit() const { return player_.state().hand_limit; }
        constexpr auto summon_limit() const { return player_.state().summon_limit; }
        constexpr auto support_limit() const { return player_.state().support_limit; }
        constexpr bool can_plunge() const { return player_.state().can_plunge; }
        constexpr const dice_counts& dice() const { return player_.state().dice; }
        constexpr auto hand_card_count() const { return player_.hand_card_count(); }
        constexpr auto deck_card_count() const { return player_.deck_card_count(); }
        constexpr auto reaction(elemental_reaction slot) const { return player_.reaction(slot); }
        template<bool SkipErased = true>
        constexpr auto characters() const { return player_.template characters<SkipErased>(); }
        template<bool SkipErased = true>
        constexpr auto supports() const { return player_.template supports<SkipErased>(); }
        template<bool SkipErased = true>
        constexpr auto summons() const { return player_.template summons<SkipErased>(); }
        template<bool SkipErased = true>
        constexpr auto combat_statuses() const { return player_.template combat_statuses<SkipErased>(); }

        template<bool SkipErased = true>
        constexpr auto hand_cards() const
        {
            return player_.template hand_cards<SkipErased>();
        }

        constexpr auto operator[](hand_card_id card) const
        {
            GIVM_ASSERT(card.player_id() == id());
            const auto view = player_.table()[card];
            return view;
        }

    private:
        template<role> friend class perspective_table_view;
        constexpr explicit perspective_player_view(player_view player) noexcept : player_{ player } {}
        player_view player_;
    };

    template<unsigned Player, role Role>
    requires ((Role == role::player0 && Player == 1) || (Role == role::player1 && Player == 0))
    class perspective_player_view<Player, Role>
    {
        static_assert(Player < 2);

    public:
        static constexpr entity_category category = entity_category::player;
        static constexpr givm::role access_role = Role;

        static constexpr player_id id() noexcept { return player_id{ Player }; }
        constexpr auto active_character_id() const { return player_.state().active_character; }
        constexpr auto dice_count() const { return player_.state().dice.total(); }
        constexpr auto hand_limit() const { return player_.state().hand_limit; }
        constexpr auto summon_limit() const { return player_.state().summon_limit; }
        constexpr auto support_limit() const { return player_.state().support_limit; }
        constexpr bool can_plunge() const { return player_.state().can_plunge; }
        constexpr auto hand_card_count() const { return player_.hand_card_count(); }
        constexpr auto deck_card_count() const { return player_.deck_card_count(); }
        constexpr auto reaction(elemental_reaction slot) const { return player_.reaction(slot); }
        template<bool SkipErased = true>
        constexpr auto characters() const { return player_.template characters<SkipErased>(); }
        template<bool SkipErased = true>
        constexpr auto supports() const { return player_.template supports<SkipErased>(); }
        template<bool SkipErased = true>
        constexpr auto summons() const { return player_.template summons<SkipErased>(); }
        template<bool SkipErased = true>
        constexpr auto combat_statuses() const { return player_.template combat_statuses<SkipErased>(); }

        template<bool SkipErased = true>
        constexpr auto hand_cards() const
        {
            return player_.template hand_cards<SkipErased>() | std::views::transform(
                [](hand_card_view card) { return hidden_hand_card_view{ card }; });
        }

        constexpr auto operator[](hand_card_id card) const
        {
            GIVM_ASSERT(card.player_id() == id());
            const auto view = player_.table()[card];
            return hidden_hand_card_view{ view };
        }

    private:
        template<role> friend class perspective_table_view;
        constexpr explicit perspective_player_view(player_view player) noexcept : player_{ player } {}
        player_view player_;
    };

    template<role Role>
    class perspective_table_view
    {
    public:
        static constexpr givm::role access_role = Role;

        constexpr const table_state& state() const noexcept { return table_->state(); }
        template<unsigned Player>
        constexpr auto player() const
        {
            static_assert(Player < 2);
            return perspective_player_view<Player, Role>{ (*table_)[player_id{ Player }] };
        }
        constexpr auto players() const { return std::tuple{ player<0>(), player<1>() }; }

        constexpr auto operator[](reaction_id id) const { return (*table_)[id]; }
        constexpr auto operator[](support_id id) const { return (*table_)[id]; }
        constexpr auto operator[](summon_id id) const { return (*table_)[id]; }
        constexpr auto operator[](combat_status_id id) const { return (*table_)[id]; }
        constexpr auto operator[](character_id id) const { return (*table_)[id]; }
        constexpr auto operator[](skill_id id) const { return (*table_)[id]; }
        constexpr auto operator[](attachment_id id) const { return (*table_)[id]; }
        constexpr auto operator[](hand_card_status_id id) const { return (*table_)[id]; }

    private:
        friend class table;
        constexpr explicit perspective_table_view(const givm::table& table) noexcept : table_{ &table } {}
        const givm::table* table_;
    };

    template<>
    class perspective_table_view<role::full>
    {
    public:
        static constexpr givm::role access_role = role::full;

        constexpr const table_state& state() const noexcept { return table_->state(); }
        constexpr auto players() const { return table_->players(); }
        template<unsigned Player>
        constexpr auto player() const
        {
            static_assert(Player < 2);
            return (*table_)[player_id{ Player }];
        }
        template<class TKey>
        constexpr decltype(auto) operator[](TKey key) const { return (*table_)[key]; }

    private:
        friend class table;
        constexpr explicit perspective_table_view(const givm::table& table) noexcept : table_{ &table } {}
        const givm::table* table_;
    };
}

#include "../macro_undef.hpp"

#endif
