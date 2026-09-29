#ifndef GIVM_EXECUTOR_VIEWS_DICE_REROLL_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_DICE_REROLL_SELECTION_HPP

#include <cstdint>
#include <utility>

#include "../executor.hpp"
#include "../command_input_error.hpp"

namespace givm::detail
{
    struct dice_reroll_lane
    {
        std::uint32_t remaining = 0;
        std::uint32_t cursor = 0;
    };

    struct single_player_dice_reroll
    {
        player_id player;
        dice_reroll_lane lane;
    };
}

namespace givm
{
    template<>
    class execution_view<execution_state::dice_reroll_selection>
    {
    public:
        constexpr player_id player() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::dice_reroll_selection>(version_);
#endif
            return get<0>(std::as_const(executor_->context_.stack()).top<detail::single_player_dice_reroll, dice_counts>()).player;
        }

        constexpr bool selection_validate(const table& card_table, const dice_counts& selected) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::dice_reroll_selection>(version_);
#endif
            return card_table[player()].state().dice.contains(selected);
        }

        template<class TRandom>
        execution_state select(
            const definition_library& library, table& card_table, TRandom& random,
            const dice_counts& selected
        ) const
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::dice_reroll_selection>(version_);
            if(not selection_validate(card_table, selected))
                throw view_input_error{ "dice_reroll_selection.select", insufficient_dice_argument{ player(), selected, card_table[player()].state().dice } };
#endif
            get<0>(executor_->context_.stack().top<dice_counts>()) = selected;
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
