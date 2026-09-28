#ifndef GIVM_DEFINITION_COMMANDS_SET_COMBAT_STATUS_STATE_HPP
#define GIVM_DEFINITION_COMMANDS_SET_COMBAT_STATUS_STATE_HPP

#include <cstddef>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"

namespace givm
{
    struct set_combat_status_state_error
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

    inline std::string error_string(const set_combat_status_state_error& error)
    {
        using reason = set_combat_status_state_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "set_combat_status_state: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "set_combat_status_state: player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "set_combat_status_state: definition ID " + std::to_string(error.value) + " is outside combat_status_view definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct set_combat_status_state_input
    {
        combat_status_id status;
        combat_status_state state;
    };

    struct set_combat_status_state
    {
        using error_type = set_combat_status_state_error;

        using input_type = set_combat_status_state_input;

        relative_player player = relative_player::self;
        definition_id<combat_status_view> definition{};
        combat_status_state state{};
        bool ignore_limit = false;
    };
}

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const set_combat_status_state& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<set_combat_status_state::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
