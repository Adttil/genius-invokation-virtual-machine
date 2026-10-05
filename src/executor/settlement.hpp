#ifndef GIVM_PRIVATE_EXECUTOR_SETTLEMENT_HPP
#define GIVM_PRIVATE_EXECUTOR_SETTLEMENT_HPP

#include <bit>
#include <cstddef>
#include <cstring>
#include <memory>
#include <optional>
#include <span>
#include <type_traits>

#include <givm/executor/executor.hpp>
#include "program_writer.hpp"

#include <givm/macro_define.hpp>

namespace givm::detail
{
    inline constexpr std::size_t settlement_instruction_count = 4;
    inline constexpr std::size_t settlement_extent = settlement_instruction_count * sizeof(execute_fn);

    struct damage_notification_record
    {
        after_damage summary;
        bool pending_dying = false;
    };

    enum class hand_entry_kind : std::uint8_t { added, drawn };

    struct hand_entry_record
    {
        hand_card_id card;
        hand_entry_kind kind;
        bool overflow = false;
        bool retained = true;
    };

    template<class TEvent>
    inline constexpr bool inline_event = std::is_same_v<TEvent, damage_preparation>
        || std::is_same_v<TEvent, damage_calculation> || std::is_same_v<TEvent, damage_effect>
        || std::is_same_v<TEvent, elemental_reaction_will_occur> || std::is_same_v<TEvent, healing>
        || std::is_same_v<TEvent, character_will_be_defeated> || std::is_same_v<TEvent, dice_roll_preparation>
        || std::is_same_v<TEvent, card_will_be_played> || std::is_same_v<TEvent, skill_will_be_used>
        || std::is_same_v<TEvent, changing_energy> || std::is_same_v<TEvent, changing_secret_points>
        || std::is_same_v<TEvent, calculating_card_payment> || std::is_same_v<TEvent, calculating_skill_payment>
        || std::is_same_v<TEvent, calculating_switch_payment>
        || std::is_same_v<TEvent, technique_will_be_used> || std::is_same_v<TEvent, elemental_tuning_modification>;

    using record_execute_fn = std::optional<execution_state> (*)(
        const definition_library&, unrestricted_table&, execution_context&, random_fn&);

    struct mixed_record_header
    {
        record_execute_fn execute;
        std::size_t extent;
    };

    struct segment_end_record
    {
        mixed_record_header header;
        std::size_t damage_begin;
        std::size_t damage_end;
        std::size_t hand_begin;
        std::size_t hand_end;
    };

    struct deferred_record
    {
        mixed_record_header header;
        program_entry entry;
        player_id player;
        std::size_t input_size;
    };

    struct settlement_progress
    {
        std::size_t cursor;
        std::size_t current;
        execution_position resume;
    };

    struct segment_begin
    {
        std::size_t damage;
        std::size_t hand_entry;
        std::size_t mixed;
    };

    decltype(auto) current_domain(auto& context) noexcept
    {
        return get<0>(context.mixed_events().template top<event_domain, substack_t>());
    }

    inline std::size_t mixed_end(const execution_context& context) noexcept
    {
        return context.mixed_events().size() - substack_tail_size;
    }

    inline std::size_t mixed_begin(const execution_context& context) noexcept
    {
        const auto frame = context.mixed_events().top<event_domain, substack_t>();
        return mixed_end(context) - get<1>(frame).size();
    }

    inline void clear_current_domain(execution_context& context)
    {
        auto domain = current_domain(context);
        context.damage_events().pop<substack_t>();
        context.hand_entry_events().pop<substack_t>();
        auto& records = context.mixed_events();
        records.pop<event_domain, substack_t>();
        domain.last_boundary = no_boundary;
        // The empty replacements fit in the capacity of the frames just removed.
        context.damage_events().push(substack());
        context.hand_entry_events().push(substack());
        records.push(domain, substack());
    }

    constexpr std::size_t record_extent(std::size_t size) noexcept
    {
        return (size + max_alignment - 1) / max_alignment * max_alignment;
    }

    template<class T>
    T read_record(const execution_context& context, std::size_t offset)
    {
        const auto& records = context.mixed_events();
        GIVM_ASSERT(offset <= records.size() && sizeof(T) <= records.size() - offset);
        return *std::launder(reinterpret_cast<const T*>(records.data() + offset));
    }

    template<class T>
    std::size_t append_record(execution_context& context, const T& record, std::span<const unsigned char> payload = {})
    {
        auto& records = context.mixed_events();
        const auto offset = mixed_end(context);
        const auto required = records.size() + record.header.extent;
        if(required > records.capacity()) records.reserve(std::bit_ceil(required));
        auto destination = get<1>(records.top<event_domain, substack_t>());
        for(std::size_t index = 0; index < record.header.extent; index += max_alignment)
            destination.push<unsigned char[max_alignment]>();
        std::memcpy(records.data() + offset, &record, sizeof(T));
        if(not payload.empty())
            std::memcpy(records.data() + offset + record_extent(sizeof(T)), payload.data(), payload.size());
        return offset;
    }

    inline segment_begin current_segment(const execution_context& context)
    {
        const auto& domain = current_domain(context);
        if(domain.last_boundary == no_boundary)
        {
            const auto& damage = context.damage_events();
            const auto& hand_entry = context.hand_entry_events();
            return {
                damage.size() - substack_tail_size - get<0>(damage.top<substack_t>()).size(),
                hand_entry.size() - substack_tail_size - get<0>(hand_entry.top<substack_t>()).size(),
                mixed_begin(context) };
        }
        const auto boundary = read_record<segment_end_record>(context, domain.last_boundary);
        return { boundary.damage_end, boundary.hand_end, domain.last_boundary + boundary.header.extent };
    }

#ifndef NDEBUG
    inline bool is_inline_response(const execution_context& context) noexcept
    {
        return current_domain(context).inline_response;
    }
#endif

