#ifndef GIVM_ENUMS_SKILL_KIND_HPP
#define GIVM_ENUMS_SKILL_KIND_HPP

#include <cstdint>

namespace givm
{
    enum class skill_kind : std::uint8_t
    {
        normal_attack,
        elemental_skill,
        elemental_burst,
        passive,
        technique
    };
}

#endif
