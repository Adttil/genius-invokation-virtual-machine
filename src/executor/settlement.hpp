#ifndef GIVM_PRIVATE_EXECUTOR_SETTLEMENT_HPP
#define GIVM_PRIVATE_EXECUTOR_SETTLEMENT_HPP

#include "response.hpp"

#include <givm/macro_define.hpp>

namespace givm::detail
{
    inline execution_state complete_queued_response(const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        const auto completion = get<0>(context.stack().top<queued_response_completion, response_return>()).complete;
        return completion(library, table, context, random);
    }

    inline execution_state resume_notification(const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        const auto continuation = get<0>(context.stack().top<notification_continuation>());
        context.stack().pop<notification_continuation>();
        context.jump(continuation.resume);
        return continuation.next(library, table, context, random);
    }

    template<class TEvent>
    struct queued_broadcast
    {
        static execution_state resume(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            if(not continue_broadcast<TEvent, true>(library, table, context, random, complete))
                return continue_execution;
            pop_broadcast<TEvent>(context);
            if constexpr(TEvent::category == event_category::immediate)
                return resume_notification(library, table, context, random);
            else return context.jump(get<0>(context.stack().top<notification_continuation>()).resume);
        }

        static execution_state complete(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            const auto result = get<1>(context.stack().top<queued_response_completion, response_return>()).result;
            context.stack().pop<queued_response_completion, response_return>();
            const auto frame = context.stack().top<handler_id<TEvent>[], broadcast_progress, TEvent, response_return>();
            auto& progress = get<1>(frame);
            if(result == return_response::null) { ++progress.cursor; progress.response_index = 0; }
            else progress.response_index = result;
            return resume(library, table, context, random);
        }

        static execution_state start(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random, const TEvent& event,
            std::span<const handler_id<TEvent>> targets, execute_fn next, execution_position response_position)
        {
            library.record_history(event, table);
            context.stack().push(notification_continuation{ next, context.position() });
            prepare_broadcast<TEvent>(targets, event, table, context.stack(), response_position - sizeof(execute_fn));
            return resume(library, table, context, random);
        }
    };

    template<class TEvent, class TId, bool AllowRemoved = true>
    struct queued_single_response
    {
        static execution_state resume(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            if(not continue_single_response<TEvent, TId, true>(library, table, context, random, complete))
                return continue_execution;
            pop_single_response<TEvent, TId>(context);
            return context.jump(get<0>(context.stack().top<notification_continuation>()).resume);
        }

        static execution_state complete(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            const auto result = get<1>(context.stack().top<queued_response_completion, response_return>()).result;
            context.stack().pop<queued_response_completion, response_return>();
            get<0>(context.stack().top<single_response_progress<TId>, TEvent, response_return>()).response_index = result;
            return resume(library, table, context, random);
        }

        static execution_state start(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random, const TEvent& event, TId handler,
            execute_fn next, execution_position response_position)
        {
            context.stack().push(notification_continuation{ next, context.position() });
            prepare_single_response(event, handler, table, context,
                response_position - sizeof(execute_fn), AllowRemoved);
            return resume(library, table, context, random);
        }
    };

    template<class TSelfEvent, class TEvent, class TId>
    struct queued_removal
    {
        static execution_state begin_notification(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            const auto event = get<2>(context.stack().top<handler_id<TEvent>[], broadcast_progress, TEvent, response_return>());
            library.record_history(event, table);
            return queued_broadcast<TEvent>::resume(library, table, context, random);
        }

        static execution_state start(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random, TId entity, const TEvent& event,
            execute_fn next, execution_position response_position)
        {
            const auto targets = collect_all_broadcast_targets<TEvent>(library, table);
            context.stack().push(notification_continuation{ next, context.position() });
            prepare_broadcast<TEvent>(targets, event, table, context.stack(), response_position - sizeof(execute_fn));
            return queued_single_response<TSelfEvent, TId>::start(library, table, context, random,
                TSelfEvent{}, entity, begin_notification, response_position);
        }
    };

    template<class T>
    T& queue_record(frame_stack& queue, std::size_t offset) noexcept
    {
        return *std::launder(reinterpret_cast<T*>(queue.data() + offset));
    }

    template<class T>
    void append_queue_record(frame_stack& queue, const T& record)
    {
        const auto saved = record;
        const auto offset = queue.size() - substack_tail_size;
        const auto required = queue.size() + record_extent(sizeof(T));
        if(required > queue.capacity()) queue.reserve(std::bit_ceil(required));
        auto destination = get<0>(queue.top<substack_t>());
        for(std::size_t index = 0; index < record_extent(sizeof(T)); index += max_alignment)
            destination.push<unsigned char[max_alignment]>();
        std::memcpy(queue.data() + offset, &saved, sizeof(T));
    }