    template<bool Independent>
    void begin_response(execution_context& context)
    {
        if constexpr(Independent)
        {
            context.damage_events().push(substack());
            context.hand_entry_events().push(substack());
            context.mixed_events().push(event_domain{}, substack());
        }
#ifndef NDEBUG
        else
        {
            auto& domain = current_domain(context);
            get<0>(context.stack().top<response_return>()).previous_inline = domain.inline_response;
            domain.inline_response = true;
        }
#endif
    }

    template<bool Independent>
    void end_response(execution_context& context)
    {
        if constexpr(Independent)
        {
            context.damage_events().pop<substack_t>();
            context.hand_entry_events().pop<substack_t>();
            context.mixed_events().pop<event_domain, substack_t>();
        }
#ifndef NDEBUG
        else current_domain(context).inline_response = get<0>(context.stack().top<response_return>()).previous_inline;
#endif
    }

    inline std::optional<execution_state> process_segment_end(
        const definition_library&, unrestricted_table&, execution_context& context, random_fn&)
    {
        const auto progress = get<0>(context.stack().top<settlement_progress>());
        const auto record = read_record<segment_end_record>(context, progress.current);
        // The first migration has no damage or hand-notification producers.
        // Boundaries already retain the separate ranges used by their domain handlers.
        GIVM_ASSERT(record.damage_begin == record.damage_end);
        GIVM_ASSERT(record.hand_begin == record.hand_end);
        return std::nullopt;
    }

    inline std::optional<execution_state> process_deferred_record(
        const definition_library&, unrestricted_table& table, execution_context& context, random_fn&)
    {
        const auto progress = get<0>(context.stack().top<settlement_progress>());
        const auto record = read_record<deferred_record>(context, progress.current);
        context.stack().push(response_return{ table.state().self_player, context.position() + sizeof(execute_fn) });
        begin_response<true>(context);
        const auto& records = context.mixed_events();
        const auto inputs = std::span<const unsigned char>{
            records.data() + progress.current + record_extent(sizeof(deferred_record)), record.input_size };
        const auto entry = context.copy_program_inputs(record.entry, inputs);
        table.state().self_player = record.player;
        return context.enter(entry);
    }

    inline execution_state continue_settlement(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        for(;;)
        {
            auto& progress = get<0>(context.stack().top<settlement_progress>());
            // A dispatched response writes into a child domain. This domain is
            // back at the top, with unchanged contents, whenever we resume here.
            const auto end = mixed_end(context);
            if(progress.cursor == end)
            {
                const auto resume = progress.resume;
                clear_current_domain(context);
                context.stack().pop<settlement_progress>();
                return context.jump(resume);
            }
            GIVM_ASSERT(progress.cursor < end);
            const auto header = read_record<mixed_record_header>(context, progress.cursor);
            GIVM_ASSERT(header.extent != 0 && header.extent <= end - progress.cursor);
            progress.current = progress.cursor;
            progress.cursor += header.extent;
            if(const auto state = header.execute(library, table, context, random)) return *state;
        }
    }

    inline void seal_segment(execution_context& context)
    {
        const auto begin = current_segment(context);
        const auto damage_end = context.damage_events().size() - substack_tail_size;
        const auto hand_end = context.hand_entry_events().size() - substack_tail_size;
        if(damage_end == begin.damage && hand_end == begin.hand_entry && mixed_end(context) == begin.mixed) return;
        const auto boundary = append_record(context, segment_end_record{
            { process_segment_end, record_extent(sizeof(segment_end_record)) },
            begin.damage, damage_end, begin.hand_entry, hand_end });
        current_domain(context).last_boundary = boundary;
    }

    inline execution_state settle_records(const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random, execution_position resume, execution_position loop)
    {
        seal_segment(context);
        const auto begin = mixed_begin(context);
        const auto end = mixed_end(context);
        if(begin == end) return context.jump(resume);
        context.stack().push(settlement_progress{ begin, begin, resume });
        context.jump(loop);
        return continue_settlement(library, table, context, random);
    }

    inline execution_state begin_settlement(const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        return settle_records(library, table, context, random,
            context.position() + settlement_extent, context.position() + sizeof(execute_fn));
    }

    inline execution_state return_deferred_response(const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        return settle_records(library, table, context, random,
            context.position() + sizeof(execute_fn), context.position() - sizeof(execute_fn));
    }

    inline void append_deferred_record(execution_context& context, program_entry entry, player_id player,
        std::span<const unsigned char> inputs)
    {
        const auto extent = record_extent(sizeof(deferred_record)) + record_extent(inputs.size());
        append_record(context, deferred_record{ { process_deferred_record, extent }, entry, player, inputs.size() }, inputs);
    }

    inline execution_state complete_deferred_response(const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        end_response<true>(context);
        context.stack().pop<response_return>();
        context.jump(context.position() - 2 * sizeof(execute_fn));
        return continue_settlement(library, table, context, random);
    }

    inline void compile_settlement(program_writer& writer, execute_fn begin)
    {
        writer.write(begin);
        writer.write(execute_fn{ continue_settlement });
        writer.write(execute_fn{ return_deferred_response });
        writer.write(execute_fn{ complete_deferred_response });
    }
}

#include <givm/macro_undef.hpp>

#endif
