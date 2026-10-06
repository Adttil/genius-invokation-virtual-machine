#ifndef GIVM_EXECUTOR_COMMANDS_REMOVE_COMBAT_STATUS_HPP
#define GIVM_EXECUTOR_COMMANDS_REMOVE_COMBAT_STATUS_HPP

#include "../program_writer.hpp"

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <algorithm>

#include "../broadcast.hpp"
#include <givm/definition.hpp>
#include <givm/macro_define.hpp>

namespace givm::detail
{


    inline combat_status_id require_combat_status(
        const unrestricted_table& table, player_id player, definition_id<combat_status_view> definition)
    {
        auto statuses = table[player].combat_statuses();
        const auto target = std::ranges::find_if(statuses,
            [&](const auto entity) { return entity.definition_id() == definition; });
        const bool found = target != statuses.end();
        GIVM_ASSERT(found);
        [[assume(found)]];
        return (*target).id();
    }

    template<bool Fixed>
    execution_state prepare_combat_status_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        combat_status_id status;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, remove_combat_status>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
#ifndef NDEBUG
            debug_validate_required_entity(table, player, command.definition, "remove_combat_status", "status");
#endif
            status = require_combat_status(table, player, command.definition);
            context.advance(instruction_extent<1, remove_combat_status>);
        }
        else
        {
            status = get<0>(context.stack().top<remove_combat_status_input>()).status;
            context.stack().pop<remove_combat_status_input>();
            context.enter_next();
        }
#ifndef NDEBUG
        debug_validate_entity(table, status, "remove_combat_status", "status");
#endif
        const bool valid = static_cast<bool>(table[status]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        table[status].erase();
        append_removal_record<combat_status_removal_effect>(context, status, combat_status_removed{ status });
        return continue_execution;
    }



    inline void compile(program_writer& writer, const givm::remove_combat_status& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ prepare_combat_status_removal<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ prepare_combat_status_removal<false> });
    }
}

namespace givm::detail
{
    inline std::vector<remove_combat_status::error_type> check(const remove_combat_status& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = remove_combat_status::error_type::reason;
        std::vector<remove_combat_status::error_type> errors;
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

#include <givm/macro_undef.hpp>

namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const remove_combat_status& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<remove_combat_status::input_type>() : std::size_t(-1);
    }
}

#endif