    inline void record_damage(execution_context& context, const damage_effect& result, bool pending_dying)
    {
        const auto reaction = result.reaction ? result.reaction.get<entity_category::reaction>().slot() : elemental_reaction::none;
        const auto begin = current_segment(context).damage;
        auto& queue = context.damage_events();
        const auto end = queue.size() - substack_tail_size;
        for(auto offset = begin; offset != end; offset += record_extent(sizeof(damage_notification_record)))
        {
            auto& record = queue_record<damage_notification_record>(queue, offset);
            if(record.target != result.target) continue;
            const auto maximum = std::numeric_limits<std::uint32_t>::max();
            record.value = maximum - record.value < result.value ? maximum : record.value + result.value;
            record.type |= result.type;
            record.flags |= result.flags;
            record.reaction |= reaction;
            record.pending_dying |= pending_dying;
            return;
        }
        append_queue_record(queue, damage_notification_record{
            result.target, result.value, result.type, result.flags, reaction, false, pending_dying });
    }

    inline void record_hand_entry(execution_context& context, hand_card_id card, hand_entry_kind kind, bool overflow)
    {
        append_queue_record(context.hand_entry_events(), hand_entry_record{ card, kind, overflow });
    }

    struct segment_seal_progress
    {
        std::size_t cursor;
        std::size_t current;
        execution_position resume;
        execution_position response_completion;
    };

    struct segment_dispatch_progress
    {
        std::size_t cursor;
        std::size_t hand_end;
        std::size_t damage_begin;
        std::size_t damage_end;
    };

    struct defeated_attachment_progress
    {
        character_id target;
        std::size_t cursor = 0;
    };

    template<class TEvent>
    struct event_record
    {
        mixed_record_header header;
        TEvent event;
    };

    template<class TSelfEvent, class TEvent, class TId>
    struct removal_record
    {
        mixed_record_header header;
        TId entity;
        TEvent event;
    };

    template<class TEvent, class TId>
    struct single_event_record
    {
        mixed_record_header header;
        TId entity;
        TEvent event;
    };

    struct active_selection_progress
    {
        execution_position resume;
        execution_position loop;
        std::array<optional_entity_id<entity_category::character>, 2> selected{};
        player_id first_player;
        std::size_t cursor = 0;
    };

    struct settlement_driver
    {
        static execution_state continue_records(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            for(;;)
            {
                auto& progress = get<0>(context.stack().top<settlement_progress>());
                const auto end = mixed_end(context);
                if(progress.cursor == end)
                {
                    const auto resume = progress.resume;
                    const auto cause_defeated = progress.cause_defeated;
                    clear_current_domain(context);
                    context.stack().pop<settlement_progress>();
                    for(std::uint8_t index = 0; index != 2; ++index)
                    {
                        if(not cause_defeated) break;
                        const player_id player{ index };
                        const auto active = table[player].state().active_character;
                        if(not active || table[*active].state().alive) continue;
                        context.stack().push(active_selection_progress{ .resume = resume, .loop = context.position(),
                            .first_player = table.state().active_player });
                        context.stack().push(player, character_id{});
                        context.advance(2 * sizeof(execute_fn));
                        return context.yield(execution_state::active_character_selection);
                    }
                    return context.jump(resume);
                }
                GIVM_ASSERT(progress.cursor < end);
                const auto header = read_record<mixed_record_header>(context, progress.cursor);
                progress.current = progress.cursor;
                progress.cursor += header.extent;
                if(const auto state = header.execute(library, table, context, random)) return *state;
            }
        }

        template<class TEvent>
        static std::optional<execution_state> process_event(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            const auto progress = get<0>(context.stack().top<settlement_progress>());
            const auto record = read_record<event_record<TEvent>>(context, progress.current);
            const auto targets = collect_all_broadcast_targets<TEvent>(library, table);
            if(targets.empty())
            {
                library.record_history(record.event, table);
                return std::nullopt;
            }
            return queued_broadcast<TEvent>::start(library, table, context, random, record.event, targets,
                continue_records, context.position() + sizeof(execute_fn));
        }

