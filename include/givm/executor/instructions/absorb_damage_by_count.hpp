#ifndef GIVM_EXECUTOR_INSTRUCTIONS_ABSORB_DAMAGE_BY_COUNT_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_ABSORB_DAMAGE_BY_COUNT_HPP

#include <algorithm>
#include <cstdint>
#include <limits>
#include <variant>

#include "../broadcast.hpp"
#include "../events.hpp"
#include "../executor.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    struct absorb_damage_by_count
    {
        using context_type = damage_effect;

        std::uint32_t maximum_count = std::numeric_limits<std::uint32_t>::max();

        bool execute(card_table& table, execution_context& context, random_fn&) const
        {
            auto&& [broadcast, activation] = context.stack().top<
                frame<
                    detail::handler_id<damage_effect>[],
                    stack_count_t,
                    damage_effect,
                    detail::handler_id<damage_effect>,
                    stage_t
                >,
                frame<execution_context::return_info, stage_t>
            >();
            auto&& [handlers, cursor, event, current_handler, broadcast_stage] = broadcast;
            auto&& [return_info, activation_stage] = activation;
            (void)handlers;
            (void)cursor;
            (void)broadcast_stage;
            (void)return_info;
            GIVM_ASSERT(activation_stage == stage_t{});

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
                    const std::uint32_t absorbed = std::min({ event.value, count, maximum_count });
                    event.value -= absorbed;
                    count -= absorbed;
                }
                else
                {
                    GIVM_ASSERT(false);
                }
            }, current_handler);

            return context.enter_next();
        }
    };

}

#include "../../macro_undef.hpp"

#endif
