#ifndef GIVM_ENUMS_EVENT_CATEGORY_HPP
#define GIVM_ENUMS_EVENT_CATEGORY_HPP

#include <cstdint>

namespace givm
{
    enum class event_category : std::uint8_t { normal, immediate, preview };
}

#endif
