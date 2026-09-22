#ifndef GIVM_EXECUTOR_COMMANDS_APPLY_ELEMENT_HPP
#define GIVM_EXECUTOR_COMMANDS_APPLY_ELEMENT_HPP

#include "deal_damage.hpp"

namespace givm::detail
{
    template<bool Observed>
    execution_state prepare_element_application(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto command = context.instruction_data<1, givm::apply_element>(library);
        const auto position = context.position() + instruction_extent<1, givm::apply_element>;
        const auto target = table[command.target];
        if(not target) return context.jump(position + damage_end_offset<Observed> * sizeof(execute_fn));
        context.stack().push(damage_group{ .instructions = position }, substack());
        prepend_damage(context.stack(), damage_node{
            .event = { .source = command.source, .target = command.target,
                .value = 0, .type = damage_type_from_element(command.element),
                .reaction = reaction_from_aura(target.state().aura, command.element),
                .reacted_aura = target.state().aura },
            .cause = command.cause, .deals_damage = false
        });
        if(const auto state = apply_group_damage_element<Observed>(library, table, context, random)) return *state;
        return continue_damage_group<false, Observed>(library, table, context, random);
    }

    template<bool Observed>
    void compile_element_application(program_writer& writer, const givm::apply_element& command)
    {
        writer.write(execute_fn{ prepare_element_application<Observed> });
        writer.write(command);
        compile_damage_resolution<false, Observed>(writer);
    }

    inline void compile(program_writer& writer, const givm::apply_element& command, compile_mode mode)
    {
        if(mode == compile_mode::observed) compile_element_application<true>(writer, command);
        else compile_element_application<false>(writer, command);
    }
}

#endif