        template<class TEvent, class TId>
        static std::optional<execution_state> process_single_event(const definition_library& library,
            unrestricted_table& table, execution_context& context, random_fn& random)
        {
            const auto progress = get<0>(context.stack().top<settlement_progress>());
            const auto record = read_record<single_event_record<TEvent, TId>>(context, progress.current);
            return queued_single_response<TEvent, TId, false>::start(library, table, context, random,
                record.event, record.entity, continue_records, context.position() + sizeof(execute_fn));
        }

        template<class TSelfEvent, class TEvent, class TId>
        static std::optional<execution_state> process_removal(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            const auto progress = get<0>(context.stack().top<settlement_progress>());
            const auto record = read_record<removal_record<TSelfEvent, TEvent, TId>>(context, progress.current);
            return queued_removal<TSelfEvent, TEvent, TId>::start(library, table, context, random,
                record.entity, record.event, continue_records, context.position() + sizeof(execute_fn));
        }

        static std::optional<execution_state> process_deferred(const definition_library&, unrestricted_table& table,
            execution_context& context, random_fn&)
        {
            const auto progress = get<0>(context.stack().top<settlement_progress>());
            const auto record = read_record<deferred_record>(context, progress.current);
            context.stack().push(notification_continuation{ continue_records, context.position() },
                queued_response_completion{ complete_deferred },
                response_return{ table.state().self_player, context.position() + sizeof(execute_fn) });
            const auto inputs = std::span<const unsigned char>{ context.mixed_events().data()
                + progress.current + record_extent(sizeof(deferred_record)), record.input_size };
            const auto entry = context.copy_program_inputs(record.entry, inputs);
            table.state().self_player = record.player;
            return context.enter(entry);
        }

        static execution_state complete_deferred(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            context.stack().pop<queued_response_completion, response_return>();
            return context.jump(get<0>(context.stack().top<notification_continuation>()).resume);
        }

        static bool all_defeated(const unrestricted_table& table, player_id player)
        {
            for(const auto character : table[player].characters())
                if(character.state().alive) return false;
            return true;
        }

        static execution_state finish_dying(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            const auto progress = get<0>(context.stack().top<segment_seal_progress>());
            auto& record = queue_record<damage_notification_record>(context.damage_events(), progress.current);
            const auto target = table[record.target];
            if(target && target.state().alive && target.state().health == 0)
            {
                auto& state = target.state();
                state.alive = false;
                state.energy = 0;
                state.aura = element_aura::none;
                record.defeated = true;
            }
            return continue_seal(library, table, context, random);
        }

        static execution_state continue_seal(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            auto& progress = get<0>(context.stack().top<segment_seal_progress>());
            for(;;)
            {
                const auto end = context.damage_events().size() - substack_tail_size;
                for(; progress.cursor != end; progress.cursor += record_extent(sizeof(damage_notification_record)))
                {
                    auto& record = queue_record<damage_notification_record>(context.damage_events(), progress.cursor);
                    if(not record.pending_dying) continue;
                    record.pending_dying = false;
                    progress.current = progress.cursor;
                    progress.cursor += record_extent(sizeof(damage_notification_record));
                    const auto target = table[record.target];
                    if(not target || not target.state().alive || target.state().health != 0) break;
                    const auto event = character_will_be_defeated{ record.target };
                    const auto targets = collect_all_broadcast_targets<character_will_be_defeated>(library, table);
                    return queued_broadcast<character_will_be_defeated>::start(library, table, context, random,
                        event, targets, finish_dying, progress.response_completion);
                }
                if(progress.cursor != end) continue;
                const auto begin = current_segment(context);
                bool pending = false;
                bool defeated = false;
                for(auto offset = begin.damage; offset != end; offset += record_extent(sizeof(damage_notification_record)))
                {
                    const auto& record = queue_record<damage_notification_record>(context.damage_events(), offset);
                    pending |= record.pending_dying;
                    defeated |= record.defeated;
                }
                if(pending) { progress.cursor = begin.damage; continue; }
                if(defeated)
                {
                    const auto player0 = all_defeated(table, player_id{ 0 });
                    const auto player1 = all_defeated(table, player_id{ 1 });
                    if(player0 || player1) return context.end_game(player0 && player1 ? game_result::both_loss
                        : player0 ? game_result::player_1_win : game_result::player_0_win);
                }
                const auto hand_end = context.hand_entry_events().size() - substack_tail_size;
                for(auto offset = begin.hand_entry; offset != hand_end; offset += record_extent(sizeof(hand_entry_record)))
                {
                    auto& record = queue_record<hand_entry_record>(context.hand_entry_events(), offset);
                    record.retained = record.overflow || static_cast<bool>(table[record.card]);
                }
                const auto resume = progress.resume;
                context.stack().pop<segment_seal_progress>();
                if(end != begin.damage || hand_end != begin.hand_entry || mixed_end(context) != begin.mixed)
                {
                    const auto boundary = append_record(context, segment_end_record{
                        { process_segment, record_extent(sizeof(segment_end_record)) },
                        begin.damage, end, begin.hand_entry, hand_end });
                    current_domain(context).last_boundary = boundary;
                }
                return context.jump(resume);
            }
        }

