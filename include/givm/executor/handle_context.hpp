#ifndef GIVM_EXECUTOR_HANDLE_CONTEXT_HPP
#define GIVM_EXECUTOR_HANDLE_CONTEXT_HPP

#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include "program_input_error.hpp"

#include "../definition_common.hpp"
#include "../utils/stack.hpp"
#include "random_fn.hpp"
#include "library.hpp"

namespace givm::detail
{
    template<command_input T>
    auto snapshot_invocation_input(const T& input) { return input; }

    inline auto snapshot_invocation_input(const defer_program_input& input) noexcept
    {
        return std::cref(input);
    }
}

namespace givm
{
    class program_invoker
    {
    public:
        program_entry operator()(program_entry entry, const program_inputs& inputs)
        {
            return invoke_packed<false>(entry, inputs);
        }

        program_entry operator()(substack_t, program_entry entry, const program_inputs& inputs)
        {
            return invoke_packed<true>(entry, inputs);
        }

        template<detail::command_input... T>
        program_entry operator()(program_entry entry, T&&... inputs)
        {
            return invoke_values<false>(entry, std::forward<T>(inputs)...);
        }

        template<detail::command_input... T>
        program_entry operator()(substack_t, program_entry entry, T&&... inputs)
        {
            return invoke_values<true>(entry, std::forward<T>(inputs)...);
        }

    private:
#ifndef NDEBUG
        template<bool InSubstack>
        void validate_invocation(const detail::debug_program_info& program) const
        {
            if(invoked_) throw program_input_error{ repeated_program_invocation{}, program.source, program.program_index };
            if(InSubstack != substack_)
                throw program_input_error{ program_invocation_mode_mismatch{ substack_, InSubstack },
                    program.source, program.program_index };
        }
#endif

        // Payment caches and delayed records contain complete, already checked frames.
        program_entry copy_inputs(program_entry entry, std::span<const unsigned char> inputs)
        {
            detail::append_input_bytes(stack_, inputs);
            return entry;
        }

        template<bool InSubstack>
        program_entry invoke_packed(program_entry entry, const program_inputs& inputs)
        {
#ifndef NDEBUG
            const auto descriptions = inputs.descriptions();
            const auto& program = detail::program_input_validator{ debug_ }.validate(entry, descriptions);
            validate_invocation<InSubstack>(program);
            invoked_ = true;
#endif
            if constexpr(InSubstack)
            {
                auto destination = get<0>(stack_.top<substack_t>());
                detail::append_input_bytes(destination, inputs.bytes());
            }
            else detail::append_input_bytes(stack_, inputs.bytes());
            return entry;
        }

        template<bool InSubstack, class... T>
        program_entry invoke_values(program_entry entry, const T&... inputs)
        {
#ifndef NDEBUG
            constexpr std::array<std::size_t, sizeof...(T)> markers{ detail::command_input_types::index_of<T>()... };
            const detail::program_input_validator validator{ debug_ };
            const auto& program = validator.validate_parameters(entry, markers.size(), [&](std::size_t index) { return markers[index]; });
            validate_invocation<InSubstack>(program);
            (validator.validate_value(inputs), ...);
            invoked_ = true;
#endif
            // Snapshot scalar inputs before stack growth. Owned deferred inputs are
            // borrowed for this call; their independent storage cannot move with stack_.
            const auto values = std::make_tuple(detail::snapshot_invocation_input(inputs)...);
            const auto push = [&]<class TDestination>(TDestination& destination)
            {
                [&]<std::size_t... I>(std::index_sequence<I...>)
                {
                    (detail::push_command_input(destination, std::get<sizeof...(T) - 1 - I>(values)), ...);
                }(std::index_sequence_for<T...>{});
            };
            if constexpr(InSubstack)
            {
                auto destination = get<0>(stack_.top<substack_t>());
                push(destination);
            }
            else push(stack_);
            return entry;
        }

        friend class detail::execution_context;

        explicit program_invoker(frame_stack& stack
#ifndef NDEBUG
            , detail::program_debug_view debug, bool substack = false
#endif
        ) noexcept
        : stack_{ stack }
#ifndef NDEBUG
        , debug_{ debug }, substack_{ substack }
#endif
        {}

        frame_stack& stack_;
#ifndef NDEBUG
        detail::program_debug_view debug_;
        bool substack_ = false;
        bool invoked_ = false;
#endif
    };

    template<class TEntity>
    class handle_context
    {
    public:
        const TEntity& entity() const noexcept { return entity_; }

        const givm::table& table() const noexcept { return entity_.table(); }

        std::uint32_t random() const { return random_(); }

        template<class TCategory, class TQuery>
            requires requires { supported_queries<TCategory>::template index_of<TQuery>(); }
        TQuery::result_t query(definition_id<TCategory> id, const TQuery& parameters) const
        {
            return library_.query(id, parameters);
        }

        program_entry invoke(program_entry entry, const program_inputs& inputs)
        {
            return invoker_(entry, inputs);
        }

        program_entry invoke(substack_t tag, program_entry entry, const program_inputs& inputs)
        {
            return invoker_(tag, entry, inputs);
        }

        template<detail::command_input... T>
        program_entry invoke(program_entry entry, T&&... inputs)
        {
            return invoker_(entry, std::forward<T>(inputs)...);
        }

        template<detail::command_input... T>
        program_entry invoke(substack_t tag, program_entry entry, T&&... inputs)
        {
            return invoker_(tag, entry, std::forward<T>(inputs)...);
        }

    private:
        friend class detail::execution_context;

        handle_context(const definition_library& library, TEntity entity, random_fn& random, program_invoker invoker) noexcept
        : library_{ library }, entity_{ entity }, random_{ random }, invoker_{ invoker }
        {}

        const definition_library& library_;
        TEntity entity_;
        random_fn& random_;
        program_invoker invoker_;
    };
}

#endif
