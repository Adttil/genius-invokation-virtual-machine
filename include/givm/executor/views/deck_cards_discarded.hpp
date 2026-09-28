#ifndef GIVM_EXECUTOR_VIEWS_DECK_CARDS_DISCARDED_HPP
#define GIVM_EXECUTOR_VIEWS_DECK_CARDS_DISCARDED_HPP

#include <span>

#include "../executor.hpp"

namespace givm
{
    template<>
    class execution_view<execution_state::deck_cards_discarded>
    {
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

    public:
        constexpr std::span<const deck_card_id> cards() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::deck_cards_discarded>(version_);
#endif
            return get<0>(std::as_const(executor_->context_.stack()).top<deck_card_id[], stack_count_t>());
        }

        template<class TRandom>
        execution_state resume(const definition_library& library, table& card_table, TRandom& random) const
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::deck_cards_discarded>(version_);
#endif
            return executor_->advance(library, card_table, random);
        }
    };
}

#endif
