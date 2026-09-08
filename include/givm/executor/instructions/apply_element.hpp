#ifndef GIVM_EXECUTOR_INSTRUCTIONS_APPLY_ELEMENT_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_APPLY_ELEMENT_HPP

#include "../executor.hpp"

#include <cstdint>

#include "../broadcast.hpp"
#include "../events.hpp"

namespace givm
{
    namespace detail
    {
        inline bool begin_element_application(
            element_application_source_id source,
            character_id target,
            element incoming_element,
            element_application_cause cause,
            card_table& table,
            execution_context& context
        )
        {
            auto target_entity = table[target];
            auto& target_state = target_entity.state();
            const auto reacted_aura = target_state.aura;
            const auto reaction = reaction_from_aura(reacted_aura, incoming_element);

            if(reaction == elemental_reaction::none)
            {
                target_state.aura = aura_without_reaction(reacted_aura, incoming_element);
                return false;
            }

            prepare_broadcast(
                elemental_reaction_will_occur{
                    .source = source,
                    .target = target,
                    .incoming_element = incoming_element,
                    .reacted_aura = reacted_aura,
                    .reaction = reaction,
                    .cause = cause,
                    .already_handled = false
                },
                table,
                context.stack()
            );
            return true;
        }

        inline after_elemental_reaction finish_elemental_reaction(
            card_table& table,
            execution_context& context
        )
        {
            auto&& [targets, cursor, event, current_handler, stored_stage] =
                context.stack().top<
                    handler_id<elemental_reaction_will_occur>[],
                    stack_count_t,
                    elemental_reaction_will_occur,
                    handler_id<elemental_reaction_will_occur>,
                    stage_t
                >();
            (void)targets;
            (void)cursor;
            (void)current_handler;
            (void)stored_stage;

            if(not event.already_handled)
            {
                table[event.target].state().aura = aura_after_reaction(
                    event.reacted_aura,
                    event.incoming_element,
                    event.reaction
                );
            }

            const after_elemental_reaction result{
                .source = event.source,
                .target = event.target,
                .incoming_element = event.incoming_element,
                .reacted_aura = event.reacted_aura,
                .reaction = event.reaction,
                .cause = event.cause
            };
            pop_broadcast<elemental_reaction_will_occur>(context);
            // TODO: apply the remaining default reaction effects here.
            return result;
        }
    }

    struct apply_element
    {
        using context_type = void;

        element_application_source_id source;
        character_id target;
        element element;
        element_application_cause cause = element_application_cause::effect;
        enum class stage_type : stage_t
        {
            apply,
            reaction_broadcast,
            after_reaction_broadcast
        };
        bool execute(card_table& table, execution_context& context, random_fn& random) const
        {
            const auto stage = static_cast<stage_type>(context.current_stage());
            if(stage == stage_type::apply)
            {
                if(not detail::begin_element_application(
                    source,
                    target,
                    element,
                    cause,
                    table,
                    context
                ))
                {
                    return context.enter_next();
                }
                context.current_stage() = static_cast<stage_t>(stage_type::reaction_broadcast);
                return true;
            }

            if(stage == stage_type::reaction_broadcast)
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
                context.current_stage() = static_cast<stage_t>(stage_type::after_reaction_broadcast);
                return true;
            }

            if(not detail::continue_broadcast<after_elemental_reaction>(table, context, random))
            {
                return true;
            }

            detail::pop_broadcast<after_elemental_reaction>(context);
            return context.enter_next();
        }
    };

}

#endif
