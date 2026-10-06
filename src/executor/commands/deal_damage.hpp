#ifndef GIVM_EXECUTOR_COMMANDS_DEAL_DAMAGE_HPP
#define GIVM_EXECUTOR_COMMANDS_DEAL_DAMAGE_HPP

#include "apply_element.hpp"

#include <givm/macro_define.hpp>

namespace givm::detail
{
    constexpr std::uint32_t calculate_damage_value(std::uint32_t value, std::uint16_t numerator,
        std::uint16_t denominator) noexcept
    {
        const auto product = static_cast<std::uint64_t>(value) * numerator;
        const auto result = (product + denominator - 1) / denominator;
        return static_cast<std::uint32_t>(std::min<std::uint64_t>(result, std::numeric_limits<std::uint32_t>::max()));
    }

#ifndef NDEBUG
    inline void debug_validate_damage_values(damage_type type, std::uint16_t denominator)
    {
        if(type > damage_type::true_damage)
            throw command_input_error{ "deal_damage", invalid_enum_argument{ "type", static_cast<std::size_t>(type) } };
        if(denominator == 0)
            throw command_input_error{ "deal_damage", invalid_numeric_argument{ "multiplier_denominator", 0, 0,
                invalid_numeric_argument::constraint_kind::nonzero } };
    }
#endif

    struct damage_progress
    {
        damage input;
        execution_position instructions;
    };

    struct damage_batch_progress
    {
        std::size_t cursor = 0;
        execution_position instructions;
    };

    struct damage_hit
    {
        damage_source_id source;
        character_id target;
        damage_type type;
        reaction_id reaction;
        element_aura reacted_aura;
    };

    inline constexpr std::size_t damage_calculation_offset = response_extent<damage_preparation>;
    inline constexpr std::size_t damage_reaction_calculation_offset = damage_calculation_offset + response_extent<damage_calculation>;
    inline constexpr std::size_t damage_effect_offset = damage_reaction_calculation_offset + response_extent<damage_calculation>;
    inline constexpr std::size_t damage_health_resume_offset = damage_effect_offset + response_extent<damage_effect>;
    template<bool Observed>
    inline constexpr std::size_t damage_reaction_offset = damage_health_resume_offset + Observed * sizeof(execute_fn);
    template<bool Observed>
    inline constexpr std::size_t damage_finish_offset = damage_reaction_offset<Observed> + reaction_effect_extent;
    template<bool Observed>
    inline constexpr std::size_t damage_extent = damage_finish_offset<Observed> + sizeof(execute_fn);

    template<bool Fixed, bool Range, bool Observed>
    struct damage_driver
    {
        static execution_state finish_input(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            const auto instructions = get<0>(context.stack().top<damage_progress>()).instructions;
            context.stack().pop<damage_progress>();
            if constexpr(Range) context.stack().pop<character_id[], std::size_t>();
            if constexpr(not Fixed) return context.jump(instructions - sizeof(execute_fn));
            else return context.jump(instructions + damage_extent<Observed>);
        }

        static execution_state advance_target(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            if constexpr(Range) return next(library, table, context, random);
            else return finish_input(library, table, context, random);
        }

        static execution_state next(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            for(;;)
            {
                damage input;
                execution_position instructions;
                if constexpr(Range)
                {
                    const auto frames = context.stack().top<frame<character_id[], std::size_t>, frame<damage_progress>>();
                    const auto targets = get<0>(get<0>(frames));
                    auto& cursor = get<1>(get<0>(frames));
                    if(cursor == targets.size()) return finish_input(library, table, context, random);
                    auto& progress = get<0>(get<1>(frames));
                    progress.input.target = targets[cursor++];
                    input = progress.input;
                    instructions = progress.instructions;
                }
                else
                {
                    const auto progress = get<0>(context.stack().top<damage_progress>());
                    input = progress.input;
                    instructions = progress.instructions;
                }
                const auto target = std::get<character_id>(input.target);
                const auto character = table[target];
                if(not character || not character.state().alive)
                {
                    if constexpr(Range) continue;
                    else return finish_input(library, table, context, random);
                }
                context.jump(instructions);
                prepare_broadcast(library, damage_preparation{ input.source, target, input.value,
                    input.multiplier_numerator, input.multiplier_denominator, input.type, input.flags },
                    table, context.stack(), instructions);
                return prepare_calculation(library, table, context, random);
            }
        }

        static execution_state prepare_calculation(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            if(not continue_broadcast<damage_preparation>(library, table, context, random)) return continue_execution;
            const auto event = get<0>(context.stack().top<damage_preparation, response_return>());
            pop_broadcast<damage_preparation>(context);
            const auto character = table[event.target];
            if(not character || not character.state().alive)
                return advance_target(library, table, context, random);
            const auto aura = character.state().aura;
            const reaction_id reaction{ other_player(event.target.player_id),
                reaction_from_aura(aura, element_from_damage_type(event.type)) };
            const auto position = get<0>(context.stack().top<damage_progress>()).instructions + damage_calculation_offset;
            prepare_broadcast(library, damage_calculation{ event.source, event.target, event.value,
                event.multiplier_numerator, event.multiplier_denominator, event.type, event.flags, reaction, aura },
                table, context.stack(), position);
            context.jump(position);
            return apply_reaction_bonus(library, table, context, random);
        }

