#ifndef GIVM_EXECUTOR_COMMANDS_REMOVE_ATTACHMENT_HPP
#define GIVM_EXECUTOR_COMMANDS_REMOVE_ATTACHMENT_HPP

#include "../program_writer.hpp"

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <algorithm>
#include <type_traits>
#include <variant>

#include "../broadcast.hpp"
#include "../character_target.hpp"
#include <givm/definition.hpp>
#include <givm/macro_define.hpp>

namespace givm::detail
{
    template<class Selector>
    struct fixed_attachment_target
    {
        relative_character_target character;
        Selector selector;
    };

#ifndef NDEBUG
    inline void debug_validate_attachment_target(const unrestricted_table& table, const attachment_target& target,
        std::string_view command, std::string_view field)
    {
        std::visit([&](const auto& selection)
        {
            using selector_type = std::remove_cvref_t<decltype(selection)>;
            if constexpr(std::is_same_v<selector_type, attachment_id>)
                debug_validate_entity(table, selection, command, field);
            else
            {
                debug_validate_entity(table, selection.character, command, std::string{ field } + ".character");
                if(selection.type >= equipment_type::none)
                    throw command_input_error{ command, invalid_enum_argument{
                        std::string{ field } + ".type", static_cast<std::size_t>(selection.type) } };
                if(not table[selection.character].has(selection.type))
                    throw command_input_error{ command, missing_entity_argument{
                        std::string{ field }, command_entity_id{ selection.character }, {}, selection.type } };
            }
        }, target);
    }

    template<bool SkipDefeated = false, class Selector>
    inline void debug_validate_attachment_target(const definition_library& library, const unrestricted_table& table,
        const fixed_attachment_target<Selector>& target, std::string_view command, std::string_view field)
    {
        const auto character = resolve_character_target<SkipDefeated>(table, target.character);
        if(not character)
            throw command_input_error{ command, missing_entity_argument{ std::string{ field } + ".character" } };
        if constexpr(std::is_same_v<Selector, equipment_type>)
            debug_validate_attachment_target(table, attachment_target{ equipment_target{ *character, target.selector } }, command, field);
        else
        {
            debug_validate_definition(library, target.selector, command, std::string{ field } + ".definition");
            const auto owner = table[*character];
            const auto type = library.equipment_type(target.selector);
            const bool found = type != equipment_type::none
                ? owner.has(type) && owner.get(type).definition_id() == target.selector
                : std::ranges::any_of(owner.attachments(), [&](const auto entity) { return entity.definition_id() == target.selector; });
            if(not found)
                throw command_input_error{ command, missing_entity_argument{
                    std::string{ field }, command_entity_id{ *character }, target.selector.value() } };
        }
    }
#endif

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
        character_id character, definition_id<definition_category::attachment> definition)
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



    template<class Selector = void>
    execution_state execute_attachment_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        attachment_id attachment;
        if constexpr(not std::is_void_v<Selector>)
        {
            const auto& target = context.instruction_data<1, fixed_attachment_target<Selector>>(library);
#ifndef NDEBUG
            debug_validate_attachment_target(library, table, target, "remove_attachment", "target");
#endif
            attachment = require_attachment(library, table, target);
            context.advance(instruction_extent<1, fixed_attachment_target<Selector>>);
        }
        else
        {
#ifndef NDEBUG
            debug_validate_attachment_target(table, get<0>(context.stack().top<remove_attachment_input>()).attachment, "remove_attachment", "attachment");
#endif
            attachment = require_attachment(table, get<0>(context.stack().top<remove_attachment_input>()).attachment);
            context.stack().pop<remove_attachment_input>();
            context.enter_next();
        }
        const bool valid = static_cast<bool>(table[attachment]);
        GIVM_ASSERT(valid);
        [[assume(valid)]];
        table[attachment].erase();
        append_removal_record<this_attachment_remove>(context, attachment, attachment_removed{ attachment });
        return continue_execution;
    }

    inline void compile(program_writer& writer, const givm::remove_attachment& command, compile_mode)
    {
        std::visit([&](auto selector)
        {
            using selector_type = std::conditional_t<std::is_same_v<decltype(selector), optional_definition_id<definition_category::attachment>>,
                definition_id<definition_category::attachment>, decltype(selector)>;
            if constexpr(std::is_same_v<decltype(selector), optional_definition_id<definition_category::attachment>>)
            {
                if(not selector)
                {
                    writer.write(execute_fn{ execute_attachment_removal<> });
                    return;
                }
            }
            else GIVM_ASSERT(selector != equipment_type::none);
            const auto fixed_selector = [&]
            {
                if constexpr(std::is_same_v<decltype(selector), equipment_type>) return selector;
                else return selector.template get<definition_category::attachment>();
            }();
            GIVM_ASSERT(command.target.character.selection == character_selection::character);
            writer.write(execute_fn{ execute_attachment_removal<selector_type> });
            writer.write(fixed_attachment_target<selector_type>{ command.target.character, fixed_selector });
        }, command.target.selector);
    }
}

namespace givm::detail
{
    inline std::vector<remove_attachment::error_type> check(const remove_attachment& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = remove_attachment::error_type::reason;
        std::vector<remove_attachment::error_type> errors;
        const auto* definition = std::get_if<optional_definition_id<definition_category::attachment>>(&command.target.selector);
        if(definition && not *definition)
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(definition)
        {
            if(definition->get<definition_category::attachment>().value() >= context.definition_count<definition_category::attachment>())
                errors.push_back({ .cause = reason::invalid_definition, .value = definition->get<definition_category::attachment>().value(), .limit = context.definition_count<definition_category::attachment>() });
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

namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const remove_attachment& command) noexcept
    {
        const auto* definition = std::get_if<optional_definition_id<definition_category::attachment>>(&command.target.selector);
        return definition && not *definition ? TInputTypes::template index_of<remove_attachment::input_type>() : std::size_t(-1);
    }
}

#endif
