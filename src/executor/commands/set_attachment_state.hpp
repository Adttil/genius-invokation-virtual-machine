#ifndef GIVM_EXECUTOR_COMMANDS_SET_ATTACHMENT_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_ATTACHMENT_STATE_HPP

#include "../program_writer.hpp"

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <algorithm>
#include <utility>

#include "add_attachment.hpp"
#include "remove_attachment.hpp"
#include <givm/macro_define.hpp>

namespace givm::detail
{
    template<class Selector>
    struct attachment_state_change_data
    {
        fixed_attachment_target<Selector> target;
        attachment_state state;
    };

    inline execution_state finish_attachment_state_change(
        const definition_library&, unrestricted_table&, execution_context& context, random_fn&)
    {
        context.stack().pop<response_return>();
        return context.enter_next();
    }

    inline execution_state change_attachment_state(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random, attachment_id id, attachment_state state)
    {
        const auto attachment = table[id];
        attachment_state_changed event{ attachment.state(), state };
        attachment.state() = state;
        const auto definition = library[attachment.definition_id()];
        if(not definition.can_handle<attachment_state_changed, attachment_view>())
            return context.enter_next();
        context.stack().push(response_return{ table.state().self_player, context.position() });
        auto response = context.make_handle_context(table, random);
        const auto entry = definition.handle<attachment_state_changed>(std::as_const(table)[id], event, response);
        if(entry)
        {
            table.state().self_player = attachment.player().id();
            return context.enter(entry);
        }
        return finish_attachment_state_change(library, table, context, random);
    }

    template<bool IgnoreLimit, class Selector = void>
    inline execution_state execute_attachment_state_change(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        attachment_id id;
        attachment_state state;
        if constexpr(not std::is_void_v<Selector>)
        {
            const auto& data = context.instruction_data<1, attachment_state_change_data<Selector>>(library);
#ifndef NDEBUG
            debug_validate_attachment_target(library, table, data.target, "set_attachment_state", "target");
#endif
            id = require_attachment(library, table, data.target);
            state = data.state;
            context.advance(instruction_extent<1, attachment_state_change_data<Selector>>);
        }
        else
        {
            const auto& input = get<0>(context.stack().top<set_attachment_state_input>());
#ifndef NDEBUG
            debug_validate_attachment_target(table, input.attachment, "set_attachment_state", "attachment");
#endif
            id = require_attachment(table, input.attachment);
            state = input.state;
            context.stack().pop<set_attachment_state_input>();
            context.enter_next();
        }
        const auto attachment = table[id];
        const bool valid = static_cast<bool>(attachment);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        if constexpr(not IgnoreLimit)
        {
            const auto current = attachment.state();
            const auto limit = library[attachment.definition_id()].query(attachment_state_limit{});
            state = {
                std::min(state.count, std::max(current.count, limit.count)),
                std::min(state.round_usages, std::max(current.round_usages, limit.round_usages))
            };
        }
        return change_attachment_state(library, table, context, random, id, state);
    }

    inline void compile(program_writer& writer, const givm::set_attachment_state& command, compile_mode)
    {
        std::visit([&](auto selector)
        {
            using selector_type = decltype(selector);
            if constexpr(std::is_same_v<selector_type, definition_id<attachment_view>>)
            {
                if(not selector)
                {
                    writer.write(command.ignore_limit
                        ? execute_fn{ execute_attachment_state_change<true> }
                        : execute_fn{ execute_attachment_state_change<false> });
                    return;
                }
            }
            else GIVM_ASSERT(selector != equipment_type::none);
            GIVM_ASSERT(command.target.character.selection == character_selection::character);
            writer.write(command.ignore_limit
                ? execute_fn{ execute_attachment_state_change<true, selector_type> }
                : execute_fn{ execute_attachment_state_change<false, selector_type> });
            writer.write(attachment_state_change_data<selector_type>{
                { command.target.character, selector }, command.state });
        }, command.target.selector);
        writer.write(execute_fn{ finish_attachment_state_change });
    }
}

namespace givm::detail
{
    inline std::vector<set_attachment_state::error_type> check(const set_attachment_state& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = set_attachment_state::error_type::reason;
        std::vector<set_attachment_state::error_type> errors;
        const auto* definition = std::get_if<definition_id<attachment_view>>(&command.target.selector);
        if(definition && not *definition)
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(definition)
        {
            if(definition->value() >= context.definition_count<attachment_view>())
                errors.push_back({ .cause = reason::invalid_definition, .value = definition->value(), .limit = context.definition_count<attachment_view>() });
        }
        else
        {
            const auto equipment = std::get<equipment_type>(command.target.selector);
            if(equipment >= equipment_type::none)
                errors.push_back({ .cause = reason::invalid_equipment_type, .value = static_cast<std::size_t>(equipment) });
        }
        if(command.target.character.player != relative_player::self && command.target.character.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_target_character_player, .value = static_cast<std::size_t>(command.target.character.player) });
        if(command.target.character.selection != character_selection::character)
            errors.push_back({ .cause = reason::invalid_target_character_selection, .value = static_cast<std::size_t>(command.target.character.selection) });
        return errors;
    }
}

#include <givm/macro_undef.hpp>

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const set_attachment_state& command) noexcept
    {
        const auto* definition = std::get_if<definition_id<attachment_view>>(&command.target.selector);
        return definition && not *definition ? TInputTypes::template index_of<set_attachment_state::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
