#ifndef GIVM_EXECUTOR_EXECUTOR_HPP
#define GIVM_EXECUTOR_EXECUTOR_HPP

#include <cstdint>
#include <utility>

#include "random_fn.hpp"
#include "events.hpp"
#include "library.hpp"
#include "../table.hpp"
#include "../utils/debug.hpp"
#include "../utils/stack.hpp"
#include "../enums/game_result.hpp"

#include "../macro_define.hpp"

namespace givm
{
    enum class execution_state : std::uint8_t
    {
        finished = 1,
        card_selection,
        initial_card_selection,
        initial_active_character_selection,
        remaining_active_character_selection,
        dice_selection,
        action,
        health_reduced,
        active_character_changed,
        initial_active_characters_selected,
        round_started,
        action_started,
        round_end_declared,
        round_ending
    };

    namespace detail
    {
        inline constexpr execution_state continue_execution = static_cast<execution_state>(0);

        constexpr bool is_input_state(execution_state state) noexcept
        {
            return state >= execution_state::card_selection && state <= execution_state::action;
        }
    }

    class executor;

    template<execution_state State>
    class execution_view
    {
        static_assert(
            State == execution_state::initial_active_characters_selected
            || State == execution_state::round_started
            || State == execution_state::action_started
            || State == execution_state::round_end_declared
            || State == execution_state::round_ending
        );
        friend class executor;
        constexpr execution_view() noexcept = default;
    };

    template<>
    class execution_view<execution_state::finished>
    {
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;

    public:
        constexpr game_result result() const noexcept
        {
            const auto [result] = std::as_const(*stack_).top<game_result>();
            return result;
        }
    };
}

namespace givm::detail
{
    using stage_t = std::uint8_t;

    class execution_context
    {
    public:
        struct return_info { execution_position return_position; };
        constexpr execution_context(const execution_context&) = default;
        constexpr execution_context(execution_context&&) noexcept = default;
        constexpr execution_context& operator=(const execution_context&) = default;
        constexpr execution_context& operator=(execution_context&&) noexcept = default;
        constexpr ~execution_context() = default;

        constexpr auto& stack(this auto& self) noexcept { return self.stack_; }

        constexpr execution_state enter_next() noexcept
        {
            current_stage() = stage_t{};
            ++position_;
            return continue_execution;
        }

        constexpr execution_state yield_next(execution_state state) noexcept
        {
            enter_next();
            return state;
        }

        constexpr execution_state yield(execution_state state) const noexcept { return state; }

        template<class TContext>
        constexpr execution_state enter(program_entry<TContext> entry)
        {
            GIVM_ASSERT(not entry.is_null());
            stack_.push(return_info{ position_ }, stage_t{});
            position_ = entry.position_;
            return continue_execution;
        }

        constexpr execution_state end_game(game_result result)
        {
            GIVM_ASSERT(result != game_result::no_result);
            stack_.push(result);
            return execution_state::finished;
        }

        constexpr stage_t& current_stage() noexcept
        {
            auto&& [stage] = stack_.top<stage_t>();
            return stage;
        }

    private:
        friend class ::givm::executor;
        constexpr execution_context() noexcept = default;

        constexpr void return_from_subroutine()
        {
            auto&& [info_ref, stage] = stack_.top<return_info, stage_t>();
            GIVM_ASSERT(stage == stage_t{});
            const auto info = info_ref;
            stack_.pop<return_info, stage_t>();
            position_ = info.return_position;
        }

        execution_position position_ = null_program_position;
        frame_stack stack_;
    };
}

namespace givm
{
    struct assume_enabled_t
    {
        explicit constexpr assume_enabled_t() noexcept = default;
    };
    inline constexpr assume_enabled_t assume_enabled;

    class executor
    {
    public:
        constexpr executor() noexcept = default;
        constexpr executor(const executor&) = default;
        constexpr executor(executor&&) noexcept = default;
        constexpr executor& operator=(const executor&) = default;
        constexpr executor& operator=(executor&&) noexcept = default;
        constexpr ~executor() = default;

        constexpr void enter_entry(const definition_library& library)
        {
            context_.stack_.clear();
            context_.stack_.push(detail::stage_t{});
            context_.position_ = library.entry();
#ifndef NDEBUG
            last_state_ = detail::continue_execution;
#endif
        }

        template<class TRandom>
        execution_state step(const definition_library& library, table& card_table, TRandom& random_source)
        {
            GIVM_ASSERT(last_state_ != execution_state::finished);
            return advance<true>(library, card_table, random_source);
        }

        template<class TRandom>
        execution_state run(const definition_library& library, table& card_table, TRandom& random_source)
        {
            GIVM_ASSERT(last_state_ == detail::continue_execution || detail::is_input_state(last_state_));
            return advance<false>(library, card_table, random_source);
        }

        template<execution_state State>
        constexpr auto view_in() noexcept
        {
            GIVM_ASSERT(last_state_ == State);
            if constexpr(requires { execution_view<State>{ context_.stack_ }; })
            {
                return execution_view<State>{ context_.stack_ };
            }
            else
            {
                return execution_view<State>{};
            }
        }

    private:
        template<bool Observed, class TRandom>
        execution_state advance(const definition_library& library, table& table, TRandom& random_source)
        {
            detail::unrestricted_table& runtime_table = table;
            random_fn random{ random_source };
            while(true)
            {
                settle_control_instructions(library);
                const auto instruction = library.instruction(context_.position_);
                const auto state = instruction.template execute<Observed>(library, runtime_table, context_, random);
                if(state != detail::continue_execution)
                {
#ifndef NDEBUG
                    last_state_ = state;
#endif
                    return state;
                }
            }
        }

        constexpr void settle_control_instructions(const definition_library& library)
        {
            while(true)
            {
                const auto instruction = library.instruction(context_.position_);
                if(instruction.template is<detail::return_instruction>())
                {
                    context_.return_from_subroutine();
                    continue;
                }
                if(instruction.template is<detail::jump_instruction>())
                {
                    context_.position_ = instruction.template as<detail::jump_instruction>().target;
                    continue;
                }
                return;
            }
        }

        detail::execution_context context_;
#ifndef NDEBUG
        execution_state last_state_ = detail::continue_execution;
#endif
    };
}

#include "../macro_undef.hpp"
#endif
