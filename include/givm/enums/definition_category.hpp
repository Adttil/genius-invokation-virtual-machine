#ifndef GIVM_ENUMS_DEFINITION_CATEGORY_HPP
#define GIVM_ENUMS_DEFINITION_CATEGORY_HPP

#include <bit>
#include <cstdint>

namespace givm
{
    enum class definition_category : std::uint8_t
    {
        card,
        card_status,
        support,
        summon,
        combat_status,
        character,
        skill,
        attachment,
        history_summary,
        reaction,
        null
    };
}

namespace givm::detail
{
    inline constexpr unsigned definition_category_bit_width =
        std::bit_width(static_cast<unsigned>(definition_category::null));
    inline constexpr unsigned definition_id_bit_width = 64 - definition_category_bit_width;
    inline constexpr std::uint64_t definition_index_mask =
        (std::uint64_t{ 1 } << definition_id_bit_width) - 1;
}

#endif
