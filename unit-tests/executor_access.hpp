#ifndef GIVM_TEST_EXECUTOR_ACCESS_HPP
#define GIVM_TEST_EXECUTOR_ACCESS_HPP

#include <givm/executor.hpp>

namespace givm::detail
{
    // Internal protocol tests may inspect a single invocation without making it public API.
    struct executor_access
    {
        static frame_stack& stack(executor& target) { return target.context_.stack_; }
        static const frame_stack& stack(const executor& target) { return target.context_.stack_; }
        static execution_position position(const executor& target) { return target.context_.position_; }
        static execution_position entry(const definition_library& library) { return library.entry(); }
        static any_instruction_view instruction(const definition_library& library, execution_position position)
        {
            return library.instruction(position);
        }

        template<bool Observed = false, class TRandom>
        static execution_state execute_next(
            executor& target, const definition_library& library, card_table& table, TRandom& source
        )
        {
            target.settle_control_instructions(library);
            random_fn random{ source };
            const auto instruction = library.instruction(target.context_.position_);
            const auto state = instruction.template execute<Observed>(library, table, target.context_, random);
            if(state == continue_execution)
                target.settle_control_instructions(library);
#ifndef NDEBUG
            target.last_state_ = state;
#endif
            return state;
        }
    };
}

#endif
