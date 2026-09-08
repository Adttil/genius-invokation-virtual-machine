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

        bool execute(card_table&, execution_context& context, random_fn&) const
        {
            context.stack().push(
                selector{ .player = player, .selected = selected }
            );
            return context.enter_next();
        }
    };
}

#endif
