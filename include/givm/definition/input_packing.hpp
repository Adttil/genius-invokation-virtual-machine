#ifndef GIVM_DEFINITION_INPUT_PACKING_HPP
#define GIVM_DEFINITION_INPUT_PACKING_HPP

#include <cstddef>
#include <cstring>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>

#include "any_command.hpp"
#include "program_inputs.hpp"

namespace givm::detail
{
    template<class T>
    concept command_input = requires { command_input_types::index_of<std::remove_cvref_t<T>>(); };

    template<command_input T>
    constexpr auto command_input_members(const T& input) noexcept
    {
        return std::tie(input);
    }

    inline auto command_input_members(const deal_damage_input& input) noexcept
    {
        return std::tuple{ dynamic_array<damage>(input.damages) };
    }

    inline auto command_input_members(const discard_hand_card_input& input) noexcept
    {
        return std::tuple{ dynamic_array<hand_card_id>(input.cards) };
    }

    inline auto command_input_members(const draw_cards_input& input) noexcept
    {
        return std::tuple{ dynamic_array<deck_card_id>(input.cards) };
    }

    inline auto command_input_members(const set_summon_state_input& input) noexcept
    {
        return std::tuple{ dynamic_array<set_summon_state_input::change>(input.changes) };
    }

    inline auto command_input_members(const modify_summon_state_input& input) noexcept
    {
        return std::tuple{ dynamic_array<summon_id>(input.summons), input.value, input.usages };
    }

    inline auto command_input_members(const remove_summon_input& input) noexcept
    {
        return std::tuple{ dynamic_array<summon_id>(input.summons) };
    }

    inline auto command_input_members(const modify_energy_input& input) noexcept
    {
        return std::tuple{ dynamic_array<character_id>(input.targets), input.delta };
    }

    template<class TDestination>
    inline void append_input_bytes(TDestination& destination, std::span<const unsigned char> bytes)
    {
        for(std::size_t offset = 0; offset != bytes.size(); offset += max_alignment)
        {
            auto frame = destination.template push<unsigned char[max_alignment]>();
            std::memcpy(get<0>(frame), bytes.data() + offset, max_alignment);
        }
    }

    template<class TDestination, command_input T>
    inline void push_command_input(TDestination& destination, const T& input)
    {
        std::apply([&](const auto&... members) { destination.push(members...); }, command_input_members(input));
    }

    template<class TDestination>
    inline void push_command_input(TDestination& destination, const defer_program_input& input)
    {
        auto frame = destination.push(input.entry, substack());
        auto inputs = get<1>(frame);
        append_input_bytes(inputs, input.inputs.bytes());
    }

#ifndef NDEBUG
    template<command_input T>
    inline program_input_description describe_command_input(const T& input)
    {
        using input_type = std::remove_cvref_t<T>;
        program_input_description result{ command_input_types::index_of<input_type>() };
        if constexpr(std::is_same_v<input_type, return_response_input>)
            result.response_index = input.index;
        else if constexpr(std::is_same_v<input_type, defer_program_input>)
        {
            result.entry = input.entry;
            const auto children = input.inputs.descriptions();
            result.children.assign(children.begin(), children.end());
        }
        return result;
    }
#endif

    struct program_inputs_builder
    {
        template<command_input... T>
        static program_inputs pack(const T&... inputs)
        {
            program_inputs result;
#ifndef NDEBUG
            result.descriptions_.reserve(sizeof...(T));
            (result.descriptions_.push_back(describe_command_input(inputs)), ...);
#endif
            const auto values = std::forward_as_tuple(inputs...);
            [&]<std::size_t... I>(std::index_sequence<I...>)
            {
                (push_command_input(result.frames_, std::get<sizeof...(T) - 1 - I>(values)), ...);
            }(std::index_sequence_for<T...>{});
            return result;
        }

        static program_inputs concat(std::span<const program_inputs> parts)
        {
            program_inputs result;
            std::size_t bytes = 0;
            for(const auto& part : parts) bytes += part.bytes().size();
            if(bytes != 0) result.frames_.reserve(bytes);
            for(auto iter = parts.rbegin(); iter != parts.rend(); ++iter)
                append_input_bytes(result.frames_, iter->bytes());
#ifndef NDEBUG
            std::size_t count = 0;
            for(const auto& part : parts) count += part.descriptions_.size();
            result.descriptions_.reserve(count);
            for(const auto& part : parts)
                result.descriptions_.insert(result.descriptions_.end(), part.descriptions_.begin(), part.descriptions_.end());
#endif
            return result;
        }
    };
}

namespace givm
{
    template<detail::command_input... T>
    inline program_inputs pack_inputs(const T&... inputs)
    {
        return detail::program_inputs_builder::pack(inputs...);
    }

    inline program_inputs concat_inputs(std::span<const program_inputs> parts)
    {
        return detail::program_inputs_builder::concat(parts);
    }

    inline defer_program_input defer_invoke(program_entry entry, program_inputs inputs)
    {
        return { entry, std::move(inputs) };
    }

    template<detail::command_input... T>
    inline defer_program_input defer_invoke(program_entry entry, const T&... inputs)
    {
        return { entry, pack_inputs(inputs...) };
    }
}

#endif