        static execution_state seal(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random, execution_position resume, execution_position completion)
        {
            context.stack().push(segment_seal_progress{ current_segment(context).damage, 0, resume, completion });
            return continue_seal(library, table, context, random);
        }

        static std::optional<execution_state> process_segment(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            const auto progress = get<0>(context.stack().top<settlement_progress>());
            const auto record = read_record<segment_end_record>(context, progress.current);
            context.stack().push(segment_dispatch_progress{
                record.hand_begin, record.hand_end, record.damage_begin, record.damage_end });
            return dispatch_hand_entries(library, table, context, random);
        }

        static execution_state dispatch_hand_entries(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            auto& progress = get<0>(context.stack().top<segment_dispatch_progress>());
            while(progress.cursor != progress.hand_end)
            {
                const auto record = queue_record<hand_entry_record>(context.hand_entry_events(), progress.cursor);
                progress.cursor += record_extent(sizeof(hand_entry_record));
                if(not record.retained) continue;
                if(record.kind == hand_entry_kind::added)
                {
                    const auto targets = collect_all_broadcast_targets<hand_card_added>(library, table);
                    return queued_broadcast<hand_card_added>::start(library, table, context, random,
                        hand_card_added{ record.card, record.overflow }, targets,
                        dispatch_hand_entries, context.position() + sizeof(execute_fn));
                }
                const auto targets = collect_all_broadcast_targets<card_drawn>(library, table);
                return queued_broadcast<card_drawn>::start(library, table, context, random,
                    card_drawn{ record.card, record.overflow }, targets,
                    dispatch_hand_entries, context.position() + sizeof(execute_fn));
            }
            progress.cursor = progress.damage_begin;
            return dispatch_damage<false>(library, table, context, random);
        }

        template<bool Defeated>
        static execution_state dispatch_damage(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            auto& progress = get<0>(context.stack().top<segment_dispatch_progress>());
            while(progress.cursor != progress.damage_end)
            {
                const auto record = queue_record<damage_notification_record>(context.damage_events(), progress.cursor);
                progress.cursor += record_extent(sizeof(damage_notification_record));
                if(record.defeated != Defeated) continue;
                const after_damage event{ record.target, record.value, record.type, record.flags,
                    record.reaction, record.defeated };
                const auto targets = collect_all_broadcast_targets<after_damage>(library, table);
                if constexpr(Defeated)
                {
                    get<0>(get<0>(context.stack().top<frame<settlement_progress>, frame<segment_dispatch_progress>>())).cause_defeated = true;
                    context.stack().push(notification_continuation{ dispatch_damage<true>, context.position() });
                    prepare_broadcast<after_damage>(targets, event, table, context.stack(), context.position());
                    context.stack().push(defeated_attachment_progress{ record.target });
                    return clear_defeated_attachments(library, table, context, random);
                }
                else return queued_broadcast<after_damage>::start(library, table, context, random, event, targets,
                    dispatch_damage<false>, context.position() + sizeof(execute_fn));
            }
            if constexpr(not Defeated)
            {
                progress.cursor = progress.damage_begin;
                return dispatch_damage<true>(library, table, context, random);
            }
            else
            {
                context.stack().pop<segment_dispatch_progress>();
                context.stack().push(notification_continuation{ continue_records, context.position() });
                return continue_execution;
            }
        }

        static execution_state clear_defeated_attachments(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            auto& progress = get<0>(context.stack().top<defeated_attachment_progress>());
            const auto attachments = table[progress.target].template attachments<false>();
            while(progress.cursor != attachments.size())
            {
                const auto attachment = attachments[progress.cursor++];
                if(not attachment) continue;
                const auto id = attachment.id();
                attachment.erase();
                return queued_removal<this_attachment_remove, attachment_removed, attachment_id>::start(
                    library, table, context, random, id, attachment_removed{ id }, clear_defeated_attachments,
                    context.position() + sizeof(execute_fn));
            }
            context.stack().pop<defeated_attachment_progress>();
            const auto event = get<2>(context.stack().top<handler_id<after_damage>[], broadcast_progress, after_damage, response_return>());
            library.record_history(event, table);
            return queued_broadcast<after_damage>::resume(library, table, context, random);
        }

