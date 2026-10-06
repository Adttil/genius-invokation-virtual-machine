#ifndef GIVM_DEFINITION_COMMANDS_MODIFY_SUMMON_STATE_HPP
#define GIVM_DEFINITION_COMMANDS_MODIFY_SUMMON_STATE_HPP

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <tuple>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"
#include "../../utils/stack.hpp"

namespace givm
{
    struct modify_summon_state_error
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

    inline std::string error_string(const modify_summon_state_error& error)
    {
        using reason = modify_summon_state_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "modify_summon_state: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "modify_summon_state: player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "modify_summon_state: definition ID " + std::to_string(error.value) + " is outside summon_view definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct modify_summon_state_input
    {
        std::span<const summon_id> summons;
        std::int64_t value{};
        std::int64_t usages{};
    };

    struct modify_summon_state
    {
        using error_type = modify_summon_state_error;

        using input_type = modify_summon_state_input;

        relative_player player = relative_player::self;
        definition_id<summon_view> definition{};
        std::int64_t value{};
        std::int64_t usages{};
        bool ignore_limit = false;
    };
}

namespace givm::detail
{
    inline auto command_input_members(const modify_summon_state_input& input) noexcept
    {
        return std::tuple{ dynamic_array<summon_id>(input.summons), input.value, input.usages };
    }
}

#endif
