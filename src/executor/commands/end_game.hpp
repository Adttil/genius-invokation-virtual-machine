#ifndef GIVM_EXECUTOR_COMMANDS_END_GAME_HPP
#define GIVM_EXECUTOR_COMMANDS_END_GAME_HPP

#include "../program_writer.hpp"

#include <vector>

#include <givm/executor/executor.hpp>
#include <givm/definition.hpp>

namespace givm::detail
{
    inline execution_state end_game_execute(
        const definition_library& library, unrestricted_table&,
        execution_context& context, random_fn&
    )
    {
        return context.end_game(context.instruction_data<1, end_game>(library).result);
    }

    inline void compile(program_writer& writer, const end_game& command, compile_mode)
    {
        writer.write<execute_fn>(&end_game_execute);
        writer.write(command);
    }
}

namespace givm::detail
{
    inline std::vector<end_game::error_type> check(const end_game& command, const definition_compile_context&, program_kind)
    {
        using reason = end_game::error_type::reason;
        std::vector<end_game::error_type> errors;
        if(command.result != game_result::player_0_win && command.result != game_result::player_1_win
            && command.result != game_result::both_loss)
            errors.push_back({ .cause = reason::invalid_result, .value = static_cast<std::size_t>(command.result) });
        return errors;
    }
}

#endif
