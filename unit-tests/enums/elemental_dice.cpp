#include <catch2/catch_test_macros.hpp>

#include <givm/enums/elemental_dice.hpp>

TEST_CASE("random values map to eight elemental dice", "[elemental_dice]")
{
    CHECK(givm::elemental_dice_from_random(0) == givm::elemental_dice::omni);
    CHECK(givm::elemental_dice_from_random(1) == givm::elemental_dice::cryo);
    CHECK(givm::elemental_dice_from_random(2) == givm::elemental_dice::hydro);
    CHECK(givm::elemental_dice_from_random(3) == givm::elemental_dice::pyro);
    CHECK(givm::elemental_dice_from_random(4) == givm::elemental_dice::electro);
    CHECK(givm::elemental_dice_from_random(5) == givm::elemental_dice::anemo);
    CHECK(givm::elemental_dice_from_random(6) == givm::elemental_dice::geo);
    CHECK(givm::elemental_dice_from_random(7) == givm::elemental_dice::dendro);
    CHECK(givm::elemental_dice_from_random(15) == givm::elemental_dice::dendro);
}

TEST_CASE("dice_counts supports containment and component arithmetic", "[elemental_dice]")
{
    givm::dice_counts available;
    available[givm::elemental_dice::omni] = 2;
    available[givm::elemental_dice::pyro] = 3;

    givm::dice_counts payment;
    payment[givm::elemental_dice::omni] = 1;
    payment[givm::elemental_dice::pyro] = 2;

    CHECK(available.total() == 5);
    CHECK(available.contains(payment));
    CHECK_FALSE(payment.contains(available));

    available -= payment;
    CHECK(available[givm::elemental_dice::omni] == 1);
    CHECK(available[givm::elemental_dice::pyro] == 1);
    CHECK(available.total() == 2);

    available += payment;
    CHECK(available[givm::elemental_dice::omni] == 2);
    CHECK(available[givm::elemental_dice::pyro] == 3);
}

TEST_CASE("elemental_dice_cost arithmetic is component-wise", "[elemental_dice]")
{
    const auto base = givm::elemental_dice::pyro * static_cast<unsigned char>(2)
        + givm::elemental_dice::hydro * static_cast<unsigned char>(4);
    const auto adjustment = givm::elemental_dice::pyro * static_cast<unsigned char>(1)
        + givm::elemental_dice::hydro * static_cast<unsigned char>(2);

    const auto increased = base + adjustment;
    CHECK(increased[givm::elemental_dice::pyro] == 3);
    CHECK(increased[givm::elemental_dice::hydro] == 6);

    const auto restored = increased - adjustment;
    CHECK(restored == base);
    const auto doubled = base * static_cast<unsigned char>(2);
    const auto halved = base / static_cast<unsigned char>(2);
    CHECK(doubled[givm::elemental_dice::pyro] == 4);
    CHECK(halved[givm::elemental_dice::hydro] == 2);
}
