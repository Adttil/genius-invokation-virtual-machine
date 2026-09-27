#ifndef GIVM_EXECUTOR_COMMANDS_TRANSFER_ATTACHMENT_HPP
#define GIVM_EXECUTOR_COMMANDS_TRANSFER_ATTACHMENT_HPP

#include <optional>
#include <utility>

#include "../character_target.hpp"
#include "remove_attachment.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<class Selector>
    struct attachment_transfer_data
    {
        fixed_attachment_target<Selector> source;
        relative_character_target target;
    };

    template<class Selector = void, bool ResetRoundUsages = false>
    inline execution_state execute_attachment_transfer(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        attachment_id id;
        character_id target_id;
        bool reset_round_usages;
        if constexpr(not std::is_void_v<Selector>)
        {
            const auto& data = context.instruction_data<1, attachment_transfer_data<Selector>>(library);
            id = require_attachment<true>(library, table, data.source);
            const auto target = resolve_character_target<true>(table, data.target);
            const bool has_target = target.has_value();
            GIVM_ASSERT(has_target);
            [[assume(has_target)]];
            target_id = *target;
            reset_round_usages = ResetRoundUsages;
            context.advance(instruction_extent<1, attachment_transfer_data<Selector>>);
        }
        else
        {
            const auto& input = get<0>(context.stack().top<transfer_attachment_input>());
            id = require_attachment(table, input.attachment);
            target_id = input.target;
            reset_round_usages = input.reset_round_usages;
            context.stack().pop<transfer_attachment_input>();
            context.enter_next();
        }
        GIVM_ASSERT(table[id].is_valid());
        GIVM_ASSERT(table[target_id].is_valid() && table[target_id].state().health != 0);
        GIVM_ASSERT(id.character_id != target_id);

        const auto attachment = table[id];
        const auto target = table[target_id];
        const auto definition = attachment.definition_id();
        if(library.is_control(definition) && library.is_control_immune(std::as_const(table)[target_id]))
            return context.enter_next();

        auto state = attachment.state();
        if(reset_round_usages)
            state.round_usages = library[definition].query(attachment_state_limit{}).round_usages;
        const auto type = library.equipment_type(definition);

        std::optional<attachment_id> removed;
        if(type != equipment_type::none && target.has(type))
        {
            removed = target.get(type).id();
            table[*removed].erase();
        }
        attachment.erase();
        if(type == equipment_type::none)
            target.add(definition, state);
        else
            target.add(definition, state, type);

        // Removal responses observe the completed transfer and cannot be overwritten by it.
        if(removed)
        {
            prepare_broadcast(library, attachment_removed{ *removed }, table, context.stack(), context.position());
            return broadcast_attachment_removal(library, table, context, random);
        }
        return context.enter_next();
    }

    inline void compile(program_writer& writer, const transfer_attachment& command, compile_mode)
    {
        std::visit([&](auto selector)
        {
            using selector_type = decltype(selector);
            if constexpr(std::is_same_v<selector_type, definition_id<attachment_view>>)
            {
                if(not selector)
                {
                    writer.write(execute_fn{ execute_attachment_transfer<> });
                    return;
                }
            }
            else GIVM_ASSERT(selector != equipment_type::none);
            GIVM_ASSERT(command.source.character.selection == character_selection::character);
            GIVM_ASSERT(command.target.selection == character_selection::character);
            writer.write(command.reset_round_usages
                ? execute_fn{ execute_attachment_transfer<selector_type, true> }
                : execute_fn{ execute_attachment_transfer<selector_type, false> });
            writer.write(attachment_transfer_data<selector_type>{
                { command.source.character, selector }, command.target });
        }, command.source.selector);
        writer.write(execute_fn{ broadcast_attachment_removal });
    }
}

#include "../../macro_undef.hpp"
#endif
