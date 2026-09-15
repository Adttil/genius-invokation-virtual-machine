#ifndef GIVM_EXECUTOR_BROADCAST_HPP
#define GIVM_EXECUTOR_BROADCAST_HPP

#include <cstddef>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "../definition/events.hpp"
#include "executor.hpp"

namespace givm::detail
{
    template<class TEvent>
    using handler_id = decltype([]<class... TDefinition>(type_list<TDefinition...>)
    {
        using views = type_list_cat<views_of_definition<TDefinition>...>;
        return []<class... TView>(type_list<TView...>)
        {
            using ids = type_list_cat<
                std::conditional_t<
                    requires { subscribed_events<TView>::template index_of<TEvent>(); },
                    type_list<decltype(std::declval<TView>().id())>,
                    type_list<>
                >...
            >;
            return typename ids::template apply<std::variant>{};
        }(views{});
    }(definition_types{}));

    template<class TEvent, class TEntity>
    void append_broadcast_target(
        const definition_library& library, TEntity entity, std::vector<handler_id<TEvent>>& targets
    )
    {
        using entity_view = std::remove_cvref_t<TEntity>;
        if constexpr(requires { subscribed_events<entity_view>::template index_of<TEvent>(); })
        {
            if(not entity || not library[entity.definition_id()].template can_handle<TEvent, entity_view>())
            {
                return;
            }
            targets.push_back(entity.id());
        }
    }

    template<class TEvent>
    std::vector<handler_id<TEvent>> collect_all_broadcast_targets(
        const definition_library& library, const unrestricted_table& table
    )
    {
        std::vector<handler_id<TEvent>> targets;

        // This table order is only the default broadcast tool. Instructions may instead select
        // entities in a rule-defined order by preparing their own frame.
        for(auto player : table.players())
        {
            for(auto card : player.hand_cards())
            {
                append_broadcast_target<TEvent>(library, card, targets);
                for(auto status : card.statuses())
                {
                    append_broadcast_target<TEvent>(library, status, targets);
                }
            }

            for(auto card : player.deck_cards())
            {
                append_broadcast_target<TEvent>(library, card, targets);
                for(auto status : card.statuses())
                {
                    append_broadcast_target<TEvent>(library, status, targets);
                }
            }

            for(auto support : player.supports())
            {
                append_broadcast_target<TEvent>(library, support, targets);
            }
            for(auto summon : player.summons())
            {
                append_broadcast_target<TEvent>(library, summon, targets);
            }
            for(auto combat_status : player.combat_statuses())
            {
                append_broadcast_target<TEvent>(library, combat_status, targets);
            }
            for(auto character : player.characters())
            {
                append_broadcast_target<TEvent>(library, character, targets);
                for(auto skill : character.skills())
                {
                    append_broadcast_target<TEvent>(library, skill, targets);
                }
                for(auto attachment : character.attachments())
                {
                    append_broadcast_target<TEvent>(library, attachment, targets);
                }
            }
        }

        return targets;
    }

    template<class TEvent>
    void prepare_broadcast(
        const definition_library& library, const TEvent& event, const unrestricted_table& table, frame_stack& stack
    )
    {
        auto targets = collect_all_broadcast_targets<TEvent>(library, table);
        stack.push(
            dynamic_array<handler_id<TEvent>>(targets),
            stack_count_t{},
            event,
            handler_id<TEvent>{}
        );
    }

    template<class TEntityView, class TEvent>
    handler_program_entry_t<TEvent> try_handle(
        const definition_library& library,
        TEntityView entity,
        TEvent& event,
        const unrestricted_table& table,
        random_fn& random
    )
    {
        if(not entity)
        {
            return handler_program_entry_t<TEvent>::null();
        }
        return library[entity.definition_id()].template handle<TEvent>(
            entity, event, table, random
        );
    }

    template<class TEvent>
    bool continue_broadcast(
        const definition_library& library,
        const unrestricted_table& table,
        execution_context& context,
        random_fn& random
    )
    {
        auto&& [targets, cursor, event, current_handler] =
            context.stack().top<
                handler_id<TEvent>[],
                stack_count_t,
                TEvent,
                handler_id<TEvent>
            >();
        const auto target_count = static_cast<stack_count_t>(targets.size());
        while(cursor < target_count)
        {
            current_handler = targets[static_cast<size_t>(cursor++)];
            const auto entry = std::visit([&](auto id)
            {
                return try_handle(
                    library,
                    std::as_const(table)[id],
                    event,
                    table,
                    random
                );
            }, current_handler);
            if(entry)
            {
                context.enter(entry);
                return false;
            }
        }
        return true;
    }

    template<class TEvent>
    void pop_broadcast(execution_context& context)
    {
        context.stack().pop<
            handler_id<TEvent>[],
            stack_count_t,
            TEvent,
            handler_id<TEvent>
        >();
    }

}

#endif
