#ifndef GIVM_EXECUTOR_COMMANDS_SET_COMBAT_STATUS_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_COMBAT_STATUS_STATE_HPP

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include "add_combat_status.hpp"
#include "remove_combat_status.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline execution_state finish_combat_status_state_change(
        const definition_library&, unrestricted_table&, execution_context& context, random_fn&)
    {
        context.stack().pop<response_return>();
        return context.enter_next();
    }

    inline execution_state change_combat_status_state(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random, const set_combat_status_state_input& input)
    {
        combat_status_state_changed event{ table[input.status].state(), input.state };
        table[input.status].state() = input.state;
        const auto status = std::as_const(table)[input.status];
        const auto definition = library[status.definition_id()];
        if(not definition.can_handle<combat_status_state_changed, combat_status_view>())
            return context.enter_next();
        context.stack().push(response_return{ table.state().self_player, context.position() });
        auto response = context.make_handle_context(table, random);
        const auto entry = definition.handle<combat_status_state_changed>(status, event, response);
        if(entry)
        {
            table.state().self_player = status.player().id();
            return context.enter(entry);
        }
        return finish_combat_status_state_change(library, table, context, random);
    }

    template<bool Fixed>
    execution_state execute_combat_status_state_change(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        set_combat_status_state_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, set_combat_status_state>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
#ifndef NDEBUG
            debug_validate_required_entity(table, player, command.definition, "set_combat_status_state", "status");
#endif
            input = { require_combat_status(table, player, command.definition), command.state };
            context.advance(instruction_extent<1, set_combat_status_state>);
        }
        else
        {
            input = get<0>(context.stack().top<set_combat_status_state_input>());
            context.stack().pop<set_combat_status_state_input>();
            context.enter_next();
        }
#ifndef NDEBUG
        debug_validate_entity(table, input.status, "set_combat_status_state", "status");
#endif
        const bool valid = static_cast<bool>(table[input.status]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        const auto definition = library[table[input.status].definition_id()];
        input.state = clamp_combat_status_state(input.state, definition.query(combat_status_state_limit{}));
        return change_combat_status_state(library, table, context, random, input);
    }

    inline void compile(program_writer& writer, const givm::set_combat_status_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_combat_status_state_change<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_combat_status_state_change<false> });
        writer.write(execute_fn{ finish_combat_status_state_change });
    }
}

namespace givm
{
    inline std::vector<set_combat_status_state::error_type> check(const set_combat_status_state& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = set_combat_status_state::error_type::reason;
        std::vector<set_combat_status_state::error_type> errors;
        if(not command.definition)
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.player != relative_player::self && command.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_player, .value = static_cast<std::size_t>(command.player) });
        if(command.definition.value() >= context.definition_count<combat_status_view>())
            errors.push_back({ .cause = reason::invalid_definition, .value = command.definition.value(), .limit = context.definition_count<combat_status_view>() });
        return errors;
    }
}

#include "../../macro_undef.hpp"
#endif
