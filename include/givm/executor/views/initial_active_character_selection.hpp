#ifndef GIVM_EXECUTOR_VIEWS_INITIAL_ACTIVE_CHARACTER_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_INITIAL_ACTIVE_CHARACTER_SELECTION_HPP

#include <cstdint>

#include "../executor.hpp"

namespace givm
{
    enum class initial_active_character_selection_validation : std::uint8_t
    {
        valid,
        invalid_player,
        invalid_character,
        wrong_player
    };

    inline std::string error_string(initial_active_character_selection_validation value)
    {
        switch(value)
        {
        case initial_active_character_selection_validation::wrong_player: return "wrong player";
        case initial_active_character_selection_validation::valid: return "valid";
        case initial_active_character_selection_validation::invalid_player: return "invalid player";
        case initial_active_character_selection_validation::invalid_character: return "invalid character";
        }
        return "unknown selection validation";
    }

    template<>
    class execution_view<execution_state::initial_active_character_selection, role::full>
    {
    public:
        static constexpr execution_state state = execution_state::initial_active_character_selection;
        static constexpr givm::role access_role = role::full;
        static constexpr bool accepts_input = true;

        constexpr initial_active_character_selection_validation selection_validate(
            const table& card_table, character_id character
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::initial_active_character_selection>(version_);
#endif
            if(character.player_id().index() >= 2)
            {
                return initial_active_character_selection_validation::invalid_player;
            }
            const auto characters = card_table[character.player_id()].characters<false>();
            if(character.index() >= characters.size() || not characters[character.index()].is_valid())
            {
                return initial_active_character_selection_validation::invalid_character;
            }
            return initial_active_character_selection_validation::valid;
        }

        template<class TRandom>
        execution_state select(
            const definition_library& library, table& card_table, TRandom& random,
            character_id character
        ) const
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::initial_active_character_selection>(version_);
            const auto result = selection_validate(card_table, character);
            if(result != initial_active_character_selection_validation::valid)
                throw view_input_error{ "initial_active_character_selection.select", result };
#endif
            get<0>(executor_->context_.stack().top<character_id>()) = character;
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
    class execution_view<execution_state::initial_active_character_selection, Role> : private execution_view<execution_state::initial_active_character_selection, role::full>
    {
        using base = execution_view<execution_state::initial_active_character_selection, role::full>;

    public:
        static constexpr execution_state state = execution_state::initial_active_character_selection;
        static constexpr givm::role access_role = Role;
        static constexpr bool accepts_input = true;

        constexpr initial_active_character_selection_validation selection_validate(
            const table& card_table, character_id character
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            this->executor_->template validate_view<state>(this->version_);
#endif
            if(character.player_id().index() < 2
                && character.player_id() != player_id{ detail::role_player<Role> })
                return initial_active_character_selection_validation::wrong_player;
            return base::selection_validate(card_table, character);
        }

        template<class TRandom>
        execution_state select(const definition_library& library, table& card_table, TRandom& random,
            character_id character) const
        {
#ifndef NDEBUG
            const auto result = selection_validate(card_table, character);
            if(result != initial_active_character_selection_validation::valid)
                throw view_input_error{ "initial_active_character_selection.select", result };
#endif
            return base::select(library, card_table, random, character);
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
    class execution_view<execution_state::initial_active_character_selection, role::spectator> : private execution_view<execution_state::initial_active_character_selection, role::full>
    {
        using base = execution_view<execution_state::initial_active_character_selection, role::full>;

    public:
        static constexpr execution_state state = execution_state::initial_active_character_selection;
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
