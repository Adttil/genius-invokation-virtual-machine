#include <array>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <givm/enums/damage_type.hpp>

using namespace givm;

TEST_CASE("damage types map to their applied element", "[damage_type]")
{
    constexpr std::array elemental_types{
        std::pair{ damage_type::cryo, element::cryo },
        std::pair{ damage_type::hydro, element::hydro },
        std::pair{ damage_type::pyro, element::pyro },
        std::pair{ damage_type::electro, element::electro },
        std::pair{ damage_type::anemo, element::anemo },
        std::pair{ damage_type::geo, element::geo },
        std::pair{ damage_type::dendro, element::dendro }
    };
    for(const auto [type, expected] : elemental_types)
    {
        CHECK(element_from_damage_type(type) == expected);
    }

    CHECK(element_from_damage_type(damage_type::physical) == element::none);
    CHECK(element_from_damage_type(damage_type::piercing) == element::none);
    CHECK(element_from_damage_type(damage_type::true_damage) == element::none);
}
