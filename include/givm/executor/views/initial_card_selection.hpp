#ifndef GIVM_EXECUTOR_VIEWS_INITIAL_CARD_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_INITIAL_CARD_SELECTION_HPP

#include <bitset>
#include <cstdint>

#include "../executor.hpp"

namespace givm
{
    enum class initial_card_selection_validation : std::uint8_t
    {
        valid,
        invalid_player,
        invalid_card_position
    };

    inline std::string error_string(initial_card_selection_validation value)
    {
        switch(value)
        {
        case initial_card_selection_validation::valid: return "valid";
        case initial_card_selection_validation::invalid_player: return "invalid player";
        case initial_card_selection_validation::invalid_card_position: return "invalid card position";
        }
        return "unknown selection validation";
    }

    template<>
    class execution_view<execution_state::initial_card_selection, role::full>
    {
    public:
        static constexpr execution_state state = execution_state::initial_card_selection;
        static constexpr givm::role access_role = role::full;
        static constexpr bool accepts_input = true;

        constexpr initial_card_selection_validation selection_validate(
            const table& card_table, player_id player,
            std::bitset<selection_capacity> selected
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::initial_card_selection>(version_);
#endif
            if(player.index() >= 2)
            {
                return initial_card_selection_validation::invalid_player;
            }
            return (selected >> card_table[player].hand_card_count()).none()
                ? initial_card_selection_validation::valid
                : initial_card_selection_validation::invalid_card_position;
        }

        template<class TRandom>
        execution_state select(
            const definition_library& library, table& card_table, TRandom& random,
            player_id player, std::bitset<selection_capacity> selected
        ) const
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::initial_card_selection>(version_);
            const auto result = selection_validate(card_table, player, selected);
            if(result != initial_card_selection_validation::valid)
                throw view_input_error{ "initial_card_selection.select", result };
#endif
            auto&& [input_player, input_selected] = executor_->context_.stack().top<player_id, std::bitset<selection_capacity>>();
            input_player = player;
            input_selected = selected;
            return executor_->advance(library, card_table, random);
        }


    private:
        friend class executor;
        template<execution_state, role> friend class execution_view;
        constexpr explicit execution_view(executor& owner
#ifndef NDEBUG
            , std::size_t version
#endif
        ) noexcept : executor_{ &owner }
#ifndef NDEBUG
            , version_{ version }
#endif
        {}
        executor* executor_;
#ifndef NDEBUG
        std::size_t version_;
#endif
    };

    template<role Role>
    requires (detail::player_role<Role>)
    class execution_view<execution_state::initial_card_selection, Role> : private execution_view<execution_state::initial_card_selection, role::full>
    {
        using base = execution_view<execution_state::initial_card_selection, role::full>;

    public:
        static constexpr execution_state state = execution_state::initial_card_selection;
        static constexpr givm::role access_role = Role;
        static constexpr bool accepts_input = true;

        constexpr initial_card_selection_validation selection_validate(
            const table& card_table, std::bitset<selection_capacity> selected
        ) const noexcept(detail::view_checks_disabled)
        {
            return base::selection_validate(
                card_table, player_id{ detail::role_player<Role> }, selected);
        }

        template<class TRandom>
        execution_state select(const definition_library& library, table& card_table, TRandom& random,
            std::bitset<selection_capacity> selected) const
        {
            return base::select(
                library, card_table, random, player_id{ detail::role_player<Role> }, selected);
        }

    private:
        friend class executor;
        constexpr explicit execution_view(executor& owner
#ifndef NDEBUG
            , std::size_t version
#endif
        ) noexcept : base{ owner
#ifndef NDEBUG
            , version
#endif
        } {}
    };

    template<>
    class execution_view<execution_state::initial_card_selection, role::spectator> : private execution_view<execution_state::initial_card_selection, role::full>
    {
        using base = execution_view<execution_state::initial_card_selection, role::full>;

    public:
        static constexpr execution_state state = execution_state::initial_card_selection;
        static constexpr givm::role access_role = role::spectator;
        static constexpr bool accepts_input = false;
        using base::selection_validate;

    private:
        friend class executor;
        constexpr explicit execution_view(executor& owner
#ifndef NDEBUG
            , std::size_t version
#endif
        ) noexcept : base{ owner
#ifndef NDEBUG
            , version
#endif
        } {}
    };
}

#endif
