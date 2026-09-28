#ifndef GIVM_DEFINITION_COMMANDS_HEAL_HPP
#define GIVM_DEFINITION_COMMANDS_HEAL_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include "../events.hpp"

namespace givm
{
    struct heal_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_source_player,
            invalid_source_selection,
            invalid_target_player,
            invalid_target_selection
        };

        reason cause;
        std::size_t value{};
    };

    inline std::string error_string(const heal_error& error)
    {
        using reason = heal_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "heal: cannot consume dynamic input in a root program";
        case reason::invalid_source_player:
            return "heal: source.player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_source_selection:
            return "heal: source.selection must be character; got " + std::to_string(error.value);
        case reason::invalid_target_player:
            return "heal: target.player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_target_selection:
            return "heal: target.selection must be character, others or all; got " + std::to_string(error.value);
        }
        return {};
    }

    struct heal_input
    {
        effect_source_id source;
        healing_target target;
        std::uint32_t value;
    };

    struct heal
    {
        using error_type = heal_error;

        using input_type = heal_input;

        relative_character_target source{};
        relative_character_target target{ {}, std::numeric_limits<std::int32_t>::max() };
        std::uint32_t value{};
    };
}

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const heal& command) noexcept
    {
        return command.target.offset == std::numeric_limits<std::int32_t>::max() ? TInputTypes::template index_of<heal::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
