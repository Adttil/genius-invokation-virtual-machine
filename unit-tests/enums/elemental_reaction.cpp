#include <array>

#include <catch2/catch_test_macros.hpp>

#include <givm/enums/elemental_reaction.hpp>

TEST_CASE("reactions between persistent elements are symmetric", "[elemental_reaction]")
{
    struct reaction_case
    {
        givm::element first;
        givm::element second;
        givm::elemental_reaction expected;
    };
    constexpr std::array cases{
        reaction_case{ givm::element::cryo, givm::element::pyro, givm::elemental_reaction::melt },
        reaction_case{ givm::element::hydro, givm::element::pyro, givm::elemental_reaction::vaporize },
        reaction_case{ givm::element::pyro, givm::element::electro, givm::elemental_reaction::overloaded },
        reaction_case{ givm::element::cryo, givm::element::electro, givm::elemental_reaction::superconduct },
        reaction_case{ givm::element::hydro, givm::element::electro, givm::elemental_reaction::electro_charged },
        reaction_case{ givm::element::cryo, givm::element::hydro, givm::elemental_reaction::frozen },
        reaction_case{ givm::element::pyro, givm::element::dendro, givm::elemental_reaction::burning },
        reaction_case{ givm::element::hydro, givm::element::dendro, givm::elemental_reaction::bloom },
        reaction_case{ givm::element::electro, givm::element::dendro, givm::elemental_reaction::quicken }
    };

    for(const auto& [first, second, expected] : cases)
    {
        CHECK(reaction_between(first, second) == expected);
        CHECK(reaction_between(second, first) == expected);
    }

}

TEST_CASE("swirl and crystallize require incoming Anemo and Geo", "[elemental_reaction]")
{
    for(const auto target : { givm::element::cryo, givm::element::hydro, givm::element::pyro, givm::element::electro })
    {
        CHECK(reaction_between(givm::element::anemo, target) == givm::elemental_reaction::none);
        CHECK(reaction_between(target, givm::element::anemo) == givm::elemental_reaction::swirl);
        CHECK(reaction_between(givm::element::geo, target) == givm::elemental_reaction::none);
        CHECK(reaction_between(target, givm::element::geo) == givm::elemental_reaction::crystallize);
    }
}

TEST_CASE("non-reactive element pairs remain non-reactive", "[elemental_reaction]")
{
    for(const auto value : {
        givm::element::cryo, givm::element::hydro, givm::element::pyro, givm::element::electro,
        givm::element::anemo, givm::element::geo, givm::element::dendro
    })
    {
        CHECK(reaction_between(value, value) == givm::elemental_reaction::none);
        CHECK(reaction_between(givm::element::none, value) == givm::elemental_reaction::none);
        CHECK(reaction_between(value, givm::element::none) == givm::elemental_reaction::none);
    }

    CHECK(reaction_between(givm::element::anemo, givm::element::dendro) == givm::elemental_reaction::none);
    CHECK(reaction_between(givm::element::geo, givm::element::dendro) == givm::elemental_reaction::none);
    CHECK(reaction_between(givm::element::anemo, givm::element::geo) == givm::elemental_reaction::none);
}

TEST_CASE("ordered Cryo Dendro auras choose the primary reaction element", "[elemental_reaction]")
{
    CHECK(reaction_from_aura(givm::element_aura::cryo_dendro, givm::element::pyro) == givm::elemental_reaction::melt);
    CHECK(reaction_from_aura(givm::element_aura::dendro_cryo, givm::element::pyro) == givm::elemental_reaction::burning);
}
