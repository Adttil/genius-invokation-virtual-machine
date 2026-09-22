#ifndef GIVM_EXECUTOR_COMMANDS_REMOVE_ATTACHMENT_HPP
#define GIVM_EXECUTOR_COMMANDS_REMOVE_ATTACHMENT_HPP

#include <algorithm>

#include "../broadcast.hpp"
#include "../../definition.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline attachment_id require_attachment(
        const unrestricted_table& table, player_id player, definition_id<attachment_view> definition)
    {
        auto attachments = table[*table[player].state().active_character].attachments();
        const auto found = std::ranges::find(attachments, definition,
            [](const auto attachment) { return attachment.definition_id(); });
        const bool exists = found != attachments.end();
        GIVM_ASSERT(exists);
        [[assume(exists)]];
        return (*found).id();
    }

    inline execution_state broadcast_attachment_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<attachment_removed>(library, table, context, random))
            return continue_execution;
        pop_broadcast<attachment_removed>(context);
        return context.enter_next();
    }

    template<bool Fixed>
    execution_state execute_attachment_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        attachment_id attachment;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, remove_attachment>(library);
            const auto player = command.player == relative_player::current
                ? table.state().active_player : other_player(table.state().active_player);
            attachment = require_attachment(table, player, command.definition);
            context.advance(instruction_extent<1, remove_attachment>);
        }
        else
        {
            attachment = get<0>(context.stack().top<attachment_removal>()).attachment;
            context.stack().pop<attachment_removal>();
            context.enter_next();
        }
        const bool valid = static_cast<bool>(table[attachment]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        table[attachment].erase();
        prepare_broadcast(library, attachment_removed{ attachment }, table, context.stack(), context.position());
        return broadcast_attachment_removal(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::remove_attachment& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_attachment_removal<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_attachment_removal<false> });
        writer.write(execute_fn{ broadcast_attachment_removal });
    }
}

#include "../../macro_undef.hpp"
#endif
