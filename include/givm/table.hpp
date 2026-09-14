#ifndef GIVM_TABLE_HPP
#define GIVM_TABLE_HPP

#include "table/unrestricted_table.hpp"

namespace givm
{
    class card_table : private detail::unrestricted_table
    {
        friend class detail::unrestricted_table;
        friend class executor;

    public:
        using game_state = table_state;

        constexpr card_table(game_parameters parameters = {})
        : detail::unrestricted_table{ parameters }
        {}

        constexpr const game_parameters& parameters() const noexcept
        {
            return static_cast<const detail::unrestricted_table&>(*this).parameters();
        }

        constexpr const table_state& state() const noexcept
        {
            return static_cast<const detail::unrestricted_table&>(*this).state();
        }

        constexpr auto players() const
        {
            return static_cast<const detail::unrestricted_table&>(*this).players();
        }

        constexpr auto operator[](player_id id) const
        {
            return static_cast<const detail::unrestricted_table&>(*this)[id];
        }

        constexpr auto operator[](support_id id) const
        {
            return static_cast<const detail::unrestricted_table&>(*this)[id];
        }

        constexpr auto operator[](summon_id id) const
        {
            return static_cast<const detail::unrestricted_table&>(*this)[id];
        }

        constexpr auto operator[](combat_status_id id) const
        {
            return static_cast<const detail::unrestricted_table&>(*this)[id];
        }

        constexpr auto operator[](hand_card_id id) const
        {
            return static_cast<const detail::unrestricted_table&>(*this)[id];
        }

        constexpr auto operator[](deck_card_id id) const
        {
            return static_cast<const detail::unrestricted_table&>(*this)[id];
        }

        constexpr auto operator[](hand_card_status_id id) const
        {
            return static_cast<const detail::unrestricted_table&>(*this)[id];
        }

        constexpr auto operator[](deck_card_status_id id) const
        {
            return static_cast<const detail::unrestricted_table&>(*this)[id];
        }

        constexpr auto operator[](character_id id) const
        {
            return static_cast<const detail::unrestricted_table&>(*this)[id];
        }

        constexpr auto operator[](skill_id id) const
        {
            return static_cast<const detail::unrestricted_table&>(*this)[id];
        }

        constexpr auto operator[](attachment_id id) const
        {
            return static_cast<const detail::unrestricted_table&>(*this)[id];
        }

        using detail::unrestricted_table::load_deck;
        using detail::unrestricted_table::clean_up;
    };
}

#endif
