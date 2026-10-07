#include "source_library_provider.hpp"

#include <string>
#include <utility>

#include <catch2/catch_test_macros.hpp>

namespace givm_test::definition::source_library_linkage
{
TEST_CASE("source library factories are usable through the source library interface alone", "[source_library][linkage]")
{
    givm::definition_source_library empty;
    CHECK(empty.empty());
    REQUIRE(empty.add());
    CHECK(empty.empty());

    const auto original = make_source_closure();
    CHECK_FALSE(original.empty());
    CHECK(original.source_views<givm::definition_category::card>().size() == 2);
    CHECK(original.has<givm::definition_category::card>("SharedCard"));
    CHECK(std::string{ original.get<givm::definition_category::card>("SharedCard").name() } == "SharedCard");
    const auto tags = original.get<givm::definition_category::card>("SharedCard").tags();
    REQUIRE(tags.size() == 1);
    CHECK(std::string{ tags.front() } == "linkage_fixture");

    auto copied = original;
    CHECK_FALSE(copied.empty());
    REQUIRE(copied.add(make_overlapping_closure()));
    CHECK(copied.source_views<givm::definition_category::card>().size() == 3);
    CHECK(copied.has<givm::definition_category::card>("PeerCard"));
    CHECK_FALSE(original.has<givm::definition_category::card>("PeerCard"));

    auto moved = std::move(copied);
    givm::definition_source_library assigned;
    assigned = moved;
    givm::definition_source_library transferred;
    transferred = std::move(assigned);
    CHECK_FALSE(transferred.empty());
    REQUIRE(transferred.add(original));
    REQUIRE(transferred.add(transferred));
    CHECK(transferred.source_views<givm::definition_category::card>().size() == 3);
    CHECK(std::string{ transferred.get<givm::definition_category::card>("ProviderCard").name() } == "ProviderCard");

    const auto result = transferred.add(make_conflicting_closure());
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().size() == 1);
    const auto& error = result.error().front();
    CHECK(error.cause == givm::source_conflict::reason::different_type);
    CHECK(error.definition.category == givm::definition_category::card);
    CHECK(error.definition.name == "SharedCard");
    CHECK_FALSE(error.first_input_index);
    CHECK_FALSE(error.second_input_index);
    const auto message = givm::error_string(result.error());
    CHECK(message == givm::error_string(error));
    CHECK(message.find("different_type") != std::string::npos);
    CHECK(message.find("SharedCard") != std::string::npos);
    CHECK_FALSE(transferred.has<givm::definition_category::card>("RejectedCard"));
    CHECK(transferred.source_views<givm::definition_category::card>().size() == 3);
}
}
