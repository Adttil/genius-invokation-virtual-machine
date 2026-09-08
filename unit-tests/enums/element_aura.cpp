#include <array>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <givm/enums/element_aura.hpp>

using namespace givm;

TEST_CASE("elements and auras map in both directions", "[element_aura]")
{
    constexpr std::array mappings{
        std::pair{ element::none, element_aura::none },
        std::pair{ element::cryo, element_aura::cryo },
        std::pair{ element::hydro, element_aura::hydro },
        std::pair{ element::pyro, element_aura::pyro },
        std::pair{ element::electro, element_aura::electro },
        std::pair{ element::anemo, element_aura::anemo },
        std::pair{ element::geo, element_aura::geo },
        std::pair{ element::dendro, element_aura::dendro }
    };

    for(const auto [value, aura] : mappings)
    {
        CHECK(aura_from_element(value) == aura);
        CHECK(primary_element_from_aura(aura) == value);
    }

    CHECK(primary_element_from_aura(element_aura::cryo_dendro) == element::cryo);
    CHECK(primary_element_from_aura(element_aura::dendro_cryo) == element::dendro);
}

TEST_CASE("non-reactive auras preserve their element order", "[element_aura]")
{
    CHECK(aura_without_reaction(element_aura::cryo, element::dendro) == element_aura::cryo_dendro);
    CHECK(aura_without_reaction(element_aura::dendro, element::cryo) == element_aura::dendro_cryo);
    CHECK(aura_without_reaction(element_aura::none, element::hydro) == element_aura::hydro);
    CHECK(aura_without_reaction(element_aura::hydro, element::hydro) == element_aura::hydro);
}
