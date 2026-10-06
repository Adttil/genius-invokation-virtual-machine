#ifndef GIVM_EXECUTOR_COMMANDS_DEFER_PROGRAM_HPP
#define GIVM_EXECUTOR_COMMANDS_DEFER_PROGRAM_HPP

#include <span>
#include <vector>
#include <givm/definition.hpp>
#include <givm/executor/executor.hpp>
#include "../program_writer.hpp"
#include "../settlement.hpp"

namespace givm::detail
{
    template<bool Fixed>
    inline execution_state execute_defer_program(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn&)
    {
        if constexpr(Fixed)
        {
            const auto input = context.instruction_data<1, deferred_program_input>(library);
            constexpr auto offset = instruction_extent<1, deferred_program_input>;
            append_deferred_record(context, input.entry, table.state().self_player,
                context.instruction_bytes(library, offset, input.input_bytes));
            return context.advance(offset + align_program_size(input.input_bytes));
        }
        else
        {
            auto& stack = context.stack();
            const auto [entry, inputs] = stack.top<program_entry, substack_t>();
            const auto begin = stack.size() - substack_tail_size - inputs.size();
            append_deferred_record(context, entry, table.state().self_player,
                std::span<const unsigned char>{ stack.data() + begin, inputs.size() });
            stack.pop<program_entry, substack_t>();
            return context.enter_next();
        }
    }

    inline void compile(program_writer& writer, const givm::defer_program& command, compile_mode)
    {
        if(command.input.entry())
        {
            writer.write(execute_fn{ execute_defer_program<true> });
            const auto bytes = command.input.bytes();
            writer.write(deferred_program_input{ command.input.entry(), bytes.size() });
            writer.write_bytes(bytes);
        }
        else writer.write(execute_fn{ execute_defer_program<false> });
    }

    inline std::vector<defer_program::error_type> check(
        const defer_program& command, const definition_compile_context&, program_kind kind)
    {
        if(kind != program_kind::response && not command.input.entry())
            return { { defer_program_error::reason::dynamic_input_in_root } };
        return {};
    }

    template<class TInputTypes>
    constexpr std::size_t input_marker(const defer_program& command) noexcept
    {
        return not command.input.entry()
            ? TInputTypes::template index_of<defer_program_input>() : std::size_t(-1);
    }
}

#endif
