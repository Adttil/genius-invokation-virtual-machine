#include <array>
#include <concepts>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition.hpp>
#include <givm/executor.hpp>

TEST_CASE("typed instruction sequences enforce their public context", "[definition][instruction]")
{
    using damage_instruction = givm::any_instruction_for<givm::damage_effect>;
    using round_instruction = givm::any_instruction_for<givm::round_ended>;
    using context_free_instruction = givm::any_instruction_for<void>;
    using onpay_instruction = givm::any_instruction_for<givm::onpay_context<givm::cost_of_switch>>;

    STATIC_REQUIRE(givm::instruction_compatible_with<givm::draw_cards, givm::damage_effect>);
    STATIC_REQUIRE(givm::instruction_compatible_with<givm::draw_cards, void>);
    STATIC_REQUIRE(not givm::instruction_compatible_with<int, givm::damage_effect>);
    STATIC_REQUIRE(not givm::instruction_compatible_with<int, void>);
    STATIC_REQUIRE(givm::instruction_compatible_with<givm::absorb_damage_by_count, givm::damage_effect>);
    STATIC_REQUIRE(not givm::instruction_compatible_with<givm::absorb_damage_by_count, givm::round_ended>);
    STATIC_REQUIRE(not givm::instruction_compatible_with<givm::absorb_damage_by_count, void>);
    STATIC_REQUIRE(givm::instruction_compatible_with<damage_instruction, givm::damage_effect>);
    STATIC_REQUIRE(not givm::instruction_compatible_with<damage_instruction, givm::round_ended>);

    STATIC_REQUIRE(std::constructible_from<damage_instruction, givm::draw_cards>);
    STATIC_REQUIRE(std::constructible_from<damage_instruction, givm::absorb_damage_by_count>);
    STATIC_REQUIRE(not std::constructible_from<round_instruction, givm::absorb_damage_by_count>);
    STATIC_REQUIRE(std::constructible_from<context_free_instruction, givm::draw_cards>);
    STATIC_REQUIRE(std::constructible_from<damage_instruction, context_free_instruction>);
    STATIC_REQUIRE(not std::constructible_from<context_free_instruction, givm::absorb_damage_by_count>);
    STATIC_REQUIRE(not std::constructible_from<damage_instruction, int>);
    STATIC_REQUIRE(std::constructible_from<onpay_instruction, givm::draw_cards>);
    STATIC_REQUIRE(not std::constructible_from<damage_instruction, round_instruction>);
    STATIC_REQUIRE(std::copy_constructible<damage_instruction>);

    const std::array<damage_instruction, 2> static_program{
        damage_instruction{ givm::draw_cards{ .count = 1 } },
        damage_instruction{ givm::absorb_damage_by_count{ .maximum_count = 2 } }
    };
    CHECK(static_program.size() == 2);

    std::vector<damage_instruction> sequence;
    sequence.emplace_back(givm::draw_cards{ .count = 3 });
    sequence.emplace_back(givm::absorb_damage_by_count{ .maximum_count = 4 });
    CHECK(sequence.size() == 2);
}
