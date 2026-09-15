#include <array>
#include <concepts>
#include <vector>
#include <variant>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition.hpp>

TEST_CASE("typed command sequences enforce their public context", "[definition][command]")
{
    using damage_command = givm::any_command_for<givm::damage_effect>;
    using round_command = givm::any_command_for<givm::round_ended>;
    using context_free_command = givm::any_command_for<void>;
    using onpay_command = givm::any_command_for<givm::handler_program_context_t<givm::cost_of_switch>>;

    STATIC_REQUIRE(givm::command_compatible_with<givm::draw_cards, givm::damage_effect>);
    STATIC_REQUIRE(givm::command_compatible_with<givm::draw_cards, void>);
    STATIC_REQUIRE(not givm::command_compatible_with<int, givm::damage_effect>);
    STATIC_REQUIRE(not givm::command_compatible_with<int, void>);
    STATIC_REQUIRE(givm::command_compatible_with<givm::absorb_damage_by_count, givm::damage_effect>);
    STATIC_REQUIRE(not givm::command_compatible_with<givm::absorb_damage_by_count, givm::round_ended>);
    STATIC_REQUIRE(not givm::command_compatible_with<givm::absorb_damage_by_count, void>);
    STATIC_REQUIRE(givm::command_compatible_with<damage_command, givm::damage_effect>);
    STATIC_REQUIRE(not givm::command_compatible_with<damage_command, givm::round_ended>);

    STATIC_REQUIRE(std::constructible_from<damage_command, givm::draw_cards>);
    STATIC_REQUIRE(std::constructible_from<damage_command, givm::absorb_damage_by_count>);
    STATIC_REQUIRE(not std::constructible_from<round_command, givm::absorb_damage_by_count>);
    STATIC_REQUIRE(std::constructible_from<context_free_command, givm::draw_cards>);
    STATIC_REQUIRE(not std::constructible_from<damage_command, context_free_command>);
    STATIC_REQUIRE(not std::constructible_from<context_free_command, givm::absorb_damage_by_count>);
    STATIC_REQUIRE(not std::constructible_from<damage_command, int>);
    STATIC_REQUIRE(std::constructible_from<onpay_command, givm::draw_cards>);
    STATIC_REQUIRE(not std::constructible_from<damage_command, round_command>);
    STATIC_REQUIRE(std::copy_constructible<damage_command>);

    const std::array<damage_command, 2> static_program{
        damage_command{ givm::draw_cards{ .count = 1 } },
        damage_command{ givm::absorb_damage_by_count{ .maximum_count = 2 } }
    };
    CHECK(static_program.size() == 2);

    std::vector<damage_command> sequence;
    sequence.emplace_back(givm::draw_cards{ .count = 3 });
    sequence.emplace_back(givm::absorb_damage_by_count{ .maximum_count = 4 });
    CHECK(sequence.size() == 2);

    const context_free_command shared = givm::draw_cards{ .count = 7 };
    const damage_command widened = std::visit([](const auto& command) -> damage_command
    {
        return command;
    }, shared);
    CHECK(std::get<givm::draw_cards>(widened).count == 7);
}
