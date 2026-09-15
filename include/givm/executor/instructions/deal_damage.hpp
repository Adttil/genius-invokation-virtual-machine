#ifndef GIVM_EXECUTOR_INSTRUCTIONS_DEAL_DAMAGE_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_DEAL_DAMAGE_HPP

#include "../executor.hpp"

#include <cstdint>
#include <limits>

#include "../broadcast.hpp"
#include "../../definition/events.hpp"
#include "../instruction.hpp"
#include "../../definition/commands.hpp"
#include "apply_element.hpp"

namespace givm::detail
{
    constexpr void add_reaction_damage_bonus(damage_calculation& event, std::uint32_t value) noexcept
    {
        const auto max_value = std::numeric_limits<std::uint32_t>::max();
        event.value = max_value - event.value < value ? max_value : event.value + value;
    }

    constexpr void apply_default_damage_reaction_effect(damage_calculation& event,
                                                        elemental_reaction reaction) noexcept
    {
        switch(reaction)
        {
        case elemental_reaction::melt:
        case elemental_reaction::vaporize:
            add_reaction_damage_bonus(event, 2);
            return;
        case elemental_reaction::none:
        case elemental_reaction::overloaded:
        case elemental_reaction::superconduct:
        case elemental_reaction::electro_charged:
        case elemental_reaction::lunar_charged:
        case elemental_reaction::frozen:
        case elemental_reaction::swirl:
        case elemental_reaction::crystallize:
        case elemental_reaction::burning:
        case elemental_reaction::bloom:
        case elemental_reaction::quicken:
            // TODO: fill in damage-stage reaction bonuses for non Melt/Vaporize reactions.
            return;
        }
    }

    constexpr std::uint32_t calculate_damage_value(std::uint32_t value, std::uint16_t multiplier_numerator,
                                                   std::uint16_t multiplier_denominator) noexcept
    {
        const auto result = static_cast<std::uint64_t>(value) * multiplier_numerator / multiplier_denominator;
        return result > std::numeric_limits<std::uint32_t>::max()
            ? std::numeric_limits<std::uint32_t>::max()
            : static_cast<std::uint32_t>(result);
    }

    inline bool all_characters_defeated(const unrestricted_table& table, player_id player)
    {
        for(auto character : table[player].characters())
        {
            if(character.state().health != 0)
            {
                return false;
            }
        }
        return true;
    }

    inline game_result result_after_health_reduction(const unrestricted_table& table)
    {
        const bool player_0_defeated = all_characters_defeated(table, player_id{ 0 });
        const bool player_1_defeated = all_characters_defeated(table, player_id{ 1 });
        if(not player_0_defeated && not player_1_defeated)
        {
            return game_result::no_result;
        }
        if(player_0_defeated)
        {
            return player_1_defeated ? game_result::both_loss : game_result::player_1_win;
        }
        return game_result::player_0_win;
    }

    namespace deal_damage_command
    {
        inline void prepare_after_damage_broadcast(
            const definition_library& library, const unrestricted_table& table, execution_context& context
        )
        {
            const auto [event] = context.stack().top<after_damage>();
            const auto next_event = event;
            context.stack().pop<after_damage>();
            prepare_broadcast(library, next_event, table, context.stack());
        }

        // ReactionOffset counts continuation opcodes from the current position. The
        // normal program omits the observation-only element-application opcode.
        template<std::size_t ReactionOffset>
        execution_state apply_damage_element(
            const definition_library& library, unrestricted_table& table, execution_context& context
        )
        {
            const auto& [event] = context.stack().top<after_damage>();
            const auto incoming_element = element_from_damage_type(event.type);
            if(incoming_element != element::none && begin_element_application(
                library, event.source, event.target, incoming_element,
                element_application_cause::damage, table, context
            ))
            {
                return context.advance(ReactionOffset * sizeof(execute_fn));
            }
            prepare_after_damage_broadcast(library, table, context);
            return context.advance((ReactionOffset + 2) * sizeof(execute_fn));
        }

        inline execution_state resume_after_health_reduction(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn&
        )
        {
            return apply_damage_element<1>(library, table, context);
        }

