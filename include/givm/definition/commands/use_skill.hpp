#ifndef GIVM_DEFINITION_COMMANDS_USE_SKILL_HPP
#define GIVM_DEFINITION_COMMANDS_USE_SKILL_HPP

#include <cstddef>
#include <string>

#include "../../table.hpp"
#include "../../enums/relative_player.hpp"
#include "../events.hpp"

namespace givm
{
    struct use_skill_error
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

    inline std::string error_string(const use_skill_error& error)
    {
        using reason = use_skill_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "use_skill: cannot consume dynamic input in a root program";
        case reason::invalid_player:
            return "use_skill: player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_definition:
            return "use_skill: definition ID " + std::to_string(error.value) + " is outside skill_view definitions [0, " + std::to_string(error.limit) + ")";
        }
        return {};
    }

    using use_skill_input = skill_effect;

    struct use_skill
    {
        using error_type = use_skill_error;

        using input_type = use_skill_input;

        relative_player player = relative_player::self;
        definition_id<skill_view> definition{};
    };
}

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const use_skill& command) noexcept
    {
        return not command.definition ? TInputTypes::template index_of<use_skill::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
