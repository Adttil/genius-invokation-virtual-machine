#include <catch2/catch_test_macros.hpp>

#include <givm/enums/elemental_dice.hpp>

using namespace givm;

TEST_CASE("random values map to eight elemental dice", "[elemental_dice]")
{
    CHECK(elemental_dice_from_random(0) == elemental_dice::omni);
    CHECK(elemental_dice_from_random(1) == elemental_dice::cryo);
    CHECK(elemental_dice_from_random(2) == elemental_dice::hydro);
    CHECK(elemental_dice_from_random(3) == elemental_dice::pyro);
    CHECK(elemental_dice_from_random(4) == elemental_dice::electro);
    CHECK(elemental_dice_from_random(5) == elemental_dice::anemo);
    CHECK(elemental_dice_from_random(6) == elemental_dice::geo);
    CHECK(elemental_dice_from_random(7) == elemental_dice::dendro);
    CHECK(elemental_dice_from_random(15) == elemental_dice::dendro);
}

TEST_CASE("dice_counts supports containment and component arithmetic", "[elemental_dice]")
{
    dice_counts available;
    available[elemental_dice::omni] = 2;
    available[elemental_dice::pyro] = 3;

    dice_counts payment;
    payment[elemental_dice::omni] = 1;
    payment[elemental_dice::pyro] = 2;

    CHECK(available.total() == 5);
    CHECK(available.contains(payment));
    CHECK_FALSE(payment.contains(available));

    available -= payment;
    CHECK(available[elemental_dice::omni] == 1);
    CHECK(available[elemental_dice::pyro] == 1);
    CHECK(available.total() == 2);

    available += payment;
    CHECK(available[elemental_dice::omni] == 2);
    CHECK(available[elemental_dice::pyro] == 3);
}

TEST_CASE("elemental_dice_cost arithmetic is component-wise", "[elemental_dice]")
{
    const auto base = elemental_dice::pyro * static_cast<unsigned char>(2)
        + elemental_dice::hydro * static_cast<unsigned char>(4);
    const auto adjustment = elemental_dice::pyro * static_cast<unsigned char>(1)
        + elemental_dice::hydro * static_cast<unsigned char>(2);

    const auto increased = base + adjustment;
    CHECK(increased[elemental_dice::pyro] == 3);
    CHECK(increased[elemental_dice::hydro] == 6);

    const auto restored = increased - adjustment;
    CHECK(restored == base);
    const auto doubled = base * static_cast<unsigned char>(2);
    const auto halved = base / static_cast<unsigned char>(2);
    CHECK(doubled[elemental_dice::pyro] == 4);
    CHECK(halved[elemental_dice::hydro] == 2);
}
