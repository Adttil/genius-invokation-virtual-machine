#ifndef GIVM_EXECUTOR_COMMANDS_SUMMON_HPP
#define GIVM_EXECUTOR_COMMANDS_SUMMON_HPP

#include "../program_writer.hpp"

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include "add_summon.hpp"
#include <givm/macro_define.hpp>

namespace givm::detail
{
    inline program_entry prepare_summoning(
        const definition_library& library, unrestricted_table& table, execution_context& context,
        random_fn& random, summon_input input, execution_position resume)
    {
#ifndef NDEBUG
        debug_validate_entity(table, input.player, "summon", "player");
        debug_validate_definition(library, input.definition, "summon", "definition");
#endif
        const auto definition = library[input.definition];
        input.state = clamp_summon_state(input.state, definition.query(summon_state_limit{}));
        const auto player = std::as_const(table)[input.player];
        size_t summon_count = 0;
        for(const auto existing : player.summons())
        {
            ++summon_count;
            if(existing.definition_id() != input.definition)
                continue;
            if(not definition.can_handle<resummoning, summon_view>())
                return {};
            resummoning event{ input.state };
            context.stack().push(response_return{ table.state().self_player, resume });
            auto response = context.make_handle_context(library, table, random);
            const auto entry = definition.handle<resummoning>(existing, event, response);
            if(not entry) context.stack().pop<response_return>();
            else table.state().self_player = existing.player().id();
            return entry;
        }

        if(summon_count >= player.state().summon_limit)
            return {};
        table[input.player].add(input.definition, input.state);
        return {};
    }

    inline execution_state finish_resummoning(
        const definition_library&, unrestricted_table&, execution_context& context, random_fn&)
    {
        context.stack().pop<response_return>();
        return context.enter_next();
    }

    template<bool Fixed>
    execution_state execute_summon(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        summon_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, summon>(library);
            input = {
                .player = command.player == relative_player::self
                    ? table.state().self_player : other_player(table.state().self_player),
                .definition = command.definition, .state = command.state
            };
            context.advance(instruction_extent<1, summon>);
        }
        else
        {
            input = get<0>(context.stack().top<summon_input>());
            context.stack().pop<summon_input>();
            context.enter_next();
        }

        if(const auto entry = prepare_summoning(library, table, context, random, input, context.position()))
            return context.enter(entry);
        return context.enter_next();
    }

    inline void compile(program_writer& writer, const givm::summon& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_summon<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_summon<false> });
        writer.write(execute_fn{ finish_resummoning });
    }
}

namespace givm::detail
{
    inline std::vector<summon::error_type> check(const summon& command, const definition_compile_context& context, program_kind kind)
    {
        using reason = summon::error_type::reason;
        std::vector<summon::error_type> errors;
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
    constexpr std::size_t input_marker(const summon& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<summon::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
