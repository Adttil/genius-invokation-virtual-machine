#ifndef GIVM_EXECUTOR_INSTRUCTIONS_ABSORB_DAMAGE_BY_COUNT_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_ABSORB_DAMAGE_BY_COUNT_HPP

#include <algorithm>
#include <cstdint>
#include <limits>
#include <variant>

#include "../broadcast.hpp"
#include "../../definition/events.hpp"
#include "../executor.hpp"
#include "../instruction.hpp"
#include "../../definition/commands.hpp"

#include "../../macro_define.hpp"

namespace givm::detail
{
    namespace absorb_damage_by_count_command
    {
        inline execution_state execute(
            const definition_library& library,
            unrestricted_table& table, execution_context& context, random_fn&
        )
        {
            const auto& command = context.instruction_data<1, givm::absorb_damage_by_count>(library);
            auto&& [broadcast, activation] = context.stack().top<
                frame<
                    handler_id<damage_effect>[],
                    stack_count_t,
                    damage_effect,
                    handler_id<damage_effect>
                >,
                frame<execution_context::return_info>
            >();
            auto&& [handlers, cursor, event, current_handler] = broadcast;
            (void)handlers;
            (void)cursor;
            (void)activation;

            std::visit([&](auto id)
            {
                auto entity = table[id];
                if(not entity)
                {
                    GIVM_ASSERT(false);
                    return;
                }

                if constexpr(requires { entity.state().count; })
                {
                    auto& count = entity.state().count;
                    const std::uint32_t absorbed = std::min({ event.value, count, command.maximum_count });
                    event.value -= absorbed;
                    count -= absorbed;
                }
                else
                {
                    GIVM_ASSERT(false);
                }
            }, current_handler);
            return context.advance(instruction_extent<1, givm::absorb_damage_by_count>);
        }
    }

    inline void compile(program_writer& writer, const givm::absorb_damage_by_count& command, compile_mode)
    {
        writer.write(execute_fn{ &absorb_damage_by_count_command::execute });
        writer.write(command);
    }
}

#include "../../macro_undef.hpp"

#endif
