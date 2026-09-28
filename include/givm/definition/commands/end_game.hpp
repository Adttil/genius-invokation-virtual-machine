#ifndef GIVM_DEFINITION_COMMANDS_END_GAME_HPP
#define GIVM_DEFINITION_COMMANDS_END_GAME_HPP

#include <cstddef>
#include <string>

#include "../../enums/game_result.hpp"

namespace givm
{
    struct end_game_error
    {
        enum class reason
        {
            invalid_result
        };

        reason cause;
        std::size_t value{};
    };

    inline std::string error_string(const end_game_error& error)
    {
        using reason = end_game_error::reason;
        switch(error.cause)
        {
        case reason::invalid_result:
            return "end_game: result must declare a winning player or both_loss; got " + std::to_string(error.value);
        }
        return {};
    }

    struct end_game
    {
        using error_type = end_game_error;

        game_result result;
    };
}

#endif