        static execution_state apply_reaction_bonus(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            if(not continue_broadcast<damage_calculation>(library, table, context, random)) return continue_execution;
            const auto event = get<0>(context.stack().top<damage_calculation, response_return>());
            pop_broadcast<damage_calculation>(context);
            if(not event.reaction || event.cancel_reaction_bonus)
                return prepare_effect(library, table, context, random, event);
            const auto position = get<0>(context.stack().top<damage_progress>()).instructions + damage_reaction_calculation_offset;
            prepare_single_response(event, event.reaction, table, context, position);
            context.jump(position);
            return complete_reaction_bonus(library, table, context, random);
        }

        static execution_state complete_reaction_bonus(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            if(not continue_single_response<damage_calculation, reaction_id>(library, table, context, random))
                return continue_execution;
            const auto event = get<1>(context.stack().top<single_response_progress<reaction_id>, damage_calculation, response_return>());
            pop_single_response<damage_calculation, reaction_id>(context);
            return prepare_effect(library, table, context, random, event);
        }

        static execution_state prepare_effect(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random, const damage_calculation& event)
        {
#ifndef NDEBUG
            debug_validate_damage_values(event.type, event.multiplier_denominator);
#endif
            const auto value = calculate_damage_value(event.value, event.multiplier_numerator, event.multiplier_denominator);
            const auto position = get<0>(context.stack().top<damage_progress>()).instructions + damage_effect_offset;
            context.stack().push(damage_hit{ event.source, event.target, event.type, event.reaction,
                event.reacted_aura });
            prepare_broadcast(library, damage_effect{ event.source, event.target, value, event.type, event.flags, event.reaction },
                table, context.stack(), position);
            context.jump(position);
            return apply(library, table, context, random);
        }

        static execution_state apply(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            if(not continue_broadcast<damage_effect>(library, table, context, random)) return continue_execution;
            const auto event = get<0>(context.stack().top<damage_effect, response_return>());
            pop_broadcast<damage_effect>(context);
            const auto character = table[event.target];
            if(character && character.state().alive)
            {
                auto& state = character.state();
                const auto health = state.health;
                state.health = health - std::min(health, event.value);
                record_damage(context, event, state.health == 0);
                if constexpr(Observed)
                {
                    if(state.health == health) return apply_reaction(library, table, context, random);
                    const auto progress = get<0>(get<0>(context.stack().top<frame<damage_progress>, frame<damage_hit>>()));
                    context.stack().push(event);
                    context.jump(progress.instructions + damage_health_resume_offset);
                    return context.yield(execution_state::health_reduced);
                }
            }
            return apply_reaction(library, table, context, random);
        }

        static execution_state resume_health(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            context.stack().pop<damage_effect>();
            return apply_reaction(library, table, context, random);
        }

        static execution_state apply_reaction(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            const auto frames = context.stack().top<frame<damage_progress>, frame<damage_hit>>();
            const auto progress = get<0>(get<0>(frames));
            const auto hit = get<0>(get<1>(frames));
            return reaction_driver::start(library, table, context, random, hit.source, hit.target,
                element_from_damage_type(hit.type), hit.reacted_aura, hit.reaction,
                element_application_cause::damage, progress.instructions + damage_reaction_offset<Observed>,
                progress.instructions + damage_finish_offset<Observed>);
        }

        static execution_state finish(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            context.stack().pop<damage_hit>();
            return advance_target(library, table, context, random);
        }

        static execution_state next_input(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random) requires (not Fixed)
        {
            const auto frames = context.stack().top<frame<damage[]>, frame<damage_batch_progress>>();
            const auto inputs = get<0>(get<0>(frames));
            auto& batch = get<0>(get<1>(frames));
            while(batch.cursor != inputs.size())
            {
                auto input = inputs[batch.cursor++];
#ifndef NDEBUG
                debug_validate_entity(table, input.source, "deal_damage", "source", true);
                if(const auto* id = std::get_if<character_id>(&input.target))
                    debug_validate_entity(table, *id, "deal_damage", "target", true);
                else debug_validate_relative_character_target(table,
                    std::get<relative_character_target>(input.target), "deal_damage", "target");
                if(input.selection != character_selection::character && input.selection != character_selection::others
                    && input.selection != character_selection::all && input.selection != character_selection::prioritized)
                    throw command_input_error{ "deal_damage", invalid_enum_argument{ "selection", static_cast<std::size_t>(input.selection) } };
                debug_validate_damage_values(input.type, input.multiplier_denominator);
#endif
                std::optional<character_id> anchor;
                auto selection = input.selection;
                if(const auto* id = std::get_if<character_id>(&input.target)) anchor = *id;
                else
                {
                    const auto relative = std::get<relative_character_target>(input.target);
                    anchor = resolve_damage_target(table, relative);
                    selection = relative.selection;
                }
                if(not anchor) continue;
                const auto targets = collect_character_targets(table, *anchor, selection);
                if(targets.empty()) continue;
                const auto instructions = batch.instructions;
                context.stack().push(dynamic_array<character_id>(targets), std::size_t{});
                context.stack().push(damage_progress{ input, instructions });
                return next(library, table, context, random);
            }
            const auto resume = batch.instructions + damage_extent<Observed>;
            context.stack().pop<damage_batch_progress>();
            context.stack().pop<damage[]>();
            return context.jump(resume);
        }

