#ifndef GIVM_EXECUTOR_COMMANDS_REMOVE_SUPPORT_HPP
#define GIVM_EXECUTOR_COMMANDS_REMOVE_SUPPORT_HPP

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


    inline support_id require_support(
        const unrestricted_table& table, player_id player, definition_id<support_view> definition)
    {
        auto supports = table[player].supports();
        const auto target = std::ranges::find_if(supports,
            [&](const auto entity) { return entity.definition_id() == definition; });
        const bool found = target != supports.end();
        GIVM_ASSERT(found);
        [[assume(found)]];
        return (*target).id();
    }

    template<bool Fixed>
    inline execution_state prepare_support_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        support_id support;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, remove_support>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
#ifndef NDEBUG
            debug_validate_required_entity(table, player, command.definition, "remove_support", "support");
#endif
            support = require_support(table, player, command.definition);
            context.advance(instruction_extent<1, remove_support>);
        }
        else
        {
            support = get<0>(context.stack().top<remove_support_input>()).support;
            context.stack().pop<remove_support_input>();
            context.enter_next();
        }
#ifndef NDEBUG
        debug_validate_entity(table, support, "remove_support", "support");
#endif
        const bool valid = static_cast<bool>(table[support]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        table[support].erase();
        append_removal_record<support_removal_effect>(context, support, support_removed{ support });
        return continue_execution;
    }

    inline void compile(program_writer& writer, const givm::remove_support& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ prepare_support_removal<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ prepare_support_removal<false> });
    }
}

namespace givm::detail
{
    inline std::vector<remove_support::error_type> check(const remove_support& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = remove_support::error_type::reason;
        std::vector<remove_support::error_type> errors;
        if(not command.definition)
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.player != relative_player::self && command.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_player, .value = static_cast<std::size_t>(command.player) });
        if(command.definition.value() >= context.definition_count<support_view>())
            errors.push_back({ .cause = reason::invalid_definition, .value = command.definition.value(), .limit = context.definition_count<support_view>() });
        return errors;
    }
}

#include <givm/macro_undef.hpp>

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const remove_support& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<remove_support::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
