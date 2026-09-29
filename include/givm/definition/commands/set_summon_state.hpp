#ifndef GIVM_DEFINITION_COMMANDS_SET_SUMMON_STATE_HPP
#define GIVM_DEFINITION_COMMANDS_SET_SUMMON_STATE_HPP

#include <cstddef>
#include <span>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"

namespace givm
{
    struct set_summon_state_error
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

    inline std::string error_string(const set_summon_state_error& error)
    {
        using reason = set_summon_state_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "set_summon_state: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "set_summon_state: player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "set_summon_state: definition ID " + std::to_string(error.value) + " is outside summon_view definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct set_summon_state_input
    {
        struct change
        {
            summon_id summon;
            summon_state state;
        };

        std::span<const change> changes;
    };

    struct set_summon_state
    {
        using error_type = set_summon_state_error;

        using input_type = set_summon_state_input;

        relative_player player = relative_player::self;
        definition_id<summon_view> definition{};
        summon_state state{};
        bool ignore_limit = false;
    };
}

#endif
