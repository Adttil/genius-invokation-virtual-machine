#ifndef GIVM_EXECUTOR_COMMANDS_REMOVE_DICE_HPP
#define GIVM_EXECUTOR_COMMANDS_REMOVE_DICE_HPP

#include <cstddef>

#include "../broadcast.hpp"
#include "../../definition.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<std::size_t Next = 1>
    inline execution_state broadcast_removed_dice(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<dice_removed>(library, table, context, random))
            return continue_execution;
        pop_broadcast<dice_removed>(context);
        return context.advance(Next * sizeof(execute_fn));
    }

    template<bool Fixed>
    inline execution_state remove_dice_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        player_id player;
        dice_counts dice;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, remove_dice>(library);
            player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            dice = command.dice;
            context.advance(instruction_extent<1, remove_dice>);
        }
        else
        {
            const auto& input = get<0>(context.stack().top<remove_dice_input>());
            player = input.player;
            dice = input.dice;
            context.stack().pop<remove_dice_input>();
            context.enter_next();
            if(dice.total() == 0)
                return context.enter_next();
        }

        GIVM_ASSERT(player.index < 2);
        auto& available = table[player].state().dice;
        GIVM_ASSERT(available.contains(dice));
        available -= dice;
        prepare_broadcast(library, dice_removed{ player, dice }, table, context.stack(), context.position());
        return broadcast_removed_dice(library, table, context, random);
    }

    inline void compile(program_writer& writer, const remove_dice& command, compile_mode)
    {
        if(command.player == static_cast<relative_player>(-1))
            writer.write(execute_fn{ remove_dice_execute<false> });
        else
        {
            GIVM_ASSERT(command.player == relative_player::self || command.player == relative_player::opponent);
            if(command.dice.total() == 0)
                return;
            writer.write(execute_fn{ remove_dice_execute<true> });
            writer.write(command);
        }
        writer.write(execute_fn{ broadcast_removed_dice<> });
    }
}

#include "../../macro_undef.hpp"
#endif
