#ifndef GIVM_PRIVATE_EXECUTOR_EVENT_QUEUES_HPP
#define GIVM_PRIVATE_EXECUTOR_EVENT_QUEUES_HPP

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
    inline constexpr std::size_t settlement_instruction_count = 6;
    inline constexpr std::size_t settlement_extent = settlement_instruction_count * sizeof(execute_fn);

    struct damage_notification_record
    {
        character_id target;
        std::uint32_t value;
        damage_type_mask type;
        damage_flags flags;
        elemental_reaction_mask reaction;
        bool defeated = false;
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
        normal_effect entry;
        optional_player_id player;
        std::size_t input_size;
    };

    struct queued_response_completion
    {
        execute_fn complete;
    };

    struct notification_continuation
    {
        execute_fn next;
        execution_position resume;
    };

    struct settlement_progress
    {
        std::size_t cursor;
        std::size_t current;
        execution_position resume;
        bool cause_defeated = false;
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

    template<bool Independent>
    void begin_response(execution_context& context)
    {
        if constexpr(Independent)
        {
            context.damage_events().push(substack());
            context.hand_entry_events().push(substack());
            context.mixed_events().push(event_domain{}, substack());
        }
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
    }

}

#include <givm/macro_undef.hpp>

#endif
