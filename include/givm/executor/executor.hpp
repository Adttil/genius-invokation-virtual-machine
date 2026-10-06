#ifndef GIVM_EXECUTOR_EXECUTOR_HPP
#define GIVM_EXECUTOR_EXECUTOR_HPP

#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

#include "random_fn.hpp"
#include "execution_state.hpp"
#include "execution_view_error.hpp"
#include "../definition_common.hpp"
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

    class executor;

    template<execution_state State>
    class execution_view
    {
        static_assert(
            State == execution_state::initialized
            || State == execution_state::initial_active_characters_selected
            || State == execution_state::round_started
            || State == execution_state::action_started
            || State == execution_state::round_end_declared
            || State == execution_state::round_ending
        );
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
        template<class TRandom>
        execution_state resume(this const auto& self, const definition_library& library, table& card_table, TRandom& random)
        {
#ifndef NDEBUG
            self.executor_->template validate_view<State>(self.version_);
#endif
            return self.executor_->advance(library, card_table, random);
        }
    };
}

namespace givm::detail
{
    inline constexpr std::size_t no_boundary = std::size_t(-1);

    struct event_domain
    {
        std::size_t last_boundary = no_boundary;
    };

    class execution_context
    {
    public:
        constexpr execution_context(const execution_context&) = default;
        constexpr execution_context(execution_context&&) noexcept = default;
        constexpr execution_context& operator=(const execution_context&) = default;
        constexpr execution_context& operator=(execution_context&&) noexcept = default;
        constexpr ~execution_context() = default;

        constexpr auto& stack(this auto& self) noexcept { return self.stack_; }

        constexpr auto& damage_events(this auto& self) noexcept { return self.damage_events_; }
        constexpr auto& hand_entry_events(this auto& self) noexcept { return self.hand_entry_events_; }
        constexpr auto& mixed_events(this auto& self) noexcept { return self.mixed_events_; }

        void reset_event_queues()
        {
            damage_events_.clear();
            hand_entry_events_.clear();
            mixed_events_.clear();
            damage_events_.push(substack());
            hand_entry_events_.push(substack());
            mixed_events_.push(event_domain{}, substack());
        }

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

        std::span<const unsigned char> instruction_bytes(
            const definition_library& library, std::size_t offset, std::size_t count) const noexcept
        {
            const auto begin = position_ + offset;
            GIVM_ASSERT(begin <= library.program_.size() && count <= library.program_.size() - begin);
            return { library.program_.data() + begin, count };
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

        template<event_category Category = event_category::normal>
        program_invoker<Category> make_program_invoker()
        {
            return program_invoker<Category>{ stack_
#ifndef NDEBUG
                , debug_
#endif
            };
        }

        template<event_category Category, class TEntity>
        requires (Category != event_category::preview)
        handle_context<TEntity, Category> make_handle_context(const definition_library& library, TEntity entity, random_fn& random)
        {
            return handle_context<TEntity, Category>{ library, entity, &random, make_program_invoker<Category>() };
        }

        template<class TEntity>
        static preview_handle_context<TEntity> make_preview_context(
            frame_stack& stack, const definition_library& library, TEntity entity)
        {
            return preview_handle_context<TEntity>{ library, entity, {}, program_invoker<event_category::preview>{ stack
#ifndef NDEBUG
                , detail::program_debug_view{ library.debug_library_identity_, library.debug_programs_, library.input_markers_ }
#endif
            } };
        }

        template<event_category Category>
        effect<Category> copy_program_inputs(effect<Category> entry, std::span<const unsigned char> inputs)
        {
            return make_program_invoker().copy_inputs(entry, inputs);
        }

        template<event_category Category>
        constexpr execution_state enter(effect<Category> entry)
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

        execution_position position_ = null_effect_position;
        frame_stack stack_;
        frame_stack damage_events_;
        frame_stack hand_entry_events_;
        frame_stack mixed_events_;
#ifndef NDEBUG
        program_debug_view debug_;
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
#ifdef NDEBUG
        constexpr executor(executor&&) noexcept = default;
        constexpr executor& operator=(const executor&) = default;
        constexpr executor& operator=(executor&&) noexcept = default;
#else
        constexpr executor(executor&& other) noexcept
        : context_{ std::move(other.context_) }, last_state_{ other.last_state_ }
        {
            other.invalidate_views();
        }

        constexpr executor& operator=(const executor& other)
        {
            if(this != &other)
            {
                invalidate_views();
                context_ = other.context_;
                last_state_ = other.last_state_;
            }
            return *this;
        }

        constexpr executor& operator=(executor&& other) noexcept
        {
            if(this != &other)
            {
                invalidate_views();
                context_ = std::move(other.context_);
                last_state_ = other.last_state_;
                other.invalidate_views();
            }
            return *this;
        }
#endif
        constexpr ~executor() = default;

        auto start(const definition_library& library, table& card_table)
        {
#ifndef NDEBUG
            invalidate_views();
#endif
            context_.stack_.clear();
            context_.reset_event_queues();
            context_.position_ = library.entry();
            library.initialize_history(card_table);
#ifndef NDEBUG
            last_state_ = execution_state::initialized;
#endif
            return view_in<execution_state::initialized>();
        }

        template<execution_state State>
        constexpr execution_view<State> view_in() noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view<State>(version_);
#endif
            return execution_view<State>{ *this
#ifndef NDEBUG
                , version_
#endif
            };
        }

    private:
        template<execution_state> friend class execution_view;

#ifndef NDEBUG
        constexpr void invalidate_views() noexcept
        {
            ++version_;
            last_state_ = detail::continue_execution;
        }

        template<execution_state State>
        constexpr void validate_view(std::size_t version) const
        {
            if(version != version_)
                throw execution_view_error{ expired_execution_view{ version, version_ } };
            if(last_state_ != State)
                throw execution_view_error{ unexpected_execution_state{ State,
                    last_state_ == detail::continue_execution ? std::nullopt : std::optional{ last_state_ } } };
        }
#endif

        template<class TRandom>
        execution_state advance(const definition_library& library, table& table, TRandom& random_source)
        {
#ifndef NDEBUG
            invalidate_views();
#endif
            detail::unrestricted_table& runtime_table = table;
            random_fn random{ random_source };
#ifndef NDEBUG
            context_.debug_ = { library.debug_library_identity_, library.debug_programs_, library.input_markers_ };
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
        std::size_t version_ = 0;
#endif
    };

    template<>
    class execution_view<execution_state::finished>
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
        constexpr game_result result() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            executor_->validate_view<execution_state::finished>(version_);
#endif
            const auto [result] = std::as_const(executor_->context_).stack().top<game_result>();
            return result;
        }
    };
}

#include "../macro_undef.hpp"
#endif