        static execution_state prepare(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            if constexpr(Fixed)
            {
                const auto command = context.instruction_data<1, deal_damage>(library);
                context.advance(instruction_extent<1, deal_damage>);
                const auto source = resolve_character_target<false>(table, command.source);
                const auto anchor = resolve_damage_target(table, command.target);
                if(not source || not anchor) return context.advance(damage_extent<Observed>);
                const damage input{ *source, *anchor, character_selection::character, command.value,
                    command.multiplier_numerator, command.multiplier_denominator, command.type, command.flags };
                if constexpr(Range)
                {
                    const auto targets = collect_character_targets(table, *anchor, command.target.selection);
                    if(targets.empty()) return context.advance(damage_extent<Observed>);
                    context.stack().push(dynamic_array<character_id>(targets), std::size_t{});
                }
                context.stack().push(damage_progress{ input, context.position() });
                return next(library, table, context, random);
            }
            else
            {
                context.enter_next();
                context.stack().push(damage_batch_progress{ 0, context.position() + sizeof(execute_fn) });
                return next_input(library, table, context, random);
            }
        }
    };

    template<bool Fixed, bool Range, bool Observed>
    void compile_damage(program_writer& writer, const deal_damage& command)
    {
        using driver = damage_driver<Fixed, Range, Observed>;
        writer.write(execute_fn{ driver::prepare });
        if constexpr(Fixed) writer.write(command);
        else writer.write(execute_fn{ driver::next_input });
        compile_broadcast<damage_preparation>(writer, driver::prepare_calculation);
        compile_broadcast<damage_calculation>(writer, driver::apply_reaction_bonus);
        compile_single_response<damage_calculation, reaction_id>(writer, driver::complete_reaction_bonus);
        compile_broadcast<damage_effect>(writer, driver::apply);
        if constexpr(Observed) writer.write(execute_fn{ driver::resume_health });
        compile_reaction_effect(writer);
        writer.write(execute_fn{ driver::finish });
    }

    template<bool Observed>
    void compile_damage(program_writer& writer, const deal_damage& command)
    {
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
            compile_damage<false, true, Observed>(writer, command);
        else if(command.target.selection == character_selection::others || command.target.selection == character_selection::all)
            compile_damage<true, true, Observed>(writer, command);
        else compile_damage<true, false, Observed>(writer, command);
    }

    inline void compile(program_writer& writer, const deal_damage& command, compile_mode mode)
    {
        if(mode == compile_mode::observed) compile_damage<true>(writer, command);
        else compile_damage<false>(writer, command);
    }

    inline std::vector<deal_damage::error_type> check(const deal_damage& command, const definition_compile_context&,
        program_kind kind)
    {
        using reason = deal_damage_error::reason;
        std::vector<deal_damage_error> errors;
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
        {
            if(kind != program_kind::response) errors.push_back({ reason::dynamic_input_in_root });
            return errors;
        }
        if(command.source.player != relative_player::self && command.source.player != relative_player::opponent)
            errors.push_back({ reason::invalid_source_player, static_cast<std::size_t>(command.source.player) });
        if(command.source.selection != character_selection::character)
            errors.push_back({ reason::invalid_source_selection, static_cast<std::size_t>(command.source.selection) });
        if(command.target.player != relative_player::self && command.target.player != relative_player::opponent)
            errors.push_back({ reason::invalid_target_player, static_cast<std::size_t>(command.target.player) });
        if(command.target.selection > character_selection::prioritized)
            errors.push_back({ reason::invalid_target_selection, static_cast<std::size_t>(command.target.selection) });
        if(command.multiplier_denominator == 0) errors.push_back({ reason::zero_multiplier_denominator });
        if(command.type > damage_type::true_damage)
            errors.push_back({ reason::invalid_damage_type, static_cast<std::size_t>(command.type) });
        return errors;
    }
}

#include <givm/macro_undef.hpp>

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const deal_damage& command) noexcept
    {
        return command.target.offset == std::numeric_limits<std::int32_t>::max()
            ? TInputTypes::template index_of<deal_damage_input>() : std::size_t(-1);
    }
}
#endif

#endif
