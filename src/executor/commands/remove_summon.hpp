#ifndef GIVM_EXECUTOR_COMMANDS_REMOVE_SUMMON_HPP
#define GIVM_EXECUTOR_COMMANDS_REMOVE_SUMMON_HPP

#include "../program_writer.hpp"

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <algorithm>
#include <cstdint>
#include <type_traits>

#include "../broadcast.hpp"
#include <givm/definition.hpp>
#include <givm/macro_define.hpp>

namespace givm::detail
{
    inline summon_id require_summon(
        const unrestricted_table& table, player_id player, definition_id<summon_view> definition)
    {
        auto summons = table[player].summons();
        const auto target = std::ranges::find_if(summons,
            [&](const auto entity) { return entity.definition_id() == definition; });
        const bool found = target != summons.end();
        GIVM_ASSERT(found);
        [[assume(found)]];
        return (*target).id();
    }

    inline void remove_summon_and_record(unrestricted_table& table, execution_context& context, summon_id id)
    {
        table[id].erase();
        append_removal_record<summon_removal_effect>(context, id, summon_removed{ id });
    }

    template<bool Fixed>
    inline execution_state prepare_summon_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, remove_summon>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
#ifndef NDEBUG
            debug_validate_required_entity(table, player, command.definition, "remove_summon", "summon");
#endif
            const auto summon = require_summon(table, player, command.definition);
            context.advance(instruction_extent<1, remove_summon>);
            remove_summon_and_record(table, context, summon);
            return continue_execution;
        }
        else
        {
            const auto summons = get<0>(context.stack().top<summon_id[]>());
#ifndef NDEBUG
            debug_validate_unique(std::span<const summon_id>{ summons }, "remove_summon", "summons");
            for(std::size_t index = 0; index != summons.size(); ++index)
                debug_validate_entity(table, summons[index], "remove_summon", "summons[" + std::to_string(index) + "]");
#endif
            for(const auto summon : summons) remove_summon_and_record(table, context, summon);
            context.stack().pop<summon_id[]>();
            return context.enter_next();
        }
    }

    inline void compile(program_writer& writer, const givm::remove_summon& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ prepare_summon_removal<true> });
            writer.write(command);
        }
        else
        {
            writer.write(execute_fn{ prepare_summon_removal<false> });
        }
    }
}

namespace givm::detail
{
    inline std::vector<remove_summon::error_type> check(const remove_summon& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = remove_summon::error_type::reason;
        std::vector<remove_summon::error_type> errors;
        if(not command.definition)
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.player != relative_player::self && command.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_player, .value = static_cast<std::size_t>(command.player) });
        if(command.definition.value() >= context.definition_count<summon_view>())
            errors.push_back({ .cause = reason::invalid_definition, .value = command.definition.value(), .limit = context.definition_count<summon_view>() });
        return errors;
    }
}

#include <givm/macro_undef.hpp>

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const remove_summon& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<remove_summon::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
