#ifndef GIVM_DEFINITION_COMMANDS_DEAL_DAMAGE_HPP
#define GIVM_DEFINITION_COMMANDS_DEAL_DAMAGE_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>

#include "../../enums/relative_player.hpp"
#include "../../enums/damage_flags.hpp"
#include "../../enums/damage_type.hpp"
#include "../events.hpp"

namespace givm
{
    struct deal_damage_error
    {
        enum class reason
        {
            dynamic_input_in_root,
            invalid_source_player,
            invalid_source_selection,
            invalid_target_player,
            invalid_target_selection,
            zero_multiplier_denominator,
            invalid_damage_type
        };

        reason cause;
        std::size_t value{};
    };

    inline std::string error_string(const deal_damage_error& error)
    {
        using reason = deal_damage_error::reason;
        switch(error.cause)
        {
        case reason::dynamic_input_in_root:
            return "deal_damage: cannot consume dynamic input in a root program";
        case reason::invalid_source_player:
            return "deal_damage: source.player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_source_selection:
            return "deal_damage: source.selection must be character; got " + std::to_string(error.value);
        case reason::invalid_target_player:
            return "deal_damage: target.player must be self or opponent; got " + std::to_string(error.value);
        case reason::invalid_target_selection:
            return "deal_damage: target.selection must be character, others, all or prioritized; got " + std::to_string(error.value);
        case reason::zero_multiplier_denominator:
            return "deal_damage: multiplier_denominator must not be zero";
        case reason::invalid_damage_type:
            return "deal_damage: type must be a declared damage_type; got " + std::to_string(error.value);
        }
        return {};
    }

    struct damage
    {
        damage_source_id source;
        damage_target target;
        character_selection selection = character_selection::character;
        std::uint32_t value;
        std::uint16_t multiplier_numerator = 1;
        std::uint16_t multiplier_denominator = 1;
        damage_type type;
        damage_flags flags;
    };

    struct deal_damage_input
    {
        std::span<const damage> damages;
    };

    struct deal_damage
    {
        using error_type = deal_damage_error;
        using input_type = deal_damage_input;

        relative_character_target source{};
        relative_character_target target{ relative_player::opponent, std::numeric_limits<std::int32_t>::max() };
        std::uint32_t value{};
        std::uint16_t multiplier_numerator = 1;
        std::uint16_t multiplier_denominator = 1;
        damage_type type;
        damage_flags flags;
    };

}

#endif
