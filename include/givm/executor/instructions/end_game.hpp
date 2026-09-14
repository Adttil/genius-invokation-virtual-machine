#ifndef GIVM_EXECUTOR_INSTRUCTIONS_END_GAME_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_END_GAME_HPP

#include "../executor.hpp"

namespace givm
{
    struct end_game
    {
        using context_type = void;

        game_result result;
    };

    template<>
    struct detail::instruction_implementation<end_game>
    {
        template<bool Observed>
        static execution_state execute(
            const givm::end_game& instruction,
            const definition_library&,
            unrestricted_table&,
            execution_context& context,
            random_fn&
        )
        {
            return context.end_game(instruction.result);
        }
    };
}

#endif
