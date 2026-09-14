#include <array>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <givm/enums/damage_type.hpp>

TEST_CASE("damage types map to their applied element", "[damage_type]")
{
    constexpr std::array elemental_types{
        std::pair{ givm::damage_type::cryo, givm::element::cryo },
        std::pair{ givm::damage_type::hydro, givm::element::hydro },
        std::pair{ givm::damage_type::pyro, givm::element::pyro },
        std::pair{ givm::damage_type::electro, givm::element::electro },
        std::pair{ givm::damage_type::anemo, givm::element::anemo },
        std::pair{ givm::damage_type::geo, givm::element::geo },
        std::pair{ givm::damage_type::dendro, givm::element::dendro }
    };
    for(const auto [type, expected] : elemental_types)
    {
        CHECK(element_from_damage_type(type) == expected);
    }

    CHECK(element_from_damage_type(givm::damage_type::physical) == givm::element::none);
    CHECK(element_from_damage_type(givm::damage_type::piercing) == givm::element::none);
    CHECK(element_from_damage_type(givm::damage_type::true_damage) == givm::element::none);
}
