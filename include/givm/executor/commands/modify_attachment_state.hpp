#ifndef GIVM_EXECUTOR_COMMANDS_MODIFY_ATTACHMENT_STATE_HPP
#define GIVM_EXECUTOR_COMMANDS_MODIFY_ATTACHMENT_STATE_HPP

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include "set_attachment_state.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<class Selector>
    struct attachment_state_modification_data
    {
        fixed_attachment_target<Selector> target;
        std::int64_t count;
        std::int64_t round_usages;
    };

    template<class Selector = void>
    execution_state execute_attachment_state_modification(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        attachment_id id;
        std::int64_t count;
        std::int64_t round_usages;
        if constexpr(not std::is_void_v<Selector>)
        {
            const auto& data = context.instruction_data<1, attachment_state_modification_data<Selector>>(library);
#ifndef NDEBUG
            debug_validate_attachment_target(library, table, data.target, "modify_attachment_state", "target");
#endif
            id = require_attachment(library, table, data.target);
            count = data.count;
            round_usages = data.round_usages;
            context.advance(instruction_extent<1, attachment_state_modification_data<Selector>>);
        }
        else
        {
            const auto& input = get<0>(context.stack().top<modify_attachment_state_input>());
#ifndef NDEBUG
            debug_validate_attachment_target(table, input.attachment, "modify_attachment_state", "attachment");
#endif
            id = require_attachment(table, input.attachment);
            count = input.count;
            round_usages = input.round_usages;
            context.stack().pop<modify_attachment_state_input>();
            context.enter_next();
        }
        const auto attachment = table[id];
        const bool valid = static_cast<bool>(attachment);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        const auto state = attachment.state();
        const auto limit = library[attachment.definition_id()].query(attachment_state_limit{});
        const auto add = [](std::uint32_t value, std::int64_t delta, std::uint32_t maximum) -> std::uint32_t
        {
            if(delta < -static_cast<std::int64_t>(value)) return 0;
            if(delta > static_cast<std::int64_t>(maximum) - value) return maximum;
            return static_cast<std::uint32_t>(static_cast<std::int64_t>(value) + delta);
        };
        return change_attachment_state(library, table, context, random, id, {
            add(state.count, count, limit.count),
            add(state.round_usages, round_usages, limit.round_usages)
        });
    }

    inline void compile(program_writer& writer, const givm::modify_attachment_state& command, compile_mode)
    {
        std::visit([&](auto selector)
        {
            using selector_type = decltype(selector);
            if constexpr(std::is_same_v<selector_type, definition_id<attachment_view>>)
            {
                if(not selector)
                {
                    writer.write(execute_fn{ execute_attachment_state_modification<> });
                    return;
                }
            }
            else GIVM_ASSERT(selector != equipment_type::none);
            GIVM_ASSERT(command.target.character.selection == character_selection::character);
            writer.write(execute_fn{ execute_attachment_state_modification<selector_type> });
            writer.write(attachment_state_modification_data<selector_type>{
                { command.target.character, selector }, command.count, command.round_usages });
        }, command.target.selector);
        writer.write(execute_fn{ finish_attachment_state_change });
    }
}

namespace givm
{
    inline std::vector<modify_attachment_state::error_type> check(const modify_attachment_state& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = modify_attachment_state::error_type::reason;
        std::vector<modify_attachment_state::error_type> errors;
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

#include "../../macro_undef.hpp"
#endif
