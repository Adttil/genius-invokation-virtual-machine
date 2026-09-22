#ifndef GIVM_EXECUTOR_COMMANDS_MODIFY_ATTACHMENT_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_MODIFY_ATTACHMENT_STATE_HPP

#include "set_attachment_state.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<bool Fixed>
    execution_state execute_attachment_state_modification(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        attachment_state_modification input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, modify_attachment_state>(library);
            const auto player = command.player == relative_player::current
                ? table.state().active_player : other_player(table.state().active_player);
            input = { require_attachment(table, player, command.definition), command.count, command.round_usages };
            context.advance(instruction_extent<1, modify_attachment_state>);
        }
        else
        {
            input = get<0>(context.stack().top<attachment_state_modification>());
            context.stack().pop<attachment_state_modification>();
            context.enter_next();
        }
        const auto attachment = table[input.attachment];
        const bool valid = static_cast<bool>(attachment);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        const auto state = attachment.state();
        const auto limit = library[attachment.definition_id()].query(attachment_state_limit{});
        const auto add = [](std::uint32_t value, std::int64_t delta, std::uint32_t maximum) -> std::uint32_t
        {
            if(delta < -static_cast<std::int64_t>(value)) return 0;
            if(delta > static_cast<std::int64_t>(maximum) - value) return maximum;
            return static_cast<std::uint32_t>(static_cast<std::int64_t>(value) + delta);
        };
        return change_attachment_state(library, table, context, random, input.attachment, {
            add(state.count, input.count, limit.count),
            add(state.round_usages, input.round_usages, limit.round_usages)
        });
    }

    inline void compile(program_writer& writer, const givm::modify_attachment_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_attachment_state_modification<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_attachment_state_modification<false> });
        writer.write(execute_fn{ finish_attachment_state_change });
    }
}

#include "../../macro_undef.hpp"
#endif
