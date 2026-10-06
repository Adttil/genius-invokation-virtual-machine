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

    template<>
    class execution_view<execution_state::active_character_selection>
    {
    public:
        player_id player() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::active_character_selection>(version_);
#endif
            return get<0>(std::as_const(executor_->context_.stack()).top<player_id, character_id>());
        }

        active_character_selection_validation selection_validate(const table& card_table,
            character_id character) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::active_character_selection>(version_);
#endif
            if(character.player_id.index >= 2) return active_character_selection_validation::invalid_player;
            if(character.player_id != player()) return active_character_selection_validation::wrong_player;
            const auto characters = card_table[player()].characters<false>();
            if(character.index >= characters.size() || not characters[character.index])
                return active_character_selection_validation::invalid_character;
            const auto& state = characters[character.index].state();
            if(not state.alive || state.health == 0) return active_character_selection_validation::defeated_character;
            return active_character_selection_validation::valid;
        }

        template<class TRandom>
        execution_state select(const definition_library& library, table& card_table, TRandom& random,
            character_id character) const
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::active_character_selection>(version_);
            const auto result = selection_validate(card_table, character);
            if(result != active_character_selection_validation::valid)
                throw view_input_error{ "active_character_selection.select", result };
#endif
            get<0>(executor_->context_.stack().top<character_id>()) = character;
            return executor_->advance(library, card_table, random);
        }

    private:
        friend class executor;
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
}

#endif
