#ifndef GIVM_EXECUTOR_VIEWS_INITIAL_CARD_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_INITIAL_CARD_SELECTION_HPP

#include <bitset>
#include <cstdint>

#include "../executor.hpp"

namespace givm
{
    enum class initial_card_selection_check_result : std::uint8_t
    {
        valid,
        invalid_player,
        invalid_card_position
    };

    template<>
    class execution_view<execution_state::initial_card_selection>
    {
    public:
        constexpr initial_card_selection_check_result check_selection(
            const table& card_table, player_id player,
            std::bitset<selection_capacity> selected
        ) const noexcept
        {
            if(player.index >= 2)
            {
                return initial_card_selection_check_result::invalid_player;
            }
            return (selected >> card_table[player].hand_card_count()).none()
                ? initial_card_selection_check_result::valid
                : initial_card_selection_check_result::invalid_card_position;
        }

        constexpr void select(player_id player, std::bitset<selection_capacity> selected) const noexcept
        {
            auto&& [input_player, input_selected] = stack_->top<player_id, std::bitset<selection_capacity>>();
            input_player = player;
            input_selected = selected;
        }

    private:
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;
    };
}

#endif
