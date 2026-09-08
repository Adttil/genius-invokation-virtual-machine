#ifndef GIVM_EXECUTOR_INSTRUCTIONS_DEAL_DAMAGE_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_DEAL_DAMAGE_HPP

#include "../executor.hpp"

#include <cstdint>
#include <limits>

#include "../broadcast.hpp"
#include "../events.hpp"
#include "apply_element.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    namespace detail
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

        inline bool all_characters_defeated(const card_table& table, player_id player)
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

        inline game_result result_after_health_reduction(const card_table& table)
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
    }

    struct deal_damage
    {
        using context_type = void;

        damage_source_id source;
        character_id target;
        std::uint32_t value;
        std::uint16_t multiplier_numerator = 1;
        std::uint16_t multiplier_denominator = 1;
        damage_type type;
        damage_flags flags;
        enum class stage_type : stage_t
        {
            prepare_damage_calculation,
            damage_calculation_broadcast,
            damage_effect_broadcast,
            prepare_after_damage,
            damage_element_reaction_broadcast,
            damage_after_elemental_reaction_broadcast,
            after_damage_broadcast
        };
        bool execute(card_table& table, execution_context& context, random_fn& random) const
        {
            const auto stage = static_cast<stage_type>(context.current_stage());
            if(stage == stage_type::prepare_damage_calculation)
            {
                detail::prepare_broadcast(
                    damage_calculation{ .source = source,
                                        .target = target,
                                        .value = value,
                                        .multiplier_numerator = multiplier_numerator,
                                        .multiplier_denominator = multiplier_denominator,
                                        .type = type,
                                        .flags = flags },
                    table,
                    context.stack()
                );
                context.current_stage() = static_cast<stage_t>(stage_type::damage_calculation_broadcast);
                return true;
            }

            if(stage == stage_type::damage_calculation_broadcast)
            {
                if(not detail::continue_broadcast<damage_calculation>(table, context, random))
                {
                    return true;
                }

                auto&& [targets, cursor, event, current_handler, stored_stage] = context.stack().top<
                    detail::handler_id<damage_calculation>[],
                    stack_count_t,
                    damage_calculation,
                    detail::handler_id<damage_calculation>,
                    stage_t
                >();
                (void)targets;
                (void)cursor;
                (void)current_handler;
                (void)stored_stage;

                const auto incoming_element = element_from_damage_type(event.type);
                if(not event.already_handled_reaction && incoming_element != element::none)
                {
                    const auto reaction = reaction_from_aura(table[event.target].state().aura, incoming_element);
                    if(reaction != elemental_reaction::none)
                    {
                        detail::apply_default_damage_reaction_effect(event, reaction);
                    }
                }

                const damage_effect next_event{
                    .source = event.source,
                    .target = event.target,
                    .value = detail::calculate_damage_value(event.value, event.multiplier_numerator,
                                                            event.multiplier_denominator),
                    .type = event.type,
                    .flags = event.flags
                };
                detail::pop_broadcast<damage_calculation>(context);

                detail::prepare_broadcast(next_event, table, context.stack());
                context.current_stage() = static_cast<stage_t>(stage_type::damage_effect_broadcast);
                return true;
            }

            if(stage == stage_type::damage_effect_broadcast)
            {
                if(not detail::continue_broadcast<damage_effect>(table, context, random))
                {
                    return true;
                }

                auto&& [targets, cursor, event, current_handler, stored_stage] = context.stack().top<
                    detail::handler_id<damage_effect>[],
                    stack_count_t,
                    damage_effect,
                    detail::handler_id<damage_effect>,
                    stage_t
                >();
                (void)targets;
                (void)cursor;
                (void)current_handler;
                (void)stored_stage;
                auto& target_state = table[event.target].state();
                target_state.health = event.value >= target_state.health ? 0 : target_state.health - event.value;

                const auto incoming_element = element_from_damage_type(event.type);
                const after_damage next_event{
                    .source = event.source,
                    .target = event.target,
                    .value = event.value,
                    .type = event.type,
                    .flags = event.flags
                };
                detail::pop_broadcast<damage_effect>(context);

                context.stack().push(
                    next_event,
                    static_cast<stage_t>(stage_type::prepare_after_damage)
                );
                if(
                    incoming_element != element::none
                    && detail::begin_element_application(
                        next_event.source,
                        next_event.target,
                        incoming_element,
                        element_application_cause::damage,
                        table,
                        context
                    )
                )
                {
                    context.current_stage() = static_cast<stage_t>(
                        stage_type::damage_element_reaction_broadcast
                    );
                    return true;
                }
                return prepare_after_damage_broadcast(table, context);
            }

            if(stage == stage_type::damage_element_reaction_broadcast)
            {
                if(not detail::continue_broadcast<elemental_reaction_will_occur>(table, context, random))
                {
                    return true;
                }

                detail::prepare_broadcast(
                    detail::finish_elemental_reaction(table, context),
                    table,
                    context.stack()
                );
                context.current_stage() = static_cast<stage_t>(
                    stage_type::damage_after_elemental_reaction_broadcast
                );
                return true;
            }

            if(stage == stage_type::damage_after_elemental_reaction_broadcast)
            {
                if(not detail::continue_broadcast<after_elemental_reaction>(table, context, random))
                {
                    return true;
                }

                detail::pop_broadcast<after_elemental_reaction>(context);
                return prepare_after_damage_broadcast(table, context);
            }

            GIVM_ASSERT(stage == stage_type::after_damage_broadcast);
            if(not detail::continue_broadcast<after_damage>(table, context, random))
            {
                return true;
            }

            detail::pop_broadcast<after_damage>(context);
            const auto result = detail::result_after_health_reduction(table);
            if(result != game_result::no_result)
            {
                return context.end_game(result);
            }
            return context.enter_next();
        }

    private:
        static bool prepare_after_damage_broadcast(
            const card_table& table,
            execution_context& context
        )
        {
            auto&& [event, stored_stage] = context.stack().top<after_damage, stage_t>();
            (void)stored_stage;
            const auto next_event = event;
            context.stack().pop<after_damage, stage_t>();
            detail::prepare_broadcast(next_event, table, context.stack());
            context.current_stage() = static_cast<stage_t>(stage_type::after_damage_broadcast);
            return true;
        }
    };

}

#include "../../macro_undef.hpp"

#endif
