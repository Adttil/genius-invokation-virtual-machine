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
        invalid_character
    };

    inline std::string error_string(initial_active_character_selection_validation value)
    {
        switch(value)
        {
        case initial_active_character_selection_validation::valid: return "valid";
        case initial_active_character_selection_validation::invalid_player: return "invalid player";
        case initial_active_character_selection_validation::invalid_character: return "invalid character";
        }
        return "unknown selection validation";
    }

    template<>
    class execution_view<execution_state::initial_active_character_selection>
    {
    public:
        constexpr initial_active_character_selection_validation selection_validate(
            const table& card_table, character_id character
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::initial_active_character_selection>(version_);
#endif
            if(character.player_id.index >= 2)
            {
                return initial_active_character_selection_validation::invalid_player;
            }
            const auto characters = card_table[character.player_id].characters<false>();
            if(character.index >= characters.size() || not characters[character.index].is_valid())
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
