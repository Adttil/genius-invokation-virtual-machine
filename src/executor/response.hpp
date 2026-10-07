#ifndef GIVM_PRIVATE_EXECUTOR_RESPONSE_HPP
#define GIVM_PRIVATE_EXECUTOR_RESPONSE_HPP

#include <cstddef>
#include <array>
#include <algorithm>
#include <span>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <givm/definition.hpp>
#include <givm/executor/executor.hpp>
#include <givm/executor/views/action_selection.hpp>
#include "event_queues.hpp"

namespace givm::detail
{
    template<class TEvent>
    inline constexpr std::size_t response_instruction_count = 2;

    template<class TEvent>
    inline constexpr std::size_t response_extent = response_instruction_count<TEvent> * sizeof(execute_fn);

    template<class TEvent, class TEntity>
    void append_broadcast_target(
        const definition_library& library, TEntity entity, std::vector<handler_id<TEvent>>& targets
    )
    {
        using entity_view = std::remove_cvref_t<TEntity>;
        if constexpr(requires { subscribed_events<std::remove_cvref_t<entity_view>::category>::template index_of<TEvent>(); })
        {
            if(not entity || not library[entity.definition_id()].template can_handle<TEvent, std::remove_cvref_t<entity_view>::category>())
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
            if(not character || not character.state().alive) return;
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
                    equipment[equipment_count++] = attachment.id().index();
                    append_broadcast_target<TEvent>(library, attachment, targets);
                }
            }
            for(auto attachment : character.attachments())
            {
                const auto end = equipment.begin() + equipment_count;
                if(std::find(equipment.begin(), end, attachment.id().index()) == end)
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
            if(active_character) append_character(characters[active_character.get().index()]);
            for(auto combat_status : player.combat_statuses())
            {
                append_broadcast_target<TEvent>(library, combat_status, targets);
            }
            if(active_character)
            {
                auto index = active_character.get().index();
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

    struct broadcast_progress
    {
        stack_count_t cursor = 0;
        std::uint32_t response_index = 0;
    };

    template<class TEvent>
    void prepare_broadcast(
        std::span<const handler_id<TEvent>> targets, const TEvent& event, const unrestricted_table& table,
        frame_stack& stack, execution_position return_position
    )
    {
        const auto saved_event = event;
        stack.push(dynamic_array<handler_id<TEvent>>(targets), broadcast_progress{}, saved_event,
            response_return{ table.state().self_player, return_position });
    }

    template<class TEvent>
    void prepare_broadcast(
        const definition_library& library, const TEvent& event, const unrestricted_table& table,
        frame_stack& stack, execution_position return_position
    )
    {
        const auto targets = collect_all_broadcast_targets<TEvent>(library, table);
        prepare_broadcast<TEvent>(targets, event, table, stack, return_position);
    }

    template<class TEvent, bool Queued = false>
    bool continue_broadcast(const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random, execute_fn completion = nullptr)
    {
        auto frame = context.stack().top<handler_id<TEvent>[], broadcast_progress, TEvent, response_return>();
        while(get<1>(frame).cursor < get<0>(frame).size())
        {
            const auto handler = get<0>(frame)[get<1>(frame).cursor];
            const auto index = get<1>(frame).response_index;
            const auto previous_player = get<3>(frame).previous_player;
            const auto return_position = get<3>(frame).position + sizeof(execute_fn);
            if constexpr(Queued) context.stack().push(queued_response_completion{ completion });
            context.stack().push(response_return{ previous_player, return_position });
            const auto [caller, call] = context.stack().top<
                givm::frame<handler_id<TEvent>[], broadcast_progress, TEvent, response_return>, [] { if constexpr(Queued) return givm::frame<queued_response_completion, response_return>;
                    else return givm::frame<response_return>; }()>();
            auto& event = get<2>(caller);
            player_id player{};
            const auto entry = handler.visit([&](auto id) -> effect<TEvent::category>
            {
                const auto entity = std::as_const(table)[id];
                if(not entity) return {};
                if constexpr(requires { entity.character(); })
                    if(not entity.character().state().alive) return {};
                player = entity.player().id();
                auto response = context.make_handle_context<TEvent::category>(library, entity, random);
                return library[entity.definition_id()].template handle<TEvent>(event, response, index);
            });
            if(entry)
            {
                table.state().self_player = player;
                context.enter(entry);
                return false;
            }
            if constexpr(Queued) context.stack().pop<queued_response_completion, response_return>();
            else context.stack().pop<response_return>();
            frame = context.stack().top<handler_id<TEvent>[], broadcast_progress, TEvent, response_return>();
            ++get<1>(frame).cursor;
            get<1>(frame).response_index = 0;
        }
        return true;
    }

    template<class TEvent>
    execution_state complete_broadcast_response(const definition_library&, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        const auto result = get<0>(context.stack().top<response_return>()).result;
        context.stack().pop<response_return>();
        const auto frame = context.stack().top<handler_id<TEvent>[], broadcast_progress, TEvent, response_return>();
        auto& progress = get<1>(frame);
        if(result == return_response::null) { ++progress.cursor; progress.response_index = 0; }
        else progress.response_index = result;
        return context.jump(get<3>(frame).position);
    }

    template<class TEvent>
    void pop_broadcast(execution_context& context)
    {
        context.stack().pop<handler_id<TEvent>[], broadcast_progress, TEvent, response_return>();
    }

    template<class TId>
    struct single_response_progress
    {
        TId handler;
        std::uint32_t response_index = 0;
        bool allow_removed = false;
        bool enabled = true;
    };

    template<class TEvent, class TId, bool Queued = false>
    bool continue_single_response(const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random, execute_fn completion = nullptr)
    {
        const auto frame = context.stack().top<single_response_progress<TId>, TEvent, response_return>();
        const auto progress = get<0>(frame);
        if(not progress.enabled || progress.response_index == return_response::null) return true;
        const auto entity = std::as_const(table)[progress.handler];
        if(not progress.allow_removed && not entity) return true;
        if constexpr(requires { entity.character(); })
            if(not progress.allow_removed && not entity.character().state().alive) return true;
        const auto definition = library[entity.definition_id()];
        if(not definition.template can_handle<TEvent, std::remove_cvref_t<decltype(entity)>::category>()) return true;
        const auto player = entity.player().id();
        const auto previous_player = get<2>(frame).previous_player;
        const auto return_position = get<2>(frame).position + sizeof(execute_fn);
        if constexpr(Queued) context.stack().push(queued_response_completion{ completion });
        context.stack().push(response_return{ previous_player, return_position });
        const auto [caller, call] = context.stack().top<
            givm::frame<single_response_progress<TId>, TEvent, response_return>, [] { if constexpr(Queued) return givm::frame<queued_response_completion, response_return>;
                else return givm::frame<response_return>; }()>();
        auto& event = get<1>(caller);
        auto response = context.make_handle_context<TEvent::category>(library, entity, random);
        const auto entry = definition.template handle<TEvent>(event, response, progress.response_index);
        if(not entry)
        {
            if constexpr(Queued) context.stack().pop<queued_response_completion, response_return>();
            else context.stack().pop<response_return>();
            return true;
        }
        table.state().self_player = player;
        context.enter(entry);
        return false;
    }

    template<class TEvent, class TId>
    execution_state complete_single_response(const definition_library&, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        const auto result = get<0>(context.stack().top<response_return>()).result;
        context.stack().pop<response_return>();
        const auto frame = context.stack().top<single_response_progress<TId>, TEvent, response_return>();
        get<0>(frame).response_index = result;
        return context.jump(get<2>(frame).position);
    }

    template<class TEvent, class TId>
    void prepare_single_response(const TEvent& event, TId handler, const unrestricted_table& table,
        execution_context& context, execution_position resume, bool allow_removed = false, bool enabled = true)
    {
        const auto saved_event = event;
        context.stack().push(single_response_progress<TId>{ handler, 0, allow_removed, enabled }, saved_event,
            response_return{ table.state().self_player, resume });
    }

    template<class TEvent, class TId>
    void pop_single_response(execution_context& context)
    {
        context.stack().pop<single_response_progress<TId>, TEvent, response_return>();
    }

}

#endif
