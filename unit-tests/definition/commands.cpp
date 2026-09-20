#include <array>
#include <concepts>
#include <vector>
#include <variant>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition.hpp>

TEST_CASE("commands share runtime sequences", "[definition][command]")
{
    STATIC_REQUIRE(std::constructible_from<givm::any_command, givm::draw_cards>);
    STATIC_REQUIRE(std::constructible_from<givm::any_command, givm::reduce_combat_status_count>);
    STATIC_REQUIRE(not std::constructible_from<givm::any_command, int>);
    STATIC_REQUIRE(std::copy_constructible<givm::any_command>);

    const std::array<givm::any_command, 2> static_program{
        givm::draw_cards{ .count = 1 }, givm::reduce_combat_status_count{}
    };
    CHECK(std::get<givm::draw_cards>(static_program.front()).count == 1);
    CHECK(std::holds_alternative<givm::reduce_combat_status_count>(static_program.back()));

    std::vector<givm::any_command> sequence;
    sequence.emplace_back(givm::draw_cards{ .count = 3 });
    sequence.emplace_back(givm::reduce_combat_status_count{});
    CHECK(std::get<givm::draw_cards>(sequence.front()).count == 3);
    CHECK(std::holds_alternative<givm::reduce_combat_status_count>(sequence.back()));
}