        static execution_state start_records(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random, execution_position resume, execution_position loop)
        {
            const auto begin = mixed_begin(context);
            if(begin == mixed_end(context)) return context.jump(resume);
            context.stack().push(settlement_progress{ begin, begin, resume },
                notification_continuation{ continue_records, loop });
            context.jump(loop);
            return continue_execution;
        }

        static execution_state begin(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            return seal(library, table, context, random, context.position() + sizeof(execute_fn),
                context.position() + 3 * sizeof(execute_fn));
        }

        static execution_state start(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            return start_records(library, table, context, random, context.position() + 5 * sizeof(execute_fn),
                context.position() + sizeof(execute_fn));
        }

        static execution_state accept_active_selection(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            const auto frame = context.stack().top<player_id, character_id>();
            const auto player = get<0>(frame);
            const auto selected = get<1>(frame);
            context.stack().pop<player_id, character_id>();
            get<0>(context.stack().top<active_selection_progress>()).selected[player.index()] = selected;
            for(std::uint8_t index = player.index() + 1; index != 2; ++index)
            {
                const player_id next{ index };
                const auto active = table[next].state().active_character;
                if(not active || table[*active].state().alive) continue;
                context.stack().push(next, character_id{});
                return context.yield(execution_state::active_character_selection);
            }
            return context.enter_next();
        }

        static execution_state apply_active_selections(const definition_library& library, unrestricted_table& table,
            execution_context& context, random_fn& random)
        {
            auto& progress = get<0>(context.stack().top<active_selection_progress>());
            while(progress.cursor != 2)
            {
                const auto player = progress.cursor++ == 0 ? progress.first_player : other_player(progress.first_player);
                if(not progress.selected[player.index()]) continue;
                const auto selected = *progress.selected[player.index()];
                table[player].state().active_character = selected;
                table[player].state().can_plunge = true;
                using record = event_record<active_character_changed>;
                append_record(context, record{ { process_event<active_character_changed>, record_extent(sizeof(record)) },
                    active_character_changed{ selected } });
                return start_records(library, table, context, random, context.position(), progress.loop);
            }
            const auto resume = progress.resume;
            context.stack().pop<active_selection_progress>();
            return context.jump(resume);
        }
    };

    inline execution_state begin_settlement(const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        return settlement_driver::begin(library, table, context, random);
    }

    template<class TEvent>
    void append_event_record(execution_context& context, const TEvent& event)
    {
        static_assert(TEvent::category == event_category::normal, "only normal events can enter the event queue");
        append_record(context, event_record<TEvent>{
            { settlement_driver::process_event<TEvent>, record_extent(sizeof(event_record<TEvent>)) }, event });
    }

    template<class TEvent, class TId>
    void append_single_event_record(execution_context& context, TId entity, const TEvent& event)
    {
        static_assert(TEvent::category == event_category::normal, "only normal events can enter the event queue");
        using record = single_event_record<TEvent, TId>;
        append_record(context, record{ { settlement_driver::process_single_event<TEvent, TId>,
            record_extent(sizeof(record)) }, entity, event });
    }

    template<class TSelfEvent, class TEvent, class TId>
    void append_removal_record(execution_context& context, TId entity, const TEvent& event)
    {
        static_assert(TEvent::category == event_category::normal, "only normal events can enter the event queue");
        static_assert(TSelfEvent::category == event_category::normal, "only normal self events can enter the event queue");
        using record = removal_record<TSelfEvent, TEvent, TId>;
        append_record(context, record{ { settlement_driver::process_removal<TSelfEvent, TEvent, TId>,
            record_extent(sizeof(record)) }, entity, event });
    }

    inline void append_deferred_record(execution_context& context, normal_effect entry, optional_player_id player,
        std::span<const unsigned char> inputs)
    {
        const auto extent = record_extent(sizeof(deferred_record)) + record_extent(inputs.size());
        append_record(context, deferred_record{ { settlement_driver::process_deferred, extent }, entry, player, inputs.size() }, inputs);
    }

    inline void compile_settlement(program_writer& writer, execute_fn begin)
    {
        writer.write(begin);
        writer.write(execute_fn{ settlement_driver::start });
        writer.write(execute_fn{ resume_notification });
        writer.write(execute_fn{ complete_queued_response });
        writer.write(execute_fn{ settlement_driver::accept_active_selection });
        writer.write(execute_fn{ settlement_driver::apply_active_selections });
    }
}

#include <givm/macro_undef.hpp>

#endif
