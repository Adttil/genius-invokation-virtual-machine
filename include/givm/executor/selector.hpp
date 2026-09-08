#ifndef GIVM_EXECUTOR_SELECTOR_HPP
#define GIVM_EXECUTOR_SELECTOR_HPP

#include <bitset>
#include <cstddef>

#include "../table/entity_id.hpp"

namespace givm
{
    inline constexpr size_t selection_capacity = 64;

    struct selector
    {
        player_id player;
        std::bitset<selection_capacity> selected;
    };
}

#endif
