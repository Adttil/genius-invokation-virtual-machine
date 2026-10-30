#include <optional>
#include <stdexcept>
#include <utility>

#include <givm/executor.hpp>

namespace givm_test
{
    constexpr bool is_action_selection(givm::execution_state state) noexcept
    {
        return state == givm::execution_state::action_selection0
            || state == givm::execution_state::action_selection1;
    }

    constexpr bool is_card_selection(givm::execution_state state) noexcept
    {
        return state == givm::execution_state::card_selection0
            || state == givm::execution_state::card_selection1;
    }

    constexpr bool is_remaining_active_character_selection(givm::execution_state state) noexcept
    {
        return state == givm::execution_state::remaining_active_character_selection0
            || state == givm::execution_state::remaining_active_character_selection1;
    }

    constexpr bool is_active_character_selection(givm::execution_state state) noexcept
    {
        return state == givm::execution_state::active_character_selection0
            || state == givm::execution_state::active_character_selection1;
    }

    constexpr bool is_dice_reroll_selection(givm::execution_state state) noexcept
    {
        return state == givm::execution_state::dice_reroll_selection0
            || state == givm::execution_state::dice_reroll_selection1;
    }

    constexpr bool is_dice_selection(givm::execution_state state) noexcept
    {
        return state == givm::execution_state::dice_selection_both
            || state == givm::execution_state::dice_selection0
            || state == givm::execution_state::dice_selection1;
    }

    inline auto zero_random = [] { return std::uint32_t{ 0 }; };
    inline auto omni_random = [] { return std::uint32_t{ std::to_underlying(givm::elemental_dice::omni) }; };
    // Preserve each returned pause until the test has asserted it. Every transition
    // itself goes through a native public view; no executor internals are used.
    class executor_driver
    {
    public:
        auto start(const givm::definition_library& library, givm::table& table)
        {
            state_ = givm::execution_state::initialized;
            submitted_ = false;
            return execution_.start(library, table);
        }

        template<givm::execution_state State, givm::role Role = givm::role::full>
        auto view_in() { return execution_.view_in<State, Role>(); }

        template<givm::execution_state State0, givm::execution_state State1, class TFunction>
        decltype(auto) with_view(TFunction&& function)
        {
            if(state_ == State0) return function(execution_.view_in<State0>());
            if(state_ == State1) return function(execution_.view_in<State1>());
            throw std::logic_error{ "test expected one of the specified input states" };
        }

        void submitted(givm::execution_state state)
        {
            state_ = state;
            submitted_ = true;
        }

        template<class TRandom>
        givm::execution_state advance(const givm::definition_library& library, givm::table& table, TRandom& random)
        {
            if(std::exchange(submitted_, false)) return *state_;
            if(not state_) throw std::logic_error{ "test executor has not been started" };
            switch(*state_)
            {
            case givm::execution_state::initialized:
                state_ = execution_.view_in<givm::execution_state::initialized>().resume(library, table, random);
                return *state_;
            case givm::execution_state::health_reduced:
                state_ = execution_.view_in<givm::execution_state::health_reduced>().resume(library, table, random);
                return *state_;
            case givm::execution_state::deck_cards_discarded:
                state_ = execution_.view_in<givm::execution_state::deck_cards_discarded>().resume(library, table, random);
                return *state_;
            case givm::execution_state::active_character_changed:
                state_ = execution_.view_in<givm::execution_state::active_character_changed>().resume(library, table, random);
                return *state_;
            case givm::execution_state::initial_active_characters_selected:
                state_ = execution_.view_in<givm::execution_state::initial_active_characters_selected>().resume(library, table, random);
                return *state_;
            case givm::execution_state::round_started:
                state_ = execution_.view_in<givm::execution_state::round_started>().resume(library, table, random);
                return *state_;
            case givm::execution_state::action_started:
                state_ = execution_.view_in<givm::execution_state::action_started>().resume(library, table, random);
                return *state_;
            case givm::execution_state::round_end_declared:
                state_ = execution_.view_in<givm::execution_state::round_end_declared>().resume(library, table, random);
                return *state_;
            case givm::execution_state::round_ending:
                state_ = execution_.view_in<givm::execution_state::round_ending>().resume(library, table, random);
                return *state_;
            default: throw std::logic_error{ "test must submit input before advancing" };
            }
        }

    private:
        givm::executor execution_;
        std::optional<givm::execution_state> state_;
        bool submitted_ = false;
    };

    template<class TRandom>
    givm::execution_state advance_selecting_first_alive(executor_driver& execution,
        const givm::definition_library& library, givm::table& table, TRandom& random)
    {
        auto state = execution.advance(library, table, random);
        while(state == givm::execution_state::active_character_selection0
            || state == givm::execution_state::active_character_selection1)
        {
            auto select = [&]<givm::execution_state State>()
            {
                const auto view = execution.view_in<State>();
                for(const auto character : table[view.player()].characters())
                {
                    if(not character.state().alive || character.state().health == 0) continue;
                    execution.submitted(view.select(library, table, random, character.id()));
                    return;
                }
                throw std::logic_error{ "no living character available for selection" };
            };
            if(state == givm::execution_state::active_character_selection0)
                select.template operator()<givm::execution_state::active_character_selection0>();
            else select.template operator()<givm::execution_state::active_character_selection1>();
            state = execution.advance(library, table, random);
        }
        return state;
    }
}
