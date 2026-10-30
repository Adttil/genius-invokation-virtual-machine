#ifndef GIVM_TABLE_HPP
#define GIVM_TABLE_HPP

#include "table/unrestricted_table.hpp"
#include "enums/role.hpp"

namespace givm
{
    template<role Role> class perspective_table_view;

    class table : private detail::unrestricted_table
    {
        friend class detail::unrestricted_table;
        friend class executor;
        friend class definition_library;

    public:
        using game_state = table_state;

        template<role Role>
        constexpr auto view_for() const noexcept
        {
            return perspective_table_view<Role>{ *this };
        }

        constexpr table(table_state state = {}, player_state player0 = {}, player_state player1 = {})
        : detail::unrestricted_table{ state, player0, player1 }
        {}

        constexpr const table_state& state() const noexcept
        {
            return static_cast<const detail::unrestricted_table&>(*this).state();
        }

        constexpr auto players() const
        {
            return static_cast<const detail::unrestricted_table&>(*this).players();
        }

        template<class T>
        decltype(auto) operator[](history_value_key<T> key) const noexcept
        {
            return static_cast<const detail::unrestricted_table&>(*this)[key];
        }

        constexpr auto operator[](player_id id) const
        {
            return static_cast<const detail::unrestricted_table&>(*this)[id];
        }

        constexpr auto operator[](reaction_id id) const
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

        using detail::unrestricted_table::clean_up;
    };
}

#include "table/perspective_table_view.hpp"

#endif
