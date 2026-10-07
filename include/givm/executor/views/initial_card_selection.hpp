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
    class execution_view<execution_state::initial_card_selection>
    {
    public:
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
