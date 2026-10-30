#ifndef GIVM_EXECUTOR_VIEWS_ACTIVE_CHARACTER_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_ACTIVE_CHARACTER_SELECTION_HPP

#include <utility>

#include "../executor.hpp"

namespace givm
{
    enum class active_character_selection_validation : std::uint8_t
    {
        valid,
        invalid_player,
        wrong_player,
        invalid_character,
        defeated_character
    };

    inline std::string error_string(active_character_selection_validation value)
    {
        switch(value)
        {
        case active_character_selection_validation::valid: return "valid";
        case active_character_selection_validation::invalid_player: return "invalid player";
        case active_character_selection_validation::wrong_player: return "wrong player";
        case active_character_selection_validation::invalid_character: return "invalid character";
        case active_character_selection_validation::defeated_character: return "defeated character";
        }
        return "unknown selection validation";
    }

    template<execution_state State>
    requires (State == execution_state::active_character_selection0 || State == execution_state::active_character_selection1)
    class execution_view<State, role::full>
    {
    public:
        static constexpr execution_state state = State;
        static constexpr givm::role access_role = role::full;
        static constexpr bool accepts_input = true;

        static constexpr player_id player() noexcept
        {
            return player_id{ (static_cast<unsigned>(State) & 1u) };
        }

        active_character_selection_validation selection_validate(const table& card_table,
            character_id character) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<State>(version_);
#endif
            if(character.player_id().index() >= 2) return active_character_selection_validation::invalid_player;
            if(character.player_id() != player()) return active_character_selection_validation::wrong_player;
            const auto characters = card_table[player()].characters<false>();
            if(character.index() >= characters.size() || not characters[character.index()])
                return active_character_selection_validation::invalid_character;
            const auto& state = characters[character.index()].state();
            if(not state.alive || state.health == 0) return active_character_selection_validation::defeated_character;
            return active_character_selection_validation::valid;
        }

        template<class TRandom>
        execution_state select(const definition_library& library, table& card_table, TRandom& random,
            character_id character) const
        {
#ifndef NDEBUG
            executor_->validate_view<State>(version_);
            const auto result = selection_validate(card_table, character);
            if(result != active_character_selection_validation::valid)
                throw view_input_error{ "active_character_selection.select", result };
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

    template<execution_state State, role Role>
    requires ((State == execution_state::active_character_selection0 || State == execution_state::active_character_selection1) && Role != role::full && detail::accepts_input<State, Role>)
    class execution_view<State, Role> : private execution_view<State, role::full>
    {
        using base = execution_view<State, role::full>;

    public:
        static constexpr execution_state state = State;
        static constexpr givm::role access_role = Role;
        static constexpr bool accepts_input = true;
        using base::player;
        using base::selection_validate;
        using base::select;

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

    template<execution_state State, role Role>
    requires ((State == execution_state::active_character_selection0 || State == execution_state::active_character_selection1) && Role != role::full && !detail::accepts_input<State, Role>)
    class execution_view<State, Role> : private execution_view<State, role::full>
    {
        using base = execution_view<State, role::full>;

    public:
        static constexpr execution_state state = State;
        static constexpr givm::role access_role = Role;
        static constexpr bool accepts_input = false;
        using base::player;
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
