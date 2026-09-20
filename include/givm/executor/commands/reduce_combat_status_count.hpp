#ifndef GIVM_EXECUTOR_COMMANDS_REDUCE_COMBAT_STATUS_COUNT_HPP
#define GIVM_EXECUTOR_COMMANDS_REDUCE_COMBAT_STATUS_COUNT_HPP

#include "../executor.hpp"
#include "../instruction.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state execute_reduce_combat_status_count(
        const definition_library&, unrestricted_table& table, execution_context& context, random_fn&)
    {
        const auto& input = get<0>(context.stack().top<combat_status_count_reduction>());
        auto& count = table[input.status].state().count;
        GIVM_ASSERT(count >= input.count);
        count -= input.count;
        context.stack().pop<combat_status_count_reduction>();
        return context.enter_next();
    }

    inline void compile(program_writer& writer, const givm::reduce_combat_status_count&, compile_mode)
    {
        writer.write(execute_fn{ &execute_reduce_combat_status_count });
    }
}

#include "../../macro_undef.hpp"
#endif
