#ifndef GIVM_EXECUTOR_COMMANDS_ATTACH_HPP
#define GIVM_EXECUTOR_COMMANDS_ATTACH_HPP

#include "../program_writer.hpp"

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <utility>

#include "add_attachment.hpp"

namespace givm::detail
{
    inline std::optional<execution_state> prepare_attachment_application(
        const definition_library& library, unrestricted_table& table, execution_context& context,
        random_fn& random, attach_input input, execution_position reapplication_resume,
        execution_position replacement_resume)
    {
#ifndef NDEBUG
        debug_validate_entity(table, input.target, "attach", "target");
        debug_validate_definition(library, input.definition, "attach", "definition");
#endif
        if(library.is_control(input.definition) && library.is_control_immune(std::as_const(table)[input.target]))
            return std::nullopt;
        const auto definition = library[input.definition];
        input.state = clamp_attachment_state(input.state, definition.query(attachment_state_limit{}));
        for(const auto attachment : std::as_const(table)[input.target].attachments())
        {
            if(attachment.definition_id() != input.definition) continue;
            if(not definition.can_handle<attachment_reapplication, attachment_view>()) return std::nullopt;
            context.stack().push(response_return{ table.state().self_player, reapplication_resume });
            attachment_reapplication event{ input.state };
            auto response = context.make_handle_context(table, random);
            const auto entry = definition.handle<attachment_reapplication>(attachment, event, response);
            if(entry)
            {
                table.state().self_player = attachment.player().id();
                return context.enter(entry);
            }
            context.stack().pop<response_return>();
            return std::nullopt;
        }
        return prepare_attachment_addition(library, table, context, random,
            { input.target, input.definition, input.state }, replacement_resume);
    }

    inline execution_state finish_attachment_reapplication(
        const definition_library&, unrestricted_table&, execution_context& context, random_fn&)
    {
        context.stack().pop<response_return>();
        return context.advance(2 * sizeof(execute_fn));
    }

    template<bool Fixed>
    execution_state execute_attachment_application(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        attach_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, attach>(library);
            const auto player = command.player == relative_player::self
                ? table.state().self_player : other_player(table.state().self_player);
#ifndef NDEBUG
            debug_validate_active_character(table, player, "attach");
#endif
            input = { *table[player].state().active_character, command.definition, command.state };
            context.advance(instruction_extent<1, attach>);
        }
        else
        {
            input = get<0>(context.stack().top<attach_input>());
            context.stack().pop<attach_input>();
            context.enter_next();
        }
        if(const auto state = prepare_attachment_application(library, table, context, random, input,
            context.position(), context.position() + sizeof(execute_fn))) return *state;
        return context.advance(2 * sizeof(execute_fn));
    }

    inline void compile(program_writer& writer, const givm::attach& command, compile_mode)
    {
        if(command.definition)
        {
            writer.write(execute_fn{ execute_attachment_application<true> });
            writer.write(command);
        }
        else
            writer.write(execute_fn{ execute_attachment_application<false> });
        writer.write(execute_fn{ finish_attachment_reapplication });
        writer.write(execute_fn{ finish_replaced_attachment_removal });
    }
}

namespace givm::detail
{
    inline std::vector<attach::error_type> check(const attach& command, const definition_compile_context& context, program_kind kind)
    {
        using reason = attach::error_type::reason;
        std::vector<attach::error_type> errors;
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


#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const attach& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<attach::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
