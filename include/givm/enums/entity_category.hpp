#ifndef GIVM_ENUMS_ENTITY_CATEGORY_HPP
#define GIVM_ENUMS_ENTITY_CATEGORY_HPP

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>

#include "definition_category.hpp"

namespace givm
{
    enum class entity_category : std::uint8_t
    {
        player,
        hand_card,
        deck_card,
        hand_card_status,
        deck_card_status,
        support,
        summon,
        combat_status,
        character,
        skill,
        attachment,
        reaction,
        null
    };

}

namespace givm::detail
{
    inline constexpr std::array definition_categories_of_entity{
        definition_category::null,
        definition_category::card,
        definition_category::card,
        definition_category::card_status,
        definition_category::card_status,
        definition_category::support,
        definition_category::summon,
        definition_category::combat_status,
        definition_category::character,
        definition_category::skill,
        definition_category::attachment,
        definition_category::reaction,
        definition_category::null
    };

    inline constexpr unsigned entity_category_bit_width =
        std::bit_width(static_cast<unsigned>(entity_category::null));
    inline constexpr unsigned entity_id_bit_width = 64 - entity_category_bit_width;
    inline constexpr unsigned entity_parent_bit_width = entity_id_bit_width - 32 - 1;

    struct definition_entity_categories
    {
        std::array<entity_category, static_cast<std::size_t>(entity_category::null)> values{};
        std::array<std::size_t, static_cast<std::size_t>(definition_category::null) + 2> offsets{};
    };

    inline constexpr auto entity_categories_of_definition = []
    {
        definition_entity_categories result;
        std::size_t count{};
        for(std::size_t d = 0; d != static_cast<std::size_t>(definition_category::null); ++d)
        {
            result.offsets[d] = count;
            for(std::size_t e = 0; e != static_cast<std::size_t>(entity_category::null); ++e)
            {
                if(definition_categories_of_entity[e] == static_cast<definition_category>(d))
                    result.values[count++] = static_cast<entity_category>(e);
            }
        }
        result.offsets[static_cast<std::size_t>(definition_category::null)] = count;
        result.offsets[static_cast<std::size_t>(definition_category::null) + 1] = count;
        return result;
    }();
}

namespace givm
{
    template<entity_category Category>
    inline constexpr definition_category definition_category_of =
        detail::definition_categories_of_entity[static_cast<std::size_t>(Category)];

    template<definition_category Category>
    inline constexpr std::span<const entity_category> entity_categories_of{
        detail::entity_categories_of_definition.values.data()
            + detail::entity_categories_of_definition.offsets[static_cast<std::size_t>(Category)],
        detail::entity_categories_of_definition.offsets[static_cast<std::size_t>(Category) + 1]
            - detail::entity_categories_of_definition.offsets[static_cast<std::size_t>(Category)]
    };
}

#endif
