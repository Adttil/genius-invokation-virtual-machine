#ifndef GIVM_EXECUTOR_COMMANDS_HEAL_HPP
#define GIVM_EXECUTOR_COMMANDS_HEAL_HPP

#include <algorithm>
#include <limits>
#include <memory>
#include <optional>

#include "../executor.hpp"
#include "../character_target.hpp"
#include "../broadcast.hpp"
#include "../instruction.hpp"
#include "../../definition/commands.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    struct healing_target_cursor
    {
        player_id player;
        std::size_t next = 0;
        std::size_t remaining = 0;
        std::size_t count = 0;
    };

    struct healing_group
    {
        effect_source_id source;
        std::uint32_t value = 0;
        healing_target_cursor targets;
        std::size_t result_count = 0;
        std::size_t result_cursor = 0;
    };

    inline healing_target_cursor healing_targets(
        const unrestricted_table& table, character_id anchor, character_selection selection)
    {
        const auto count = table[anchor.player_id].template characters<false>().size();
        auto next = anchor.index;
        if(selection == character_selection::others && ++next == count) next = 0;
        return { anchor.player_id, next,
            selection == character_selection::character ? 1
                : count - (selection == character_selection::others), count };
    }

    inline std::optional<character_id> next_healing_target(
        const unrestricted_table& table, healing_target_cursor& cursor)
    {
        while(cursor.remaining != 0)
        {
            const character_id id{ cursor.player, cursor.next };
            --cursor.remaining;
            if(++cursor.next == cursor.count) cursor.next = 0;
            const auto target = table[id];
            if(target && target.state().health != 0) return id;
        }
        return std::nullopt;
    }

    inline execution_state broadcast_healing_completed(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<healed>(library, table, context, random)) return continue_execution;
        pop_broadcast<healed>(context);
        return context.enter_next();
    }

    inline execution_state apply_healing(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<healing>(library, table, context, random)) return continue_execution;
        const auto event = get<0>(context.stack().top<healing, response_return>());
        pop_broadcast<healing>(context);
        auto& state = table[event.target].state();
        GIVM_ASSERT(state.health <= state.max_health);
        [[assume(state.health <= state.max_health)]];
        const auto value = std::min(event.value, state.max_health - state.health);
        state.health += value;
        context.enter_next();
        prepare_broadcast(library, healed{ event.source, event.target, value }, table, context.stack(), context.position());
        return broadcast_healing_completed(library, table, context, random);
    }

    template<bool Dynamic>
    inline execution_state broadcast_healing_group(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        for(;;)
        {
            if(not continue_broadcast<healed>(library, table, context, random)) return continue_execution;
            pop_broadcast<healed>(context);
            const auto frame = context.stack().top<healing[], healing_group>();
            auto& group = get<1>(frame);
            if(++group.result_cursor == group.result_count)
            {
                context.stack().pop<healing[], healing_group>();
                return context.advance((Dynamic ? 3 : 1) * sizeof(execute_fn));
            }
            const auto current = get<0>(frame)[group.result_cursor];
            prepare_broadcast(library, healed{ current.source, current.target, current.value },
                table, context.stack(), context.position());
        }
    }

    template<bool Dynamic>
    inline execution_state apply_healing_group_target(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        for(;;)
        {
            if(not continue_broadcast<healing>(library, table, context, random)) return continue_execution;
            const auto event = get<0>(context.stack().top<healing, response_return>());
            pop_broadcast<healing>(context);
            auto& state = table[event.target].state();
            GIVM_ASSERT(state.health <= state.max_health);
            [[assume(state.health <= state.max_health)]];
            const auto value = std::min(event.value, state.max_health - state.health);
            state.health += value;

            const auto frame = context.stack().top<healing[], healing_group>();
            auto& group = get<1>(frame);
            std::construct_at(get<0>(frame).data() + group.result_count++,
                healing{ event.source, event.target, value });
            if(const auto target = next_healing_target(table, group.targets))
            {
                const auto next = healing{ group.source, *target, group.value };
                prepare_broadcast(library, next, table, context.stack(), context.position());
            }
            else
            {
                const auto first = get<0>(frame)[0];
                context.enter_next();
                prepare_broadcast(library, healed{ first.source, first.target, first.value },
                    table, context.stack(), context.position());
                return broadcast_healing_group<Dynamic>(library, table, context, random);
            }
        }
    }

    template<bool Fixed>
    inline execution_state prepare_healing_group(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        healing_application input;
        std::optional<character_id> anchor;
        character_selection selection;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, heal>(library);
            context.advance(instruction_extent<1, heal>);
            const auto source = resolve_character_target<false>(table, command.source);
            anchor = resolve_character_target<true>(table, command.target);
            if(not source || not anchor) return context.advance(2 * sizeof(execute_fn));
            input = { .source = *source, .target = command.target, .value = command.value };
            selection = command.target.selection;
        }
        else
        {
            input = get<0>(context.stack().top<healing_application>());
            context.stack().pop<healing_application>();
            context.enter_next();
            if(const auto* id = std::get_if<character_id>(&input.target))
            {
                anchor = table[*id] ? std::optional{ *id } : std::nullopt;
                selection = character_selection::character;
            }
            else
            {
                const auto& relative = std::get<relative_character_target>(input.target);
                selection = relative.selection;
                anchor = selection == character_selection::character
                    ? resolve_character_target<false>(table, relative)
                    : resolve_character_target<true>(table, relative);
            }
            if(not anchor) return context.advance(4 * sizeof(execute_fn));
            if(selection == character_selection::character)
            {
                // The dynamic command stores group continuations before the single-target pair.
                context.advance(2 * sizeof(execute_fn));
                prepare_broadcast(library, healing{ input.source, *anchor, input.value },
                    table, context.stack(), context.position());
                return apply_healing(library, table, context, random);
            }
        }

        const auto capacity = table[anchor->player_id].template characters<false>().size();
        auto targets = healing_targets(table, *anchor, selection);
        const auto first = next_healing_target(table, targets);
        if(not first) return context.advance((Fixed ? 2 : 4) * sizeof(execute_fn));
        context.stack().push(dynamic_array<healing>(capacity), healing_group{
            .source = input.source,
            .value = input.value,
            .targets = targets
        });
        prepare_broadcast(library, healing{ input.source, *first, input.value },
            table, context.stack(), context.position());
        return apply_healing_group_target<not Fixed>(library, table, context, random);
    }

    inline execution_state prepare_healing(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto& command = context.instruction_data<1, heal>(library);
        context.advance(instruction_extent<1, heal>);
        const auto source = resolve_character_target<false>(table, command.source);
        const auto target = resolve_character_target<false>(table, command.target);
        if(not source || not target) return context.advance(2 * sizeof(execute_fn));
        prepare_broadcast(library, healing{ *source, *target, command.value },
            table, context.stack(), context.position());
        return apply_healing(library, table, context, random);
    }

    inline void compile(program_writer& writer, const givm::heal& command, compile_mode)
    {
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
        {
            writer.write(execute_fn{ prepare_healing_group<false> });
            writer.write(execute_fn{ apply_healing_group_target<true> });
            writer.write(execute_fn{ broadcast_healing_group<true> });
            writer.write(execute_fn{ apply_healing });
            writer.write(execute_fn{ broadcast_healing_completed });
            return;
        }
        else if(command.target.selection != character_selection::character)
        {
            writer.write(execute_fn{ prepare_healing_group<true> });
            writer.write(command);
            writer.write(execute_fn{ apply_healing_group_target<false> });
            writer.write(execute_fn{ broadcast_healing_group<false> });
            return;
        }
        else
        {
            writer.write(execute_fn{ prepare_healing });
            writer.write(command);
        }
        writer.write(execute_fn{ apply_healing });
        writer.write(execute_fn{ broadcast_healing_completed });
    }
}

#include "../../macro_undef.hpp"
#endif
