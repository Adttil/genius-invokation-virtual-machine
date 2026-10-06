#ifndef GIVM_EXECUTOR_COMMANDS_HEAL_HPP
#define GIVM_EXECUTOR_COMMANDS_HEAL_HPP

#include "../broadcast.hpp"
#include "../character_target.hpp"
#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <givm/macro_define.hpp>

namespace givm::detail
{
    struct healing_progress
    {
        heal_input::item input;
        execution_position instructions;
    };

    struct healing_batch_progress
    {
        std::size_t cursor = 0;
        execution_position instructions;
    };

    inline bool can_heal(const character_state& state, healing_kind kind) noexcept
    {
        switch(kind)
        {
        case healing_kind::normal: return state.alive && state.health != 0;
        case healing_kind::prevent_defeat: return state.alive && state.health == 0;
        case healing_kind::revive: return not state.alive;
        }
        std::unreachable();
    }

    inline void apply_healing_result(execution_context& context, unrestricted_table& table,
        const effect_source_id& source, character_id target, std::uint32_t value, healing_kind kind)
    {
        const auto character = table[target];
        if(not character || not can_heal(character.state(), kind)) return;
        auto& state = character.state();
        value = std::min(value, state.max_health - state.health);
        state.health += value;
        if(kind == healing_kind::revive && value != 0)
        {
            state.alive = true;
            append_event_record(context, character_revived{ target });
        }
        append_event_record(context, healed{ source, target, value, kind });
    }

    template<bool Fixed, bool Range>
    struct healing_driver
    {
        static execution_state finish_input(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            const auto instructions = get<0>(context.stack().top<healing_progress>()).instructions;
            context.stack().pop<healing_progress>();
            if constexpr(Range) context.stack().pop<character_id[], std::size_t>();
            if constexpr(not Fixed) return context.jump(instructions - sizeof(execute_fn));
            else return context.jump(instructions + response_extent<healing>);
        }

        static execution_state next(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            for(;;)
            {
                heal_input::item input;
                execution_position instructions;
                if constexpr(Range)
                {
                    const auto frames = context.stack().top<frame<character_id[], std::size_t>, frame<healing_progress>>();
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
                    const auto progress = get<0>(context.stack().top<healing_progress>());
                    input = progress.input;
                    instructions = progress.instructions;
                }
                const auto target = std::get<character_id>(input.target);
                const auto character = table[target];
                if(character && can_heal(character.state(), input.kind))
                {
                    if(input.kind == healing_kind::normal)
                    {
                        context.jump(instructions);
                        prepare_broadcast(library, healing{ input.source, target, input.value },
                            table, context.stack(), instructions);
                        return apply(library, table, context, random);
                    }
                    apply_healing_result(context, table, input.source, target, input.value, input.kind);
                }
                if constexpr(not Range) return finish_input(library, table, context, random);
            }
        }

        static execution_state apply(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            if(not continue_broadcast<healing>(library, table, context, random)) return continue_execution;
            const auto event = get<0>(context.stack().top<healing, response_return>());
            pop_broadcast<healing>(context);
            apply_healing_result(context, table, event.source, event.target, event.value, healing_kind::normal);
            if constexpr(Range) return next(library, table, context, random);
            else return finish_input(library, table, context, random);
        }

        static execution_state next_input(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random) requires (not Fixed)
        {
            const auto frames = context.stack().top<frame<heal_input::item[]>, frame<healing_batch_progress>>();
            const auto inputs = get<0>(get<0>(frames));
            auto& batch = get<0>(get<1>(frames));
            while(batch.cursor != inputs.size())
            {
                auto input = inputs[batch.cursor++];
#ifndef NDEBUG
                debug_validate_entity(table, input.source, "heal", "source", true);
                if(const auto* id = std::get_if<character_id>(&input.target))
                    debug_validate_entity(table, *id, "heal", "target", true);
                else
                {
                    const auto relative = std::get<relative_character_target>(input.target);
                    debug_validate_relative_character_target(table, relative, "heal", "target");
                    if(relative.selection == character_selection::prioritized)
                        throw command_input_error{ "heal", invalid_enum_argument{ "target.selection", static_cast<std::size_t>(relative.selection) } };
                }
#endif
                std::optional<character_id> anchor;
                auto selection = character_selection::character;
                if(const auto* id = std::get_if<character_id>(&input.target)) anchor = *id;
                else
                {
                    const auto relative = std::get<relative_character_target>(input.target);
                    anchor = resolve_character_target<false>(table, relative);
                    selection = relative.selection;
                }
                if(not anchor) continue;
                const auto targets = collect_character_targets(table, *anchor, selection, input.kind == healing_kind::revive);
                if(targets.empty()) continue;
                const auto instructions = batch.instructions;
                context.stack().push(dynamic_array<character_id>(targets), std::size_t{});
                context.stack().push(healing_progress{ input, instructions });
                return next(library, table, context, random);
            }
            const auto resume = batch.instructions + response_extent<healing>;
            context.stack().pop<healing_batch_progress>();
            context.stack().pop<heal_input::item[]>();
            return context.jump(resume);
        }

        static execution_state prepare(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            if constexpr(Fixed)
            {
                const auto command = context.instruction_data<1, heal>(library);
                context.advance(instruction_extent<1, heal>);
                const auto source = resolve_character_target<false>(table, command.source);
                const auto anchor = resolve_character_target<false>(table, command.target);
                if(not source || not anchor) return context.advance(response_extent<healing>);
                if constexpr(Range)
                {
                    const auto targets = collect_character_targets(table, *anchor, command.target.selection, command.kind == healing_kind::revive);
                    if(targets.empty()) return context.advance(response_extent<healing>);
                    context.stack().push(dynamic_array<character_id>(targets), std::size_t{});
                }
                context.stack().push(healing_progress{ { *source, *anchor, command.value, command.kind }, context.position() });
                return next(library, table, context, random);
            }
            else
            {
                context.enter_next();
                context.stack().push(healing_batch_progress{ 0, context.position() + sizeof(execute_fn) });
                return next_input(library, table, context, random);
            }
        }
    };

    template<bool Fixed, bool Range>
    void compile_healing(program_writer& writer, const heal& command)
    {
        using driver = healing_driver<Fixed, Range>;
        writer.write(execute_fn{ driver::prepare });
        if constexpr(Fixed) writer.write(command);
        else writer.write(execute_fn{ driver::next_input });
        compile_broadcast<healing>(writer, driver::apply);
    }

    inline void compile(program_writer& writer, const givm::heal& command, compile_mode)
    {
        if(command.target.offset == std::numeric_limits<std::int32_t>::max()) compile_healing<false, true>(writer, command);
        else if(command.target.selection == character_selection::others || command.target.selection == character_selection::all)
            compile_healing<true, true>(writer, command);
        else compile_healing<true, false>(writer, command);
    }
}

namespace givm::detail
{
    inline std::vector<heal::error_type> check(const heal& command, const definition_compile_context&, program_kind kind)
    {
        using reason = heal::error_type::reason;
        std::vector<heal::error_type> errors;
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
        if(command.target.selection != character_selection::character && command.target.selection != character_selection::others
            && command.target.selection != character_selection::all)
            errors.push_back({ .cause = reason::invalid_target_selection, .value = static_cast<std::size_t>(command.target.selection) });
        return errors;
    }
}

#include <givm/macro_undef.hpp>

namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const heal& command) noexcept
    {
        return command.target.offset == std::numeric_limits<std::int32_t>::max() ? TInputTypes::template index_of<heal::input_type>() : std::size_t(-1);
    }
}

#endif
