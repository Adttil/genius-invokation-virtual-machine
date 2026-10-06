#ifndef GIVM_EXECUTOR_BROADCAST_HPP
#define GIVM_EXECUTOR_BROADCAST_HPP

#include "settlement.hpp"

namespace givm::detail
{
    template<class TEvent>
    void compile_broadcast(program_writer& writer, execute_fn continuation)
    {
        writer.write(continuation);
        if constexpr(not inline_event<TEvent>) compile_settlement(writer, begin_settlement);
        writer.write(execute_fn{ complete_broadcast_response<TEvent> });
    }

    template<class TEvent, class TId>
    void compile_single_response(program_writer& writer, execute_fn continuation)
    {
        writer.write(continuation);
        if constexpr(not inline_event<TEvent>) compile_settlement(writer, begin_settlement);
        writer.write(execute_fn{ complete_single_response<TEvent, TId> });
    }
}

#endif
