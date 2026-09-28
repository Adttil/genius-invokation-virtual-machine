#ifndef GIVM_EXECUTOR_COMMANDS_ADD_DICE_HPP
#define GIVM_EXECUTOR_COMMANDS_ADD_DICE_HPP

#include <vector>

#include "../broadcast.hpp"
#include "../../definition.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state broadcast_added_dice(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<dice_added>(library, table, context, random))
            return continue_execution;
        pop_broadcast<dice_added>(context);
        return context.enter_next();
    }

    template<bool Fixed>
    inline execution_state add_dice_execute(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        player_id player;
        dice_counts dice;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, add_dice>(library);
            player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            dice = command.dice;
            context.advance(instruction_extent<1, add_dice>);
        }
        else
        {
            const auto& input = get<0>(context.stack().top<add_dice_input>());
            player = input.player;
            dice = input.dice;
            context.stack().pop<add_dice_input>();
            context.enter_next();
            if(dice.total() == 0)
                return context.enter_next();
        }

        table[player].state().dice += dice;
        prepare_broadcast(library, dice_added{ player, dice }, table, context.stack(), context.position());
        return broadcast_added_dice(library, table, context, random);
    }

    inline void compile(program_writer& writer, const add_dice& command, compile_mode)
    {
        if(command.player == static_cast<relative_player>(-1))
            writer.write(execute_fn{ add_dice_execute<false> });
        else
        {
            GIVM_ASSERT(command.player == relative_player::self || command.player == relative_player::opponent);
            if(command.dice.total() == 0)
                return;
            writer.write(execute_fn{ add_dice_execute<true> });
            writer.write(command);
        }
        writer.write(execute_fn{ broadcast_added_dice });
    }
}

namespace givm
{
    inline std::vector<add_dice::error_type> check(const add_dice& command, const definition_compile_context&, program_kind kind)
    {
        using reason = add_dice::error_type::reason;
        std::vector<add_dice::error_type> errors;
        if(command.player == static_cast<relative_player>(-1))
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.player != relative_player::self && command.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_player, .value = static_cast<std::size_t>(command.player) });
        return errors;
    }
}

#include "../../macro_undef.hpp"
#endif
