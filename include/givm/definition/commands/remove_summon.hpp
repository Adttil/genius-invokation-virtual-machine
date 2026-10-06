#ifndef GIVM_DEFINITION_COMMANDS_REMOVE_SUMMON_HPP
#define GIVM_DEFINITION_COMMANDS_REMOVE_SUMMON_HPP

#include <cstddef>
#include <span>
#include <string>
#include <tuple>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"
#include "../../utils/stack.hpp"

namespace givm
{
    struct remove_summon_error
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

    inline std::string error_string(const remove_summon_error& error)
    {
        using reason = remove_summon_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "remove_summon: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "remove_summon: player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "remove_summon: definition ID " + std::to_string(error.value) + " is outside summon_view definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    struct remove_summon_input
    {
        std::span<const summon_id> summons;
    };

    struct remove_summon
    {
        using error_type = remove_summon_error;

        using input_type = remove_summon_input;

        relative_player player = relative_player::self;
        definition_id<summon_view> definition{};
    };
}

namespace givm::detail
{
    inline auto command_input_members(const remove_summon_input& input) noexcept
    {
        return std::tuple{ dynamic_array<summon_id>(input.summons) };
    }
}

#endif
