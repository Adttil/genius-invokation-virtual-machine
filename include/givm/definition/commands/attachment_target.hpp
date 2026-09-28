#ifndef GIVM_DEFINITION_COMMANDS_ATTACHMENT_TARGET_HPP
#define GIVM_DEFINITION_COMMANDS_ATTACHMENT_TARGET_HPP

#include <variant>

#include "../../table.hpp"
#include "../../enums/equipment_type.hpp"
#include "../events.hpp"

namespace givm
{
    struct equipment_target
    {
        character_id character;
        equipment_type type;
    };

    using attachment_target = std::variant<attachment_id, equipment_target>;

    struct relative_attachment_target
    {
        relative_character_target character{};
        std::variant<definition_id<attachment_view>, equipment_type> selector{};
    };
}

#endif
