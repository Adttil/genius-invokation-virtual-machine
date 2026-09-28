#ifndef GIVM_EXECUTOR_HANDLE_CONTEXT_HPP
#define GIVM_EXECUTOR_HANDLE_CONTEXT_HPP

#include <array>
#include <cstdint>
#include <cstring>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include "program_input_error.hpp"

#include "../definition.hpp"
#include "../utils/stack.hpp"
#include "random_fn.hpp"

namespace givm::detail
{
    template<class T>
    concept command_input = requires { command_input_types::index_of<std::remove_cvref_t<T>>(); };

    template<command_input T>
    constexpr auto command_input_members(const T& input) noexcept
    {
        return std::tie(input);
    }

    inline auto command_input_members(const deal_damage_input& input) noexcept
    {
        return std::tuple{ dynamic_array<damage>(input.damages) };
    }

    inline auto command_input_members(const discard_hand_card_input& input) noexcept
    {
        return std::tuple{ dynamic_array<hand_card_id>(input.cards) };
    }

    inline auto command_input_members(const draw_cards_input& input) noexcept
    {
        return std::tuple{ dynamic_array<deck_card_id>(input.cards) };
    }

    inline auto command_input_members(const set_summon_state_input& input) noexcept
    {
        return std::tuple{ dynamic_array<set_summon_state_input::change>(input.changes) };
    }

    inline auto command_input_members(const modify_summon_state_input& input) noexcept
    {
        return std::tuple{ dynamic_array<summon_id>(input.summons), input.value, input.usages };
    }

    inline auto command_input_members(const remove_summon_input& input) noexcept
    {
        return std::tuple{ dynamic_array<summon_id>(input.summons) };
    }

    inline auto command_input_members(const modify_energy_input& input) noexcept
    {
        return std::tuple{ dynamic_array<character_id>(input.targets), input.delta };
    }

    template<class TDestination, command_input T>
    inline void push_command_input(TDestination& destination, const T& input)
    {
        std::apply([&](const auto&... members) { destination.push(members...); }, command_input_members(input));
    }
}

namespace givm
{
    class program_invoker
    {
    public:
        program_entry operator()(program_entry entry, std::span<const any_command_input> inputs)
        {
            return invoke_sequence<false>(entry, inputs);
        }

        program_entry operator()(substack_t, program_entry entry, std::span<const any_command_input> inputs)
        {
            return invoke_sequence<true>(entry, inputs);
        }

        template<detail::command_input... T>
        program_entry operator()(program_entry entry, T... inputs)
        {
            return invoke_values<false>(entry, inputs...);
        }

        template<detail::command_input... T>
        program_entry operator()(substack_t, program_entry entry, T... inputs)
        {
            return invoke_values<true>(entry, inputs...);
        }

    private:
#ifndef NDEBUG
        template<bool InSubstack, class TMarker>
        void validate_inputs(program_entry entry, std::size_t count, TMarker marker)
        {
            if(not entry) throw program_input_error{ invalid_program_entry::null_entry };
            if(entry.library_identity_ != debug_.library_identity)
                throw program_input_error{ invalid_program_entry::different_library };
            if(entry.debug_index_ >= debug_.programs.size())
                throw program_input_error{ invalid_program_entry::unknown_entry };
            const auto& program = debug_.programs[entry.debug_index_];
            if(program.position != entry.position_ || program.inputs_begin > debug_.inputs.size()
                || program.inputs_count > debug_.inputs.size() - program.inputs_begin)
                throw program_input_error{ invalid_program_entry::unknown_entry };
            const auto fail = [&](program_input_error_reason reason)
            {
                throw program_input_error{ std::move(reason), program.source, program.program_index };
            };
            if(invoked_) fail(repeated_program_invocation{});
            if(InSubstack != substack_) fail(program_invocation_mode_mismatch{ substack_, InSubstack });
            if(program.inputs_count != count)
                fail(program_input_count_mismatch{ program.inputs_count, count });
            for(std::size_t index = 0; index != count; ++index)
            {
                const auto& expected = debug_.inputs[program.inputs_begin + index];
                const auto actual = marker(index);
                if(expected.marker != actual)
                    fail(program_input_type_mismatch{ index, expected.command_index, std::string{ expected.command },
                        std::string{ expected.command } + "_input", actual < detail::debug_input_command_names.size()
                            ? std::string{ detail::debug_input_command_names[actual] } + "_input" : "valueless input" });
            }
            invoked_ = true;
        }
#endif

        // Payment caches contain complete frames already checked by the original invoke.
        program_entry copy_inputs(program_entry entry, std::span<const unsigned char> inputs)
        {
            const auto offset = stack_.size();
            for(std::size_t bytes = 0; bytes < inputs.size(); bytes += max_alignment)
                stack_.push<unsigned char[max_alignment]>();
            if(not inputs.empty())
                std::memcpy(stack_.data() + offset, inputs.data(), inputs.size());
            return entry;
        }

        template<bool InSubstack>
        program_entry invoke_sequence(program_entry entry, std::span<const any_command_input> inputs)
        {
#ifndef NDEBUG
            validate_inputs<InSubstack>(entry, inputs.size(), [&](std::size_t index) { return inputs[index].index(); });
#endif
            const auto push = [&](auto& destination)
            {
                for(auto iter = inputs.rbegin(); iter != inputs.rend(); ++iter)
                    std::visit([&](const auto& input) { detail::push_command_input(destination, input); }, *iter);
            };
            if constexpr(InSubstack)
            {
                auto destination = get<0>(stack_.top<substack_t>());
                push(destination);
            }
            else
            {
                push(stack_);
            }
            return entry;
        }

        template<bool InSubstack, class... T>
        program_entry invoke_values(program_entry entry, const T&... inputs)
        {
#ifndef NDEBUG
            constexpr std::array<std::size_t, sizeof...(T)> markers{ detail::command_input_types::index_of<T>()... };
            validate_inputs<InSubstack>(entry, markers.size(), [&](std::size_t index) { return markers[index]; });
#endif
            // Scalar inputs are copied before a push can relocate the stack.
            // Ranges borrowed by an input must remain valid until their contents have been copied.
            const auto values = std::forward_as_tuple(inputs...);
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
            else
            {
                push(stack_);
            }
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

    class handle_context
    {
    public:
        const givm::table& table() const noexcept { return table_; }

        std::uint32_t random() const { return random_(); }

        program_entry invoke(program_entry entry, std::span<const any_command_input> inputs)
        {
            return invoker_(entry, inputs);
        }

        program_entry invoke(substack_t tag, program_entry entry, std::span<const any_command_input> inputs)
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

        handle_context(const givm::table& table, random_fn& random, program_invoker invoker) noexcept
        : table_{ table }, random_{ random }, invoker_{ invoker }
        {}

        const givm::table& table_;
        random_fn& random_;
        program_invoker invoker_;
    };
}

#endif
