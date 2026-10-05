#ifndef GIVM_EXECUTOR_COMMANDS_END_SEGMENT_HPP
#define GIVM_EXECUTOR_COMMANDS_END_SEGMENT_HPP

#include <givm/definition.hpp>
#include <givm/executor/command_input_error.hpp>
#include "../program_writer.hpp"
#include "../settlement.hpp"

namespace givm::detail
{
    inline execution_state execute_end_segment(
        const definition_library&, unrestricted_table&, execution_context& context, random_fn&)
    {
#ifndef NDEBUG
        if(is_inline_response(context))
            throw command_input_error{ "end_segment", settlement_in_inline_response{} };
#endif
        seal_segment(context);
        return context.enter_next();
    }

    inline void compile(program_writer& writer, const givm::end_segment&, compile_mode)
    {
        writer.write(execute_fn{ execute_end_segment });
    }
}

#endif
