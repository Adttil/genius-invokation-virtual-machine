#ifndef GIVM_EXECUTOR_COMMANDS_APPLY_ELEMENT_HPP
#define GIVM_EXECUTOR_COMMANDS_APPLY_ELEMENT_HPP

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <utility>

#include "../broadcast.hpp"
#include "../character_target.hpp"
#include "attach.hpp"
#include "generate_combat_status.hpp"
#include "summon.hpp"
#include "set_active_character.hpp"
#include "../../definition.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline std::optional<execution_state> prepare_default_reaction_entities(
        const definition_library& library, unrestricted_table& table, execution_context& context,
        random_fn& random, elemental_reaction reaction, character_id target,
        execution_position entity_resume, execution_position attachment_resume)
    {
        if(reaction == elemental_reaction::quicken || reaction == elemental_reaction::burning
            || reaction == elemental_reaction::bloom)
        {
            const auto player = other_player(target.player_id);
            program_entry entry;
            if(reaction == elemental_reaction::burning)
                entry = prepare_summoning(library, table, context, random,
                    { .player = player, .definition = library.burning_flame_id(), .state = { 1, 1 } }, entity_resume);
            else
            {
                const auto definition = reaction == elemental_reaction::quicken
                    ? library.catalyzing_field_id() : library.dendro_core_id();
                entry = prepare_combat_status_generation(library, table, context, random,
                    { .player = player, .definition = definition }, entity_resume);
            }
            if(entry) return context.enter(entry);
        }
        else if(reaction == elemental_reaction::frozen && table[target].state().health != 0)
            return prepare_attachment_application(library, table, context, random,
                { .target = target, .definition = library.frozen_id() }, entity_resume, attachment_resume);
        return std::nullopt;
    }

    inline std::optional<execution_state> continue_reaction_overloaded_switch(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<active_character_changed>(library, table, context, random)) return continue_execution;
        pop_broadcast<active_character_changed>(context);
        return std::nullopt;
    }

    inline std::optional<execution_state> resume_reaction_overloaded_observation(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto frame = context.stack().top<active_character_changed, response_return>();
        const auto event = get<0>(frame);
        const auto position = get<1>(frame).position;
        context.stack().pop<active_character_changed, response_return>();
        if(const auto state = prepare_active_character_switch(library, table, context, random,
            event, position - 2 * sizeof(execute_fn), position)) return *state;
        return continue_reaction_overloaded_switch(library, table, context, random);
    }

    template<bool Observed>
    inline std::optional<execution_state> prepare_reaction_overloaded_switch(
        const definition_library& library, unrestricted_table& table, execution_context& context,
        random_fn& random, player_id affected_player, execution_position removal_resume,
        execution_position observation_resume, execution_position broadcast_resume)
    {
        const auto player = table[affected_player];
        const auto characters = player.template characters<false>();
        GIVM_ASSERT(player.state().active_character.has_value());
        const auto current = *player.state().active_character;
        if(library.is_control_immune(std::as_const(table)[current])) return std::nullopt;
        auto index = current.index;
        for(std::size_t remaining = characters.size(); remaining != 0; --remaining)
        {
            if(++index == characters.size()) index = 0;
            const auto target = characters[index];
            if(not target || target.state().health == 0) continue;
            if(target.id() == current) break;

            const active_character_changed event{ .current = target.id() };
            if constexpr(Observed)
            {
                // Collect responders after applying the change, in both modes.
                context.stack().push(event, response_return{ table.state().self_player, broadcast_resume });
                context.jump(observation_resume);
                return context.yield(execution_state::active_character_changed);
            }
            else
            {
                if(const auto state = prepare_active_character_switch(library, table, context, random,
                    event, removal_resume, broadcast_resume)) return *state;
                return continue_reaction_overloaded_switch(library, table, context, random);
            }
        }
        return std::nullopt;
    }

    struct element_application_frame
    {
        after_elemental_reaction event;
        execution_position instructions;
    };

    inline constexpr std::size_t element_reaction_offset = 0;
    inline constexpr std::size_t element_entity_resume_offset = 1;
    inline constexpr std::size_t element_attachment_resume_offset = 2;
    inline constexpr std::size_t element_overloaded_removal_offset = 3;
    inline constexpr std::size_t element_overloaded_observation_offset = 4;
    template<bool Observed>
    inline constexpr std::size_t element_overloaded_broadcast_offset = 4 + Observed;
    template<bool Observed>
    inline constexpr std::size_t element_after_reaction_offset = 5 + Observed;
    template<bool Observed>
    inline constexpr std::size_t element_end_offset = 6 + Observed;

    template<bool Observed>
    inline execution_state continue_element_application_completion(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<after_elemental_reaction>(library, table, context, random)) return continue_execution;
        pop_broadcast<after_elemental_reaction>(context);
        const auto position = get<0>(context.stack().top<element_application_frame>()).instructions;
        context.stack().pop<element_application_frame>();
        return context.jump(position + element_end_offset<Observed> * sizeof(execute_fn));
    }

    template<bool Observed>
    inline execution_state prepare_element_application_completion(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto frame = get<0>(context.stack().top<element_application_frame>());
        prepare_broadcast(library, frame.event, table, context.stack(),
            frame.instructions + element_after_reaction_offset<Observed> * sizeof(execute_fn));
        return continue_element_application_completion<Observed>(library, table, context, random);
    }

    template<bool Observed>
    inline execution_state resume_element_entity_generation(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        context.stack().pop<response_return>();
        return prepare_element_application_completion<Observed>(library, table, context, random);
    }

    template<bool Observed>
    inline execution_state resume_element_attachment_replacement(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(const auto state = continue_attachment_replacement(library, table, context, random)) return *state;
        return prepare_element_application_completion<Observed>(library, table, context, random);
    }

    template<bool Observed>
    inline execution_state continue_element_overloaded_switch(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(const auto state = continue_reaction_overloaded_switch(library, table, context, random)) return *state;
        return prepare_element_application_completion<Observed>(library, table, context, random);
    }

    template<bool Observed>
    inline execution_state resume_element_overloaded_removal(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(const auto state = continue_switch_prepared_removal(library, table, context, random)) return *state;
        return continue_element_overloaded_switch<Observed>(library, table, context, random);
    }

    inline execution_state resume_element_overloaded_observation(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(const auto state = resume_reaction_overloaded_observation(library, table, context, random)) return *state;
        return prepare_element_application_completion<true>(library, table, context, random);
    }

    template<bool Observed>
    inline execution_state continue_element_application_reaction(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<elemental_reaction_will_occur>(library, table, context, random)) return continue_execution;
        const auto event = get<0>(context.stack().top<elemental_reaction_will_occur, response_return>());
        pop_broadcast<elemental_reaction_will_occur>(context);
        auto& frame = get<0>(context.stack().top<element_application_frame>());
        std::construct_at(&frame.event, after_elemental_reaction{
            .source = event.source, .target = event.target, .incoming_element = event.incoming_element,
            .reacted_aura = event.reacted_aura, .reaction = event.reaction, .cause = event.cause,
            .replacement_reaction = event.replacement_reaction
        });
        // Replacement changes effects, never the reaction's aura consumption.
        table[event.target].state().aura = aura_after_reaction(event.reacted_aura, event.incoming_element, event.reaction);
        if(not event.replacement_reaction)
        {
            if(event.reaction == elemental_reaction::overloaded
                && table[event.target.player_id].state().active_character == event.target)
            {
                if(const auto state = prepare_reaction_overloaded_switch<Observed>(library, table, context, random,
                    event.target.player_id, frame.instructions + element_overloaded_removal_offset * sizeof(execute_fn),
                    frame.instructions + element_overloaded_observation_offset * sizeof(execute_fn),
                    frame.instructions + element_overloaded_broadcast_offset<Observed> * sizeof(execute_fn))) return *state;
            }
            else if(const auto state = prepare_default_reaction_entities(library, table, context, random,
                event.reaction, event.target, frame.instructions + element_entity_resume_offset * sizeof(execute_fn),
                frame.instructions + element_attachment_resume_offset * sizeof(execute_fn))) return *state;
        }
        return prepare_element_application_completion<Observed>(library, table, context, random);
    }

    template<bool Fixed, bool Observed>
    inline execution_state prepare_element_application(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        element_application input;
        execution_position position;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, givm::apply_element>(library);
            position = context.position() + instruction_extent<1, givm::apply_element>;
            const auto source = resolve_character_target<false>(table, command.source);
            const auto target = resolve_character_target<false>(table, command.target);
            if(not source || not target)
                return context.jump(position + element_end_offset<Observed> * sizeof(execute_fn));
            input = { .source = *source, .target = *target, .element = command.element, .cause = command.cause };
        }
        else
        {
            input = get<0>(context.stack().top<element_application>());
            context.stack().pop<element_application>();
            position = context.position() + sizeof(execute_fn);
        }
        const auto target = table[input.target];
        if(not target) return context.jump(position + element_end_offset<Observed> * sizeof(execute_fn));
        const auto aura = target.state().aura;
        const auto reaction = reaction_from_aura(aura, input.element);
        if(reaction == elemental_reaction::none)
        {
            target.state().aura = aura_without_reaction(aura, input.element);
            return context.jump(position + element_end_offset<Observed> * sizeof(execute_fn));
        }
        context.stack().push(element_application_frame{ .event = {
            .source = input.source, .target = input.target, .incoming_element = input.element,
            .reacted_aura = aura, .reaction = reaction, .cause = input.cause
        }, .instructions = position });
        prepare_broadcast(library, elemental_reaction_will_occur{
            .source = input.source, .target = input.target, .incoming_element = input.element,
            .reacted_aura = aura, .reaction = reaction, .cause = input.cause
        }, table, context.stack(), position + element_reaction_offset * sizeof(execute_fn));
        return continue_element_application_reaction<Observed>(library, table, context, random);
    }

    template<bool Fixed, bool Observed>
    inline void compile_element_application(program_writer& writer, const givm::apply_element& command)
    {
        writer.write(execute_fn{ prepare_element_application<Fixed, Observed> });
        if constexpr(Fixed) writer.write(command);
        writer.write(execute_fn{ continue_element_application_reaction<Observed> });
        writer.write(execute_fn{ resume_element_entity_generation<Observed> });
        writer.write(execute_fn{ resume_element_attachment_replacement<Observed> });
        writer.write(execute_fn{ resume_element_overloaded_removal<Observed> });
        if constexpr(Observed) writer.write(execute_fn{ resume_element_overloaded_observation });
        writer.write(execute_fn{ continue_element_overloaded_switch<Observed> });
        writer.write(execute_fn{ continue_element_application_completion<Observed> });
    }

    inline void compile(program_writer& writer, const givm::apply_element& command, compile_mode mode)
    {
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
        {
            if(mode == compile_mode::observed) compile_element_application<false, true>(writer, command);
            else compile_element_application<false, false>(writer, command);
        }
        else
        {
            if(mode == compile_mode::observed) compile_element_application<true, true>(writer, command);
            else compile_element_application<true, false>(writer, command);
        }
    }
}

#include "../../macro_undef.hpp"
#endif
