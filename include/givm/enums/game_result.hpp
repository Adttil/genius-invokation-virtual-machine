#ifndef GIVM_ENUMS_GAME_RESULT_HPP
#define GIVM_ENUMS_GAME_RESULT_HPP

#include <cstdint>

namespace givm
{
    enum class game_result : std::uint8_t
    {
        no_result = 0,
        player_0_win = 1,
        player_1_win = 2,
        both_loss = 3
    };
}

#endif
