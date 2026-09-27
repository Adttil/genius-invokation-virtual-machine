#ifndef GIVM_EXECUTOR_COMMANDS_REMOVE_ATTACHMENT_HPP
#define GIVM_EXECUTOR_COMMANDS_REMOVE_ATTACHMENT_HPP

#include <algorithm>
#include <type_traits>
#include <variant>

#include "../broadcast.hpp"
#include "../character_target.hpp"
#include "../../definition.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<class Selector>
    struct fixed_attachment_target
    {
        relative_character_target character;
        Selector selector;
    };

    inline attachment_id require_attachment(
        const unrestricted_table& table, character_id character, equipment_type type)
    {
        GIVM_ASSERT(type != equipment_type::none);
        const auto target = table[character];
        GIVM_ASSERT(target.has(type));
        return target.get(type).id();
    }

    inline attachment_id require_attachment(
        const definition_library& library, const unrestricted_table& table,
        character_id character, definition_id<attachment_view> definition)
    {
        const auto type = library.equipment_type(definition);
        if(type != equipment_type::none)
        {
            const auto id = require_attachment(table, character, type);
            GIVM_ASSERT(table[id].definition_id() == definition);
            return id;
        }
        auto attachments = table[character].attachments();
        const auto found = std::ranges::find(attachments, definition,
            [](const auto attachment) { return attachment.definition_id(); });
        const bool exists = found != attachments.end();
        GIVM_ASSERT(exists);
        [[assume(exists)]];
        return (*found).id();
    }

    template<bool SkipDefeated = false, class Selector>
    inline attachment_id require_attachment(
        const definition_library& library, const unrestricted_table& table, const fixed_attachment_target<Selector>& target)
    {
        const auto character = resolve_character_target<SkipDefeated>(table, target.character);
        const bool exists = character.has_value();
        GIVM_ASSERT(exists);
        [[assume(exists)]];
        if constexpr(std::is_same_v<Selector, equipment_type>)
            return require_attachment(table, *character, target.selector);
        else
            return require_attachment(library, table, *character, target.selector);
    }

    inline attachment_id require_attachment(const unrestricted_table& table, const attachment_target& target)
    {
        if(const auto* id = std::get_if<attachment_id>(&target)) return *id;
        const auto* equipment = std::get_if<equipment_target>(&target);
        GIVM_ASSERT(equipment != nullptr);
        [[assume(equipment != nullptr)]];
        return require_attachment(table, equipment->character, equipment->type);
    }

    inline execution_state broadcast_attachment_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<attachment_removed>(library, table, context, random))
            return continue_execution;
        pop_broadcast<attachment_removed>(context);
        return context.enter_next();
    }

    template<class Selector = void>
    execution_state execute_attachment_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        attachment_id attachment;
        if constexpr(not std::is_void_v<Selector>)
        {
            const auto& target = context.instruction_data<1, fixed_attachment_target<Selector>>(library);
            attachment = require_attachment(library, table, target);
            context.advance(instruction_extent<1, fixed_attachment_target<Selector>>);
        }
        else
        {
            attachment = require_attachment(table, get<0>(context.stack().top<remove_attachment_input>()).attachment);
            context.stack().pop<remove_attachment_input>();
            context.enter_next();
        }
        const bool valid = static_cast<bool>(table[attachment]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        table[attachment].erase();
        prepare_broadcast(library, attachment_removed{ attachment }, table, context.stack(), context.position());
        return broadcast_attachment_removal(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::remove_attachment& command, compile_mode)
    {
        std::visit([&](auto selector)
        {
            using selector_type = decltype(selector);
            if constexpr(std::is_same_v<selector_type, definition_id<attachment_view>>)
            {
                if(not selector)
                {
                    writer.write(execute_fn{ execute_attachment_removal<> });
                    return;
                }
            }
            else GIVM_ASSERT(selector != equipment_type::none);
            GIVM_ASSERT(command.target.character.selection == character_selection::character);
            writer.write(execute_fn{ execute_attachment_removal<selector_type> });
            writer.write(fixed_attachment_target<selector_type>{ command.target.character, selector });
        }, command.target.selector);
        writer.write(execute_fn{ broadcast_attachment_removal });
    }
}

#include "../../macro_undef.hpp"
#endif
