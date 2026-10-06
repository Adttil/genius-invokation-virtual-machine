#ifndef GIVM_EXECUTOR_COMMANDS_TRANSFER_ATTACHMENT_HPP
#define GIVM_EXECUTOR_COMMANDS_TRANSFER_ATTACHMENT_HPP

#include "../program_writer.hpp"

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <optional>
#include <utility>

#include "../character_target.hpp"
#include "remove_attachment.hpp"
#include <givm/macro_define.hpp>

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
#ifndef NDEBUG
            debug_validate_attachment_target<true>(library, table, data.source, "transfer_attachment", "source");
#endif
            id = require_attachment<true>(library, table, data.source);
            const auto target = resolve_character_target<true>(table, data.target);
            const bool has_target = target.has_value();
#ifndef NDEBUG
            if(not has_target)
                throw command_input_error{ "transfer_attachment", missing_entity_argument{ "target" } };
#endif
            GIVM_ASSERT(has_target);
            [[assume(has_target)]];
            target_id = *target;
            reset_round_usages = ResetRoundUsages;
            context.advance(instruction_extent<1, attachment_transfer_data<Selector>>);
        }
        else
        {
            const auto& input = get<0>(context.stack().top<transfer_attachment_input>());
#ifndef NDEBUG
            debug_validate_attachment_target(table, input.attachment, "transfer_attachment", "attachment");
#endif
            id = require_attachment(table, input.attachment);
            target_id = input.target;
            reset_round_usages = input.reset_round_usages;
            context.stack().pop<transfer_attachment_input>();
            context.enter_next();
        }
#ifndef NDEBUG
        debug_validate_entity(table, target_id, "transfer_attachment", "target");
        if(table[target_id].state().health == 0)
            throw command_input_error{ "transfer_attachment", invalid_entity_relation{
                "target", invalid_entity_relation::reason::defeated_character } };
        if(id.character_id == target_id)
            throw command_input_error{ "transfer_attachment", invalid_entity_relation{
                "target", invalid_entity_relation::reason::same_character } };
#endif
        GIVM_ASSERT(table[id].is_valid());
        GIVM_ASSERT(table[target_id].is_valid() && table[target_id].state().health != 0);
        GIVM_ASSERT(id.character_id != target_id);

        const auto attachment = table[id];
        const auto target = table[target_id];
        const auto definition = attachment.definition_id();
        if(library.is_control(definition) && library.is_control_immune(std::as_const(table)[target_id]))
            return continue_execution;

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

        if(removed)
            append_removal_record<attachment_removal_effect>(context, *removed, attachment_removed{ *removed });
        return continue_execution;
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
    }
}

namespace givm::detail
{
    inline std::vector<transfer_attachment::error_type> check(const transfer_attachment& command,
        const definition_compile_context& context, program_kind kind)
    {
        using reason = transfer_attachment::error_type::reason;
        std::vector<transfer_attachment::error_type> errors;
        const auto* definition = std::get_if<definition_id<attachment_view>>(&command.source.selector);
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
            const auto equipment = std::get<equipment_type>(command.source.selector);
            if(equipment >= equipment_type::none)
                errors.push_back({ .cause = reason::invalid_equipment_type, .value = static_cast<std::size_t>(equipment) });
        }
        if(command.source.character.player != relative_player::self && command.source.character.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_source_character_player, .value = static_cast<std::size_t>(command.source.character.player) });
        if(command.source.character.selection != character_selection::character)
            errors.push_back({ .cause = reason::invalid_source_character_selection, .value = static_cast<std::size_t>(command.source.character.selection) });
        if(command.target.player != relative_player::self && command.target.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_target_player, .value = static_cast<std::size_t>(command.target.player) });
        if(command.target.selection != character_selection::character)
            errors.push_back({ .cause = reason::invalid_target_selection, .value = static_cast<std::size_t>(command.target.selection) });
        return errors;
    }
}

#include <givm/macro_undef.hpp>

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const transfer_attachment& command) noexcept
    {
        const auto* definition = std::get_if<definition_id<attachment_view>>(&command.source.selector);
        return definition && not *definition ? TInputTypes::template index_of<transfer_attachment::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
