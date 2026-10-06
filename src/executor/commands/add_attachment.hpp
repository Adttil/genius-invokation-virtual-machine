#ifndef GIVM_EXECUTOR_COMMANDS_ADD_ATTACHMENT_HPP
#define GIVM_EXECUTOR_COMMANDS_ADD_ATTACHMENT_HPP

#include "../program_writer.hpp"

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <algorithm>
#include <optional>
#include <utility>

#include "../broadcast.hpp"
#include <givm/definition.hpp>

namespace givm::detail
{
    inline attachment_state clamp_attachment_state(attachment_state state, attachment_state limit) noexcept
    {
        return { std::min(state.count, limit.count), std::min(state.round_usages, limit.round_usages) };
    }

    inline void apply_attachment_addition(const definition_library& library, unrestricted_table& table,
        execution_context& context, add_attachment_input input)
    {
        const auto character = table[input.target];
        const auto type = library.equipment_type(input.definition);
        if(type != equipment_type::none && character.has(type))
        {
            const auto old = character.get(type).id();
            table[old].erase();
            append_removal_record<attachment_removal_effect>(context, old, attachment_removed{ old });
        }
        if(type == equipment_type::none) character.add(input.definition, input.state);
        else character.add(input.definition, input.state, type);
    }

    template<bool Fixed>
    execution_state execute_attachment_addition(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        add_attachment_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, add_attachment>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
#ifndef NDEBUG
            debug_validate_active_character(table, player, "add_attachment");
#endif
            input = {
                .target = *table[player].state().active_character,
                .definition = command.definition, .state = command.state
            };
            context.advance(instruction_extent<1, add_attachment>);
        }
        else
        {
            input = get<0>(context.stack().top<add_attachment_input>());
            context.stack().pop<add_attachment_input>();
            context.enter_next();
        }
#ifndef NDEBUG
        debug_validate_entity(table, input.target, "add_attachment", "target");
        debug_validate_definition(library, input.definition, "add_attachment", "definition");
#endif
        if(library.is_control(input.definition) && library.is_control_immune(std::as_const(table)[input.target]))
            return continue_execution;
        input.state = clamp_attachment_state(input.state, library[input.definition].query(attachment_state_limit{}));
        apply_attachment_addition(library, table, context, input);
        return continue_execution;
    }

    inline void compile(program_writer& writer, const givm::add_attachment& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_attachment_addition<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_attachment_addition<false> });
    }
}

namespace givm::detail
{
    inline std::vector<add_attachment::error_type> check(const add_attachment& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = add_attachment::error_type::reason;
        std::vector<add_attachment::error_type> errors;
        if(not command.definition)
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.player != relative_player::self && command.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_player, .value = static_cast<std::size_t>(command.player) });
        if(command.definition.value() >= context.definition_count<attachment_view>())
            errors.push_back({ .cause = reason::invalid_definition, .value = command.definition.value(), .limit = context.definition_count<attachment_view>() });
        return errors;
    }
}


namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const add_attachment& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<add_attachment::input_type>() : std::size_t(-1);
    }
}

#endif
