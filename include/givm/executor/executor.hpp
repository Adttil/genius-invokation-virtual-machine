#ifndef GIVM_EXECUTOR_EXECUTOR_HPP
#define GIVM_EXECUTOR_EXECUTOR_HPP

#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

#include "random_fn.hpp"
#include "../definition/events.hpp"
#include "library.hpp"
#include "handle_context.hpp"
#include "../table.hpp"
#include "../utils/debug.hpp"
#include "../utils/stack.hpp"
#include "../enums/game_result.hpp"

#include "../macro_define.hpp"

namespace givm
{
    inline constexpr size_t selection_capacity = 64;

    enum class execution_state : std::uint8_t
    {
        finished = 1,
        card_selection,
        initial_card_selection,
        initial_active_character_selection,
        remaining_active_character_selection,
        dice_selection,
        action_selection,
        health_reduced,
        deck_cards_discarded,
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
    class execution_context
    {
    public:
        constexpr execution_context(const execution_context&) = default;
        constexpr execution_context(execution_context&&) noexcept = default;
        constexpr execution_context& operator=(const execution_context&) = default;
        constexpr execution_context& operator=(execution_context&&) noexcept = default;
        constexpr ~execution_context() = default;

        constexpr auto& stack(this auto& self) noexcept { return self.stack_; }

        constexpr execution_position position() const noexcept { return position_; }

        template<std::size_t N, class T>
        const T& instruction_data(const definition_library& library) const noexcept
        {
            static_assert(std::is_trivially_copyable_v<T>);
            static_assert(alignof(T) <= program_alignment);
            const auto& bytes = library.program_;
            const auto data_position = position_ + N * sizeof(execute_fn);
            GIVM_ASSERT(position_ % program_alignment == 0);
            GIVM_ASSERT(data_position <= bytes.size() && sizeof(T) <= bytes.size() - data_position);
            return *std::launder(reinterpret_cast<const T*>(bytes.data() + data_position));
        }

        constexpr execution_state jump(execution_position position) noexcept
        {
            position_ = position;
            return continue_execution;
        }

        constexpr execution_state advance(std::size_t bytes) noexcept
        {
            return jump(position_ + bytes);
        }

        constexpr execution_state enter_next() noexcept
        {
            return advance(sizeof(execute_fn));
        }

        constexpr execution_state yield_next(execution_state state) noexcept
        {
            enter_next();
            return state;
        }

        constexpr execution_state yield(execution_state state) const noexcept { return state; }

        program_invoker make_program_invoker()
        {
            return program_invoker{ stack_
#ifndef NDEBUG
                , input_markers_
#endif
            };
        }

        handle_context make_handle_context(const table& table, random_fn& random)
        {
            return handle_context{ table, random, make_program_invoker() };
        }

        static handle_context make_handle_context(
            frame_stack& stack, const definition_library& library, const table& table, random_fn& random)
        {
            return handle_context{ table, random, program_invoker{ stack
#ifndef NDEBUG
                , library.input_markers_
#endif
            } };
        }

        program_entry copy_program_inputs(program_entry entry, std::span<const unsigned char> inputs)
        {
            return make_program_invoker().copy_inputs(entry, inputs);
        }

        constexpr execution_state enter(program_entry entry)
        {
            GIVM_ASSERT(static_cast<bool>(entry));
            return jump(entry.position_);
        }

        constexpr execution_state end_game(game_result result)
        {
            GIVM_ASSERT(result != game_result::no_result);
            stack_.push(result);
            return execution_state::finished;
        }

    private:
        friend class ::givm::executor;
        constexpr execution_context() noexcept = default;

        execution_position position_ = null_program_position;
        frame_stack stack_;
#ifndef NDEBUG
        std::span<const std::size_t> input_markers_;
#endif
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
            context_.position_ = library.entry();
#ifndef NDEBUG
            last_state_ = detail::continue_execution;
#endif
        }

        template<class TRandom>
        execution_state step(const definition_library& library, table& card_table, TRandom& random_source)
        {
            GIVM_ASSERT(last_state_ != execution_state::finished);
            return advance(library, card_table, random_source);
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
        template<class TRandom>
        execution_state advance(const definition_library& library, table& table, TRandom& random_source)
        {
            detail::unrestricted_table& runtime_table = table;
            random_fn random{ random_source };
#ifndef NDEBUG
            context_.input_markers_ = library.input_markers_;
#endif
            while(true)
            {
                const auto execute = context_.instruction_data<0, detail::execute_fn>(library);
                const auto state = execute(library, runtime_table, context_, random);
                if(state != detail::continue_execution)
                {
#ifndef NDEBUG
                    last_state_ = state;
#endif
                    return state;
                }
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
