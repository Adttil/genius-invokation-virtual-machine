#ifndef GIVM_DEFINITION_COMMANDS_SET_ATTACHMENT_STATE_HPP
#define GIVM_DEFINITION_COMMANDS_SET_ATTACHMENT_STATE_HPP

#include <cstddef>
#include <string>
#include <variant>

#include "../../table.hpp"
#include "attachment_target.hpp"

namespace givm
{
    struct set_attachment_state_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_definition,
            invalid_equipment_type,
            invalid_target_character_player,
            invalid_target_character_selection
        };

        reason cause;
        std::size_t value{};
        std::size_t limit{};
    };

    inline std::string error_string(const set_attachment_state_error& error)
    {
        using reason = set_attachment_state_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "set_attachment_state: cannot consume dynamic input in a root program";
        case reason::invalid_definition:
            return "set_attachment_state: target.selector definition ID " + std::to_string(error.value) + " is outside attachment_view definitions [0, " + std::to_string(error.limit) + ")";
        case reason::invalid_equipment_type:
            return "set_attachment_state: target.selector must be an equipment type before none; got " + std::to_string(error.value);
        case reason::invalid_target_character_player:
            return "set_attachment_state: target.character.player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_target_character_selection:
            return "set_attachment_state: target.character.selection must be character; got " + std::to_string(error.value);
        }
        return {};
    }

    struct set_attachment_state_input
    {
        attachment_target attachment;
        attachment_state state;
    };

    struct set_attachment_state
    {
        using error_type = set_attachment_state_error;

        using input_type = set_attachment_state_input;

        relative_attachment_target target{};
        attachment_state state{};
        bool ignore_limit = false;
    };
}

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const set_attachment_state& command) noexcept
    {
        const auto* definition = std::get_if<definition_id<attachment_view>>(&command.target.selector);
        return definition && not *definition ? TInputTypes::template index_of<set_attachment_state::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
