#ifndef GIVM_TABLE_TABLE_STORAGE_HPP
#define GIVM_TABLE_TABLE_STORAGE_HPP

#include <cstdint>
#include <vector>

#include "../definition.hpp"
#include "../enums/game_result.hpp"
#include "data/player_data.hpp"
#include "data/status_data.hpp"
#include "entity_id.hpp"

namespace givm
{
    struct game_parameters
    {
        std::uint32_t hand_limit = 10;
        std::uint32_t support_limit = 4;
        std::uint32_t summon_limit = 4;
    };

    struct table_state
    {
        std::uint32_t round_number = 0;
        player_id active_player{ 0 };
        bool first_ended = false;
    };

    namespace detail
    {
        struct table_storage
        {
            const definition_library* definition_library_;
            game_parameters parameters;
            table_state state;
            player_data player_datas[2]{};
            std::vector<status_slot> status_slots;
        };
    }
}

#endif
