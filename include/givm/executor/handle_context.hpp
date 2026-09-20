#ifndef GIVM_EXECUTOR_HANDLE_CONTEXT_HPP
#define GIVM_EXECUTOR_HANDLE_CONTEXT_HPP

#include <cstdint>
#include <cstring>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#ifndef NDEBUG
#include <stdexcept>
#endif

#include "../definition.hpp"
#include "../utils/stack.hpp"
#include "random_fn.hpp"

namespace givm
{
    class program_invoker
    {
    public:
        program_entry operator()(program_entry entry, std::span<const unsigned char> inputs)
        {
            return invoke_bytes<false>(entry, inputs);
        }

        program_entry operator()(substack_t, program_entry entry, std::span<const unsigned char> inputs)
        {
            return invoke_bytes<true>(entry, inputs);
        }

        template<class... T>
        requires ((std::is_trivially_copyable_v<T> && ...)
            && (not std::is_convertible_v<T, std::span<const unsigned char>> && ...))
        program_entry operator()(program_entry entry, T... inputs)
        {
            return invoke_values<false>(entry, inputs...);
        }

        template<class... T>
        requires ((std::is_trivially_copyable_v<T> && ...)
            && (not std::is_convertible_v<T, std::span<const unsigned char>> && ...))
        program_entry operator()(substack_t, program_entry entry, T... inputs)
        {
            return invoke_values<true>(entry, inputs...);
        }

    private:
        template<bool InSubstack>
        program_entry invoke_bytes(program_entry entry, std::span<const unsigned char> inputs)
        {
#ifndef NDEBUG
            if(entry.inputs_size_ != inputs.size())
            {
                throw std::invalid_argument{ "program input byte size does not match the entry" };
            }
#endif
            if constexpr(InSubstack)
            {
                if(not inputs.empty())
                {
                    const auto destination = get<0>(stack_.top<substack_t>());
                    const auto& first = get<0>(destination.push<unsigned char[max_alignment]>());
                    const auto offset = static_cast<std::size_t>(first - stack_.data());
                    for(std::size_t bytes = max_alignment; bytes < inputs.size(); bytes += max_alignment)
                    {
                        destination.push<unsigned char[max_alignment]>();
                    }
                    std::memcpy(stack_.data() + offset, inputs.data(), inputs.size());
                }
            }
            else
            {
                const auto offset = stack_.size();
                for(std::size_t bytes = 0; bytes < inputs.size(); bytes += max_alignment)
                {
                    stack_.push<unsigned char[max_alignment]>();
                }
                if(not inputs.empty())
                {
                    std::memcpy(stack_.data() + offset, inputs.data(), inputs.size());
                }
            }
            return entry;
        }

        template<bool InSubstack, class... T>
        program_entry invoke_values(program_entry entry, const T&... inputs)
        {
            static_assert(((alignof(T) <= max_alignment) && ...));
#ifndef NDEBUG
            constexpr auto inputs_size = (0uz + ... + ((sizeof(T) + max_alignment - 1)
                / max_alignment * max_alignment));
            if(entry.inputs_size_ != inputs_size)
            {
                throw std::invalid_argument{ "program input byte size does not match the entry" };
            }
#endif
            // The by-value inputs remain valid while pushing can relocate the stack.
            const auto values = std::forward_as_tuple(inputs...);
            if constexpr(InSubstack)
            {
                const auto destination = get<0>(stack_.top<substack_t>());
                [&]<std::size_t... I>(std::index_sequence<I...>)
                {
                    (destination.push(std::get<sizeof...(T) - 1 - I>(values)), ...);
                }(std::index_sequence_for<T...>{});
            }
            else
            {
                [&]<std::size_t... I>(std::index_sequence<I...>)
                {
                    (stack_.push(std::get<sizeof...(T) - 1 - I>(values)), ...);
                }(std::index_sequence_for<T...>{});
            }
            return entry;
        }

        friend class detail::execution_context;

        explicit program_invoker(frame_stack& stack) noexcept
        : stack_{ stack }
        {}

        frame_stack& stack_;
    };

    class handle_context
    {
    public:
        const givm::table& table() const noexcept { return table_; }

        std::uint32_t random() const { return random_(); }

        program_entry invoke(program_entry entry, std::span<const unsigned char> inputs)
        {
            return invoker_(entry, inputs);
        }

        program_entry invoke(substack_t tag, program_entry entry, std::span<const unsigned char> inputs)
        {
            return invoker_(tag, entry, inputs);
        }

        template<class... T>
        requires (not std::is_convertible_v<T, std::span<const unsigned char>> && ...)
        program_entry invoke(program_entry entry, T&&... inputs)
        {
            return invoker_(entry, std::forward<T>(inputs)...);
        }

        template<class... T>
        requires (not std::is_convertible_v<T, std::span<const unsigned char>> && ...)
        program_entry invoke(substack_t tag, program_entry entry, T&&... inputs)
        {
            return invoker_(tag, entry, std::forward<T>(inputs)...);
        }

    private:
        friend class detail::execution_context;

        handle_context(const givm::table& table, random_fn& random, program_invoker invoker) noexcept
        : table_{ table }, random_{ random }, invoker_{ invoker }
        {}

        const givm::table& table_;
        random_fn& random_;
        program_invoker invoker_;
    };
}

#endif
