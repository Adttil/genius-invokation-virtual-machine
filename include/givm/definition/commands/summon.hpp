#ifndef GIVM_DEFINITION_COMMANDS_SUMMON_HPP
#define GIVM_DEFINITION_COMMANDS_SUMMON_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"

namespace givm
{
    struct summon_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_player,
            invalid_definition
        };

        reason cause;
        std::size_t value{};
        std::size_t limit{};
    };

    inline std::string error_string(const summon_error& error)
    {
        using reason = summon_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "summon: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "summon: player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "summon: definition ID " + std::to_string(error.value) + " is outside summon_view definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct summon_input
    {
        player_id player;
        definition_id<summon_view> definition;
        summon_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };

    struct summon
    {
        using error_type = summon_error;

        using input_type = summon_input;

        relative_player player = relative_player::self;
        definition_id<summon_view> definition{};
        summon_state state{ std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    };
}

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const summon& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<summon::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
