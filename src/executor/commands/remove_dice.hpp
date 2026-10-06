#ifndef GIVM_EXECUTOR_COMMANDS_REMOVE_DICE_HPP
#define GIVM_EXECUTOR_COMMANDS_REMOVE_DICE_HPP

#include "../program_writer.hpp"

#include <vector>

#include <cstddef>

#include "../broadcast.hpp"
#include <givm/definition.hpp>
#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <givm/macro_define.hpp>

namespace givm::detail
{
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
#ifndef NDEBUG
            debug_validate_entity(table, table.state().self_player, "remove_dice", "self_player");
#endif
            player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
            dice = command.dice;
#ifndef NDEBUG
            debug_validate_entity(table, player, "remove_dice", "player");
            if(not table[player].state().dice.contains(dice))
                throw command_input_error{ "remove_dice", insufficient_dice_argument{ player, dice, table[player].state().dice } };
#endif
            context.advance(instruction_extent<1, remove_dice>);
        }
        else
        {
            const auto& input = get<0>(context.stack().top<remove_dice_input>());
            player = input.player;
            dice = input.dice;
#ifndef NDEBUG
            debug_validate_entity(table, player, "remove_dice", "player");
            if(not table[player].state().dice.contains(dice))
                throw command_input_error{ "remove_dice", insufficient_dice_argument{ player, dice, table[player].state().dice } };
#endif
            context.stack().pop<remove_dice_input>();
            context.enter_next();
            if(dice.total() == 0)
                return continue_execution;
        }

        GIVM_ASSERT(player.index < 2);
        auto& available = table[player].state().dice;
        GIVM_ASSERT(available.contains(dice));
        available -= dice;
        append_event_record(context, dice_removed{ player, dice });
        return continue_execution;
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
    }
}

namespace givm::detail
{
    inline std::vector<remove_dice::error_type> check(const remove_dice& command, const definition_compile_context&, program_kind kind)
    {
        using reason = remove_dice::error_type::reason;
        std::vector<remove_dice::error_type> errors;
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

#include <givm/macro_undef.hpp>

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const remove_dice& command) noexcept
    {
        return command.player == static_cast<relative_player>(-1) ? TInputTypes::template index_of<remove_dice::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
