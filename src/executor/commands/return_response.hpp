#ifndef GIVM_EXECUTOR_COMMANDS_RETURN_RESPONSE_HPP
#define GIVM_EXECUTOR_COMMANDS_RETURN_RESPONSE_HPP

#include "../program_writer.hpp"
#include <vector>
#include <givm/executor/executor.hpp>
#include <givm/definition.hpp>

namespace givm::detail
{
    template<bool Fixed>
    inline execution_state execute_response_return(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn&)
    {
        std::uint32_t index;
        if constexpr(Fixed)
            index = context.instruction_data<1, return_response>(library).index;
        else
        {
            index = get<0>(context.stack().top<return_response_input>()).index;
            context.stack().pop<return_response_input>();
        }
        auto& record = get<0>(context.stack().top<response_return>());
        record.result = index;
        return context.jump(record.position);
    }

    inline void compile(program_writer& writer, const return_response& command, compile_mode)
    {
        if(command.index == return_response::dynamic)
            writer.write(execute_fn{ execute_response_return<false> });
        else
        {
            writer.write(execute_fn{ execute_response_return<true> });
            writer.write(command);
        }
    }

    inline std::vector<return_response_error> check(
        const return_response&, const definition_compile_context&, program_kind kind)
    {
        if(kind != program_kind::response)
            return { { return_response_error::reason::return_in_root } };
        return {};
    }

#ifndef NDEBUG
    template<class TInputTypes>
    constexpr std::size_t input_marker(const return_response& command) noexcept
    {
        return command.index == return_response::dynamic
            ? TInputTypes::template index_of<return_response_input>() : std::size_t(-1);
    }
#endif
}

#endif
