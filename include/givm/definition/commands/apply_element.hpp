#ifndef GIVM_DEFINITION_COMMANDS_APPLY_ELEMENT_HPP
#define GIVM_DEFINITION_COMMANDS_APPLY_ELEMENT_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include "../../table.hpp"
#include "../../enums/element_application_cause.hpp"
#include "../../enums/element.hpp"
#include "../events.hpp"

namespace givm
{
    struct apply_element_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_source_player,
            invalid_source_selection,
            invalid_target_player,
            invalid_target_selection,
            invalid_element,
            invalid_cause
        };

        reason cause;
        std::size_t value{};
    };

    inline std::string error_string(const apply_element_error& error)
    {
        using reason = apply_element_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "apply_element: cannot consume dynamic input in a root program";
        case reason::invalid_source_player:
            return "apply_element: source.player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_source_selection:
            return "apply_element: source.selection must be character; got " + std::to_string(error.value);
        case reason::invalid_target_player:
            return "apply_element: target.player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_target_selection:
            return "apply_element: target.selection must be character; got " + std::to_string(error.value);
        case reason::invalid_element:
            return "apply_element: element must be a declared element value; got " + std::to_string(error.value);
        case reason::invalid_cause:
            return "apply_element: cause must be effect or damage; got " + std::to_string(error.value);
        }
        return {};
    }

    struct apply_element_input
    {
        element_application_source_id source;
        character_id target;
        element element;
        element_application_cause cause = element_application_cause::effect;
    };

    struct apply_element
    {
        using error_type = apply_element_error;

        using input_type = apply_element_input;

        relative_character_target source{};
        relative_character_target target{ {}, std::numeric_limits<std::int32_t>::max() };
        element element;
        element_application_cause cause = element_application_cause::effect;
    };
}

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const apply_element& command) noexcept
    {
        return command.target.offset == std::numeric_limits<std::int32_t>::max() ? TInputTypes::template index_of<apply_element::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
