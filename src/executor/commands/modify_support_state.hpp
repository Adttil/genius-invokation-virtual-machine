#ifndef GIVM_EXECUTOR_COMMANDS_MODIFY_SUPPORT_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_MODIFY_SUPPORT_STATE_HPP

#include "../program_writer.hpp"

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <cstdint>

#include "set_support_state.hpp"
#include <givm/macro_define.hpp>

namespace givm::detail
{
    template<bool Fixed>
    inline execution_state execute_support_state_modification(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        modify_support_state_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, modify_support_state>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
#ifndef NDEBUG
            debug_validate_required_entity(table, player, command.definition, "modify_support_state", "support");
#endif
            input = { require_support(table, player, command.definition), command.count, command.round_usages };
            context.advance(instruction_extent<1, modify_support_state>);
        }
        else
        {
            input = get<0>(context.stack().top<modify_support_state_input>());
            context.stack().pop<modify_support_state_input>();
            context.enter_next();
        }

#ifndef NDEBUG
        debug_validate_entity(table, input.support, "modify_support_state", "support");
#endif
        const bool valid = static_cast<bool>(table[input.support]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        const auto current = table[input.support].state();
        const auto limit = library[table[input.support].definition_id()].query(support_state_limit{});
        const auto add_saturated = [](std::uint32_t value, std::int64_t delta, std::uint32_t maximum)
        {
            const auto previous = static_cast<std::int64_t>(value);
            if(delta <= -previous)
                return std::uint32_t{};
            if(delta >= static_cast<std::int64_t>(maximum) - previous)
                return maximum;
            return static_cast<std::uint32_t>(previous + delta);
        };
        const support_state state{
            add_saturated(current.count, input.count, limit.count),
            add_saturated(current.round_usages, input.round_usages, limit.round_usages)
        };
        return change_support_state(library, table, context, random, set_support_state_input{ input.support, state });
    }

    inline void compile(program_writer& writer, const givm::modify_support_state& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_support_state_modification<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_support_state_modification<false> });

    }
}

namespace givm::detail
{
    inline std::vector<modify_support_state::error_type> check(const modify_support_state& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = modify_support_state::error_type::reason;
        std::vector<modify_support_state::error_type> errors;
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

namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const modify_support_state& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<modify_support_state::input_type>() : std::size_t(-1);
    }
}

#endif
