#include <optional>
#include <stdexcept>
#include <utility>

#include <givm/executor.hpp>

namespace givm_test
{
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

        template<givm::execution_state State>
        auto view_in() { return execution_.view_in<State>(); }

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
}
