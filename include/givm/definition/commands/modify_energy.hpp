#ifndef GIVM_DEFINITION_COMMANDS_MODIFY_ENERGY_HPP
#define GIVM_DEFINITION_COMMANDS_MODIFY_ENERGY_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>

#include "../../table.hpp"
#include "../events.hpp"

namespace givm
{
    struct modify_energy_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_target_player,
            invalid_target_selection
        };

        reason cause;
        std::size_t value{};
    };

    inline std::string error_string(const modify_energy_error& error)
    {
        using reason = modify_energy_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "modify_energy: cannot consume dynamic input in a root program";
        case reason::invalid_target_player:
            return "modify_energy: target.player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_target_selection:
            return "modify_energy: target.selection must be character, others or all; got " + std::to_string(error.value);
        }
        return {};
    }

    struct modify_energy_input
    {
        std::span<const character_id> targets;
        std::int64_t delta{};
    };

    struct modify_energy
    {
        using error_type = modify_energy_error;

        using input_type = modify_energy_input;

        relative_character_target target{ {}, std::numeric_limits<std::int32_t>::max() };
        std::int64_t delta{};
    };
}

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const modify_energy& command) noexcept
    {
        return command.target.offset == std::numeric_limits<std::int32_t>::max() ? TInputTypes::template index_of<modify_energy::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
