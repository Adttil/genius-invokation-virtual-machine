#include <array>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <givm/enums/element_aura.hpp>

namespace givm_test::enums::element_aura
{
TEST_CASE("persistent elements and auras map in both directions", "[element_aura]")
{
    constexpr std::array mappings{
        std::pair{ givm::element::none, givm::element_aura::none },
        std::pair{ givm::element::cryo, givm::element_aura::cryo },
        std::pair{ givm::element::hydro, givm::element_aura::hydro },
        std::pair{ givm::element::pyro, givm::element_aura::pyro },
        std::pair{ givm::element::electro, givm::element_aura::electro },
        std::pair{ givm::element::dendro, givm::element_aura::dendro }
    };

    for(const auto [value, aura] : mappings)
    {
        CHECK(aura_from_element(value) == aura);
        CHECK(primary_element_from_aura(aura) == value);
    }

    CHECK(primary_element_from_aura(givm::element_aura::cryo_dendro) == givm::element::cryo);
}

TEST_CASE("Anemo and Geo do not leave or replace non-reacting auras", "[element_aura]")
{
    for(const auto incoming : { givm::element::anemo, givm::element::geo })
    {
        CHECK(aura_from_element(incoming) == givm::element_aura::none);
        for(const auto aura : {
            givm::element_aura::none, givm::element_aura::cryo, givm::element_aura::hydro,
            givm::element_aura::pyro, givm::element_aura::electro, givm::element_aura::dendro,
            givm::element_aura::cryo_dendro
        })
            CHECK(aura_without_reaction(aura, incoming) == aura);
    }
}

TEST_CASE("Cryo Dendro coexistence has a single order and survives repeated application", "[element_aura]")
{
    CHECK(aura_without_reaction(givm::element_aura::cryo, givm::element::dendro) == givm::element_aura::cryo_dendro);
    CHECK(aura_without_reaction(givm::element_aura::dendro, givm::element::cryo) == givm::element_aura::cryo_dendro);
    CHECK(aura_without_reaction(givm::element_aura::cryo_dendro, givm::element::cryo) == givm::element_aura::cryo_dendro);
    CHECK(aura_without_reaction(givm::element_aura::cryo_dendro, givm::element::dendro) == givm::element_aura::cryo_dendro);
    CHECK(aura_without_reaction(givm::element_aura::none, givm::element::hydro) == givm::element_aura::hydro);
    CHECK(aura_without_reaction(givm::element_aura::hydro, givm::element::hydro) == givm::element_aura::hydro);
}
}