        inline execution_state finish_after_damage(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            if(not continue_broadcast<after_damage>(library, table, context, random))
            {
                return continue_execution;
            }
            pop_broadcast<after_damage>(context);
            const auto result = result_after_health_reduction(table);
            if(result != game_result::no_result)
            {
                return context.end_game(result);
            }
            return context.enter_next();
        }

        inline execution_state finish_after_elemental_reaction(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            if(not continue_broadcast<after_elemental_reaction>(library, table, context, random))
            {
                return continue_execution;
            }
            pop_broadcast<after_elemental_reaction>(context);
            prepare_after_damage_broadcast(library, table, context);
            return context.enter_next();
        }

        template<bool Observed>
        execution_state finish_damage_effect(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            if(not continue_broadcast<damage_effect>(library, table, context, random))
            {
                return continue_execution;
            }

            auto&& [targets, cursor, event, current_handler] = context.stack().top<
                handler_id<damage_effect>[], stack_count_t, damage_effect, handler_id<damage_effect>
            >();
            auto& target_state = table[event.target].state();
            target_state.health = event.value >= target_state.health ? 0 : target_state.health - event.value;
            const after_damage next_event{
                .source = event.source,
                .target = event.target,
                .value = event.value,
                .type = event.type,
                .flags = event.flags
            };
            pop_broadcast<damage_effect>(context);
            context.stack().push(next_event);
            if constexpr(Observed)
            {
                if(next_event.value != 0)
                {
                    return context.yield_next(execution_state::health_reduced);
                }
            }
            return apply_damage_element<Observed ? 2 : 1>(library, table, context);
        }

        inline execution_state finish_damage_calculation(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            if(not continue_broadcast<damage_calculation>(library, table, context, random))
            {
                return continue_execution;
            }

            auto&& [targets, cursor, event, current_handler] = context.stack().top<
                handler_id<damage_calculation>[], stack_count_t, damage_calculation, handler_id<damage_calculation>
            >();
            const auto incoming_element = element_from_damage_type(event.type);
            if(not event.already_handled_reaction && incoming_element != element::none)
            {
                const auto reaction = reaction_from_aura(table[event.target].state().aura, incoming_element);
                if(reaction != elemental_reaction::none)
                {
                    apply_default_damage_reaction_effect(event, reaction);
                }
            }
            const damage_effect next_event{
                .source = event.source,
                .target = event.target,
                .value = calculate_damage_value(event.value, event.multiplier_numerator, event.multiplier_denominator),
                .type = event.type,
                .flags = event.flags
            };
            pop_broadcast<damage_calculation>(context);
            prepare_broadcast(library, next_event, table, context.stack());
            return context.enter_next();
        }

        inline execution_state prepare(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn&
        )
        {
            const auto& command = context.instruction_data<1, givm::deal_damage>(library);
            prepare_broadcast(
                library,
                damage_calculation{
                    .source = command.source,
                    .target = command.target,
                    .value = command.value,
                    .multiplier_numerator = command.multiplier_numerator,
                    .multiplier_denominator = command.multiplier_denominator,
                    .type = command.type,
                    .flags = command.flags
                },
                table,
                context.stack()
            );
            return context.advance(instruction_extent<1, givm::deal_damage>);
        }
    }

    inline void compile(program_writer& writer, const givm::deal_damage& command, compile_mode mode)
    {
        using namespace deal_damage_command;
        writer.write(execute_fn{ &prepare });
        writer.write(command);
        writer.write(execute_fn{ &finish_damage_calculation });
        writer.write(mode == compile_mode::observed
            ? execute_fn{ &finish_damage_effect<true> } : execute_fn{ &finish_damage_effect<false> });
        if(mode == compile_mode::observed)
        {
            writer.write(execute_fn{ &resume_after_health_reduction });
        }
        writer.write(execute_fn{ &continue_elemental_reaction });
        writer.write(execute_fn{ &finish_after_elemental_reaction });
        writer.write(execute_fn{ &finish_after_damage });
    }
}

#endif
