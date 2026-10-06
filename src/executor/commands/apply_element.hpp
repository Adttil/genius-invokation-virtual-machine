#ifndef GIVM_EXECUTOR_COMMANDS_APPLY_ELEMENT_HPP
#define GIVM_EXECUTOR_COMMANDS_APPLY_ELEMENT_HPP

#include "../broadcast.hpp"
#include "../character_target.hpp"
#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <givm/macro_define.hpp>

namespace givm::detail
{
    struct element_application_frame
    {
        after_elemental_reaction event;
        execution_position resume;
    };

    inline constexpr std::size_t reaction_effect_extent = 2 * response_extent<elemental_reaction_will_occur>;

    struct reaction_driver
    {
        static execution_state complete(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            if(not continue_single_response<elemental_reaction_will_occur, reaction_id>(library, table, context, random))
                return continue_execution;
            pop_single_response<elemental_reaction_will_occur, reaction_id>(context);
            return finish(context);
        }

        static execution_state finish(execution_context& context)
        {
            const auto frame = get<0>(context.stack().top<element_application_frame>());
            append_event_record(context, frame.event);
            context.stack().pop<element_application_frame>();
            return context.jump(frame.resume);
        }

        static execution_state apply(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            if(not continue_broadcast<elemental_reaction_will_occur>(library, table, context, random))
                return continue_execution;
            const auto event = get<0>(context.stack().top<elemental_reaction_will_occur, response_return>());
            pop_broadcast<elemental_reaction_will_occur>(context);
            const auto target = table[event.target];
            if(target && target.state().alive) target.state().aura = event.new_aura;
            if(event.cancel_default_effects) return finish(context);
            context.advance(response_extent<elemental_reaction_will_occur>);
            prepare_single_response(event, event.reaction, table, context, context.position());
            return complete(library, table, context, random);
        }

        static execution_state start(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random, const element_application_source_id& source,
            character_id target, element incoming, element_aura aura, reaction_id reaction,
            element_application_cause cause, execution_position position, execution_position resume)
        {
            const auto character = table[target];
            if(not character || not character.state().alive) return context.jump(resume);
            if(incoming == element::none)
            {
                if(cause == element_application_cause::effect) character.state().aura = element_aura::none;
                return context.jump(resume);
            }
            if(not reaction)
            {
                character.state().aura = aura_without_reaction(aura, incoming);
                return context.jump(resume);
            }
            const auto new_aura = library[table[reaction].definition_id()].query(reaction_aura{ reaction.slot, aura, incoming });
            context.stack().push(element_application_frame{
                { source, target, incoming, aura, reaction, cause }, resume });
            prepare_broadcast(library, elemental_reaction_will_occur{
                source, target, incoming, aura, reaction, cause, new_aura }, table, context.stack(), position);
            context.jump(position);
            return apply(library, table, context, random);
        }
    };

    template<bool Fixed>
    inline execution_state prepare_element_application(const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        apply_element_input input;
        if constexpr(Fixed)
        {
            const auto command = context.instruction_data<1, apply_element>(library);
            context.advance(instruction_extent<1, apply_element>);
            const auto source = resolve_character_target<false>(table, command.source);
            const auto target = resolve_character_target<false>(table, command.target);
            if(not source || not target) return context.advance(reaction_effect_extent);
            input = { *source, *target, command.element, command.cause };
        }
        else
        {
            input = get<0>(context.stack().top<apply_element_input>());
#ifndef NDEBUG
            debug_validate_entity(table, input.source, "apply_element", "source", true);
            debug_validate_entity(table, input.target, "apply_element", "target", true);
            if(input.element > element::none)
                throw command_input_error{ "apply_element", invalid_enum_argument{ "element", static_cast<std::size_t>(input.element) } };
            if(input.cause != element_application_cause::effect && input.cause != element_application_cause::damage)
                throw command_input_error{ "apply_element", invalid_enum_argument{ "cause", static_cast<std::size_t>(input.cause) } };
#endif
            context.stack().pop<apply_element_input>();
            context.enter_next();
        }
        const auto target = table[input.target];
        if(not target || not target.state().alive) return context.advance(reaction_effect_extent);
        const auto aura = target.state().aura;
        const reaction_id reaction{ other_player(input.target.player_id), reaction_from_aura(aura, input.element) };
        return reaction_driver::start(library, table, context, random, input.source, input.target,
            input.element, aura, reaction, input.cause, context.position(), context.position() + reaction_effect_extent);
    }

    inline void compile_reaction_effect(program_writer& writer)
    {
        compile_broadcast<elemental_reaction_will_occur>(writer, reaction_driver::apply);
        compile_single_response<elemental_reaction_will_occur, reaction_id>(writer, reaction_driver::complete);
    }

    inline void compile(program_writer& writer, const givm::apply_element& command, compile_mode)
    {
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
            writer.write(execute_fn{ prepare_element_application<false> });
        else
        {
            writer.write(execute_fn{ prepare_element_application<true> });
            writer.write(command);
        }
        compile_reaction_effect(writer);
    }
}

namespace givm::detail
{
    inline std::vector<apply_element::error_type> check(const apply_element& command, const definition_compile_context&, program_kind kind)
    {
        using reason = apply_element::error_type::reason;
        std::vector<apply_element::error_type> errors;
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.source.player != relative_player::self && command.source.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_source_player, .value = static_cast<std::size_t>(command.source.player) });
        if(command.source.selection != character_selection::character)
            errors.push_back({ .cause = reason::invalid_source_selection, .value = static_cast<std::size_t>(command.source.selection) });
        if(command.target.player != relative_player::self && command.target.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_target_player, .value = static_cast<std::size_t>(command.target.player) });
        if(command.target.selection != character_selection::character)
            errors.push_back({ .cause = reason::invalid_target_selection, .value = static_cast<std::size_t>(command.target.selection) });
        if(command.element > element::none)
            errors.push_back({ .cause = reason::invalid_element, .value = static_cast<std::size_t>(command.element) });
        if(command.cause != element_application_cause::effect && command.cause != element_application_cause::damage)
            errors.push_back({ .cause = reason::invalid_cause, .value = static_cast<std::size_t>(command.cause) });
        return errors;
    }
}

#include <givm/macro_undef.hpp>

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const apply_element& command) noexcept
    {
        return command.target.offset == std::numeric_limits<std::int32_t>::max() ? TInputTypes::template index_of<apply_element::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
