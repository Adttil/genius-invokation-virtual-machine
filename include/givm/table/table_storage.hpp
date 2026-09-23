#ifndef GIVM_TABLE_TABLE_STORAGE_HPP
#define GIVM_TABLE_TABLE_STORAGE_HPP

#include <cstdint>
#include <vector>

#include "../enums/game_result.hpp"
#include "data/player_data.hpp"
#include "data/status_data.hpp"
#include "entity_id.hpp"

namespace givm
{
    struct table_state
    {
        std::uint32_t round_number = 0;
        std::uint32_t max_rounds = 14;
        player_id active_player{ 0 };
        bool first_ended = false;
        player_id self_player{ 2 };
    };

    namespace detail
    {
        struct table_storage
        {
            table_state state;
            player_data player_datas[2]{};
            std::vector<status_slot> status_slots;
        };
    }
}

#endif
