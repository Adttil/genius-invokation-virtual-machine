#ifndef GIVM_EXECUTOR_BROADCAST_HPP
#define GIVM_EXECUTOR_BROADCAST_HPP

#include <cstddef>
#include <array>
#include <algorithm>
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

        const auto append_character = [&](auto character)
        {
            if(not character) return;
            for(auto skill : character.skills())
            {
                append_broadcast_target<TEvent>(library, skill, targets);
            }
            std::array<std::size_t, static_cast<std::size_t>(equipment_type::none)> equipment{};
            std::size_t equipment_count = 0;
            for(std::size_t index = 0; index != equipment.size(); ++index)
            {
                const auto type = static_cast<equipment_type>(index);
                if(character.has(type))
                {
                    const auto attachment = character.get(type);
                    equipment[equipment_count++] = attachment.id().index;
                    append_broadcast_target<TEvent>(library, attachment, targets);
                }
            }
            for(auto attachment : character.attachments())
            {
                const auto end = equipment.begin() + equipment_count;
                if(std::find(equipment.begin(), end, attachment.id().index) == end)
                    append_broadcast_target<TEvent>(library, attachment, targets);
            }
        };

        // Each broadcast snapshots the acting player and relative character order here.
        const auto active_player = table.state().active_player;
        for(const auto player_id : { active_player, other_player(active_player) })
        {
            const auto player = table[player_id];
            const auto characters = player.template characters<false>();
            const auto active_character = player.state().active_character;
            if(active_character) append_character(characters[active_character->index]);
            for(auto combat_status : player.combat_statuses())
            {
                append_broadcast_target<TEvent>(library, combat_status, targets);
            }
            if(active_character)
            {
                auto index = active_character->index;
                for(std::size_t offset = 1; offset < characters.size(); ++offset)
                {
                    if(++index == characters.size()) index = 0;
                    append_character(characters[index]);
                }
            }
            else
            {
                for(auto character : characters) append_character(character);
            }
            for(auto summon : player.summons())
            {
                append_broadcast_target<TEvent>(library, summon, targets);
            }
            for(auto support : player.supports())
            {
                append_broadcast_target<TEvent>(library, support, targets);
            }
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
        }

        return targets;
    }

    template<class TEvent>
    void prepare_broadcast(
        const definition_library& library, const TEvent& event, const unrestricted_table& table,
        frame_stack& stack, execution_position return_position
    )
    {
        auto targets = collect_all_broadcast_targets<TEvent>(library, table);
        stack.push(
            dynamic_array<handler_id<TEvent>>(targets),
            stack_count_t{},
            event,
            return_position
        );
    }

    template<class TEntityView, class TEvent>
    program_entry try_handle(
        const definition_library& library,
        TEntityView entity,
        TEvent& event,
        handle_context& response
    )
    {
        if(not entity)
        {
            return {};
        }
        return library[entity.definition_id()].template handle<TEvent>(
            entity, event, response
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
        auto&& [targets, cursor, event, return_position] =
            context.stack().top<
                handler_id<TEvent>[],
                stack_count_t,
                TEvent,
                execution_position
            >();
        const auto target_count = static_cast<stack_count_t>(targets.size());
        while(cursor < target_count)
        {
            const auto current_handler = targets[static_cast<size_t>(cursor++)];
            auto response = context.make_handle_context(table, random);
            const auto entry = std::visit([&](auto id)
            {
                return try_handle(
                    library,
                    std::as_const(table)[id],
                    event,
                    response
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
            execution_position
        >();
    }

}

#endif
