#ifndef GIVM_EXECUTOR_COMMANDS_ADD_SUPPORT_HPP
#define GIVM_EXECUTOR_COMMANDS_ADD_SUPPORT_HPP

#include "../program_writer.hpp"

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <algorithm>
#include <utility>

#include <givm/executor/executor.hpp>
#include <givm/definition.hpp>

namespace givm::detail
{
    constexpr support_state clamp_support_state(support_state state, support_state limit) noexcept
    {
        return { std::min(state.count, limit.count), std::min(state.round_usages, limit.round_usages) };
    }

    template<bool Fixed>
    inline execution_state apply_support_addition(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        add_support_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, add_support>(library);
            input = {
                .player = command.player == relative_player::self
                    ? table.state().self_player : other_player(table.state().self_player),
                .definition = command.definition, .state = command.state
            };
            context.advance(instruction_extent<1, add_support>);
        }
        else
        {
            input = get<0>(context.stack().top<add_support_input>());
            context.stack().pop<add_support_input>();
            context.enter_next();
        }

#ifndef NDEBUG
        debug_validate_entity(table, input.player, "add_support", "player");
        debug_validate_definition(library, input.definition, "add_support", "definition");
#endif
        const auto player = std::as_const(table)[input.player];
        auto remaining_capacity = player.state().support_limit;
        if(remaining_capacity == 0)
            return continue_execution;
        for([[maybe_unused]] const auto existing : player.supports())
        {
            if(--remaining_capacity == 0)
                return continue_execution;
        }
        input.state = clamp_support_state(input.state, library[input.definition].query(support_state_limit{}));
        table[input.player].add(input.definition, input.state);
        return continue_execution;
    }

    inline void compile(program_writer& writer, const givm::add_support& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ apply_support_addition<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ apply_support_addition<false> });
    }
}

namespace givm::detail
{
    inline std::vector<add_support::error_type> check(const add_support& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = add_support::error_type::reason;
        std::vector<add_support::error_type> errors;
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


#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const add_support& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<add_support::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
