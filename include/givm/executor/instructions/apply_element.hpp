#ifndef GIVM_EXECUTOR_INSTRUCTIONS_APPLY_ELEMENT_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_APPLY_ELEMENT_HPP

#include "../executor.hpp"

#include <cstdint>

#include "../broadcast.hpp"
#include "../../definition/events.hpp"
#include "../instruction.hpp"
#include "../../definition/commands.hpp"

namespace givm
{
    namespace detail
    {
        inline bool begin_element_application(
            const definition_library& library,
            element_application_source_id source,
            character_id target,
            element incoming_element,
            element_application_cause cause,
            unrestricted_table& table,
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
                library,
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
            unrestricted_table& table,
            execution_context& context
        )
        {
            auto&& [targets, cursor, event, current_handler] =
                context.stack().top<
                    handler_id<elemental_reaction_will_occur>[],
                    stack_count_t,
                    elemental_reaction_will_occur,
                    handler_id<elemental_reaction_will_occur>
                >();
            (void)targets;
            (void)cursor;
            (void)current_handler;

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

        // These two continuations only use the broadcast frame, so damage and
        // direct element application can share the same reaction instruction.
        inline execution_state continue_elemental_reaction(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            if(not continue_broadcast<elemental_reaction_will_occur>(library, table, context, random))
            {
                return continue_execution;
            }
            prepare_broadcast(library, finish_elemental_reaction(table, context), table, context.stack());
            return context.enter_next();
        }

        inline execution_state finish_element_application(
            const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random
        )
        {
            if(not continue_broadcast<after_elemental_reaction>(library, table, context, random))
            {
                return continue_execution;
            }
            pop_broadcast<after_elemental_reaction>(context);
            return context.enter_next();
        }

        namespace apply_element_command
        {
            inline execution_state execute(
                const definition_library& library, unrestricted_table& table,
                execution_context& context, random_fn&
            )
            {
                const auto& command = context.instruction_data<1, givm::apply_element>(library);
                const bool reacting = begin_element_application(
                    library, command.source, command.target, command.element, command.cause, table, context
                );
                return context.advance(instruction_extent<1, givm::apply_element>
                    + (reacting ? 0 : 2 * sizeof(execute_fn)));
            }
        }

        inline void compile(program_writer& writer, const givm::apply_element& command, compile_mode)
        {
            writer.write(execute_fn{ &apply_element_command::execute });
            writer.write(command);
            writer.write(execute_fn{ &continue_elemental_reaction });
            writer.write(execute_fn{ &finish_element_application });
        }
    }
}

#endif
