#include <array>

#include <catch2/catch_test_macros.hpp>

#include <givm/enums/elemental_reaction.hpp>

using namespace givm;

TEST_CASE("elemental reaction pairs are symmetric", "[elemental_reaction]")
{
    struct reaction_case
    {
        element first;
        element second;
        elemental_reaction expected;
    };
    constexpr std::array cases{
        reaction_case{ element::cryo, element::pyro, elemental_reaction::melt },
        reaction_case{ element::hydro, element::pyro, elemental_reaction::vaporize },
        reaction_case{ element::pyro, element::electro, elemental_reaction::overloaded },
        reaction_case{ element::cryo, element::electro, elemental_reaction::superconduct },
        reaction_case{ element::hydro, element::electro, elemental_reaction::electro_charged },
        reaction_case{ element::cryo, element::hydro, elemental_reaction::frozen },
        reaction_case{ element::pyro, element::dendro, elemental_reaction::burning },
        reaction_case{ element::hydro, element::dendro, elemental_reaction::bloom },
        reaction_case{ element::electro, element::dendro, elemental_reaction::quicken }
    };

    for(const auto& [first, second, expected] : cases)
    {
        CHECK(reaction_between(first, second) == expected);
        CHECK(reaction_between(second, first) == expected);
    }

    for(const auto target : { element::cryo, element::hydro, element::pyro, element::electro })
    {
        CHECK(reaction_between(element::anemo, target) == elemental_reaction::swirl);
        CHECK(reaction_between(target, element::anemo) == elemental_reaction::swirl);
        CHECK(reaction_between(element::geo, target) == elemental_reaction::crystallize);
        CHECK(reaction_between(target, element::geo) == elemental_reaction::crystallize);
    }
}

TEST_CASE("non-reactive element pairs remain non-reactive", "[elemental_reaction]")
{
    for(const auto value : {
        element::cryo, element::hydro, element::pyro, element::electro,
        element::anemo, element::geo, element::dendro
    })
    {
        CHECK(reaction_between(value, value) == elemental_reaction::none);
        CHECK(reaction_between(element::none, value) == elemental_reaction::none);
        CHECK(reaction_between(value, element::none) == elemental_reaction::none);
    }

    CHECK(reaction_between(element::anemo, element::dendro) == elemental_reaction::none);
    CHECK(reaction_between(element::geo, element::dendro) == elemental_reaction::none);
    CHECK(reaction_between(element::anemo, element::geo) == elemental_reaction::none);
}

TEST_CASE("ordered Cryo Dendro auras choose the primary reaction element", "[elemental_reaction]")
{
    CHECK(reaction_from_aura(element_aura::cryo_dendro, element::pyro) == elemental_reaction::melt);
    CHECK(reaction_from_aura(element_aura::dendro_cryo, element::pyro) == elemental_reaction::burning);
}
