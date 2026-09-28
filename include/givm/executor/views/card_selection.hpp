#ifndef GIVM_EXECUTOR_VIEWS_CARD_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_CARD_SELECTION_HPP

#include <bitset>
#include <utility>

#include "../executor.hpp"

namespace givm
{
    struct invalid_card_positions
    {
        std::bitset<selection_capacity> selected;
        std::size_t card_count;
    };

    inline std::string error_string(const invalid_card_positions& error)
    {
        return "invalid card positions " + error.selected.to_string() + "; hand size " + std::to_string(error.card_count);
    }

    template<>
    class execution_view<execution_state::card_selection>
    {
    public:
        constexpr player_id player() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::card_selection>(version_);
#endif
            return get<0>(std::as_const(executor_->context_.stack()).top<player_id, std::bitset<selection_capacity>>());
        }

        constexpr std::bitset<selection_capacity> selected() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::card_selection>(version_);
#endif
            return get<0>(std::as_const(executor_->context_.stack()).top<std::bitset<selection_capacity>>());
        }

        constexpr bool selection_validate(
            const table& card_table, std::bitset<selection_capacity> selected
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::card_selection>(version_);
#endif
            return (selected >> card_table[player()].hand_card_count()).none();
        }

        template<class TRandom>
        execution_state select(
            const definition_library& library, table& card_table, TRandom& random,
            std::bitset<selection_capacity> selected
        ) const
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::card_selection>(version_);
            if(not selection_validate(card_table, selected))
                throw view_input_error{ "card_selection.select", invalid_card_positions{ selected, card_table[player()].hand_card_count() } };
#endif
            get<0>(executor_->context_.stack().top<std::bitset<selection_capacity>>()) = selected;
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
