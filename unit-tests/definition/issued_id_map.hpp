#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition/issued_id_map.hpp>

namespace givm_test::definition::issued_id_map
{
TEST_CASE("tag filters treat absent required and excluded tags differently", "[issued_id_map]")
{
    givm::issued_id_map ids{ "healing", "food" };
    const auto potion = ids.add<givm::definition_category::card>("Potion", { "healing" });
    const auto meal = ids.add<givm::definition_category::card>("Meal", { "healing", "food" });
    const auto other = ids.add<givm::definition_category::card>("Other", {});
    ids.add<givm::definition_category::summon>("Healing summon", { "healing" });

    CHECK(ids.query_by_tag<givm::definition_category::card>("unknown").empty());
    CHECK(ids.query_by_tag<givm::definition_category::card>("healing & unknown & !food").empty());
    CHECK((ids.query_by_tag<givm::definition_category::card>("!unknown") == std::vector{ potion, meal, other }));
    CHECK((ids.query_by_tag<givm::definition_category::card>("!unknown & healing") == std::vector{ potion, meal }));
    CHECK((ids.query_by_tag<givm::definition_category::card>(" healing & ! unknown & !food ") == std::vector{ potion }));
    CHECK((ids.query_by_tag<givm::definition_category::card>("!unknown & !other_unknown & !healing") == std::vector{ other }));
    CHECK_FALSE(ids.has_tag("unknown"));
    CHECK(ids.tag_names().size() == 2);
}

TEST_CASE("absent-tag filters work when no tags were registered", "[issued_id_map]")
{
    givm::issued_id_map ids{};
    const auto card = ids.add<givm::definition_category::card>("Untagged", {});

    CHECK_FALSE(ids.has_tag("unknown"));
    CHECK(ids.query_by_tag<givm::definition_category::card>("unknown").empty());
    CHECK((ids.query_by_tag<givm::definition_category::card>("!unknown") == std::vector{ card }));
    CHECK(ids.query_by_tag<givm::definition_category::summon>("!unknown").empty());
}
}
