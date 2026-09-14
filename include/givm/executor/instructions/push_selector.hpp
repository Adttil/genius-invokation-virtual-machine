#ifndef GIVM_EXECUTOR_INSTRUCTIONS_PUSH_SELECTOR_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_PUSH_SELECTOR_HPP

#include "../executor.hpp"
#include "../selector.hpp"

namespace givm
{
    struct push_selector
    {
        using context_type = void;

        player_id player;
        std::bitset<selection_capacity> selected;
    };

    namespace detail
    {
        template<>
        struct instruction_implementation<push_selector>
        {
            template<bool Observed>
            static execution_state execute(
                const givm::push_selector& instruction, const definition_library&,
                unrestricted_table&, execution_context& context, random_fn&
            )
            {
                context.stack().push(
                    selector{ .player = instruction.player, .selected = instruction.selected }
                );
                return context.enter_next();
            }

        };
    }
}

#endif
