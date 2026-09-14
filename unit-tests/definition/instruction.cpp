#include <array>
#include <concepts>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition.hpp>
#include <givm/executor.hpp>

using namespace givm;

TEST_CASE("typed instruction sequences enforce their public context", "[definition][instruction]")
{
    using damage_instruction = any_instruction_for<damage_effect>;
    using round_instruction = any_instruction_for<round_ended>;
    using context_free_instruction = any_instruction_for<void>;
    using onpay_instruction = any_instruction_for<onpay_context<cost_of_switch>>;

    STATIC_REQUIRE(instruction_compatible_with<draw_cards, damage_effect>);
    STATIC_REQUIRE(instruction_compatible_with<draw_cards, void>);
    STATIC_REQUIRE(not instruction_compatible_with<int, damage_effect>);
    STATIC_REQUIRE(not instruction_compatible_with<int, void>);
    STATIC_REQUIRE(instruction_compatible_with<absorb_damage_by_count, damage_effect>);
    STATIC_REQUIRE(not instruction_compatible_with<absorb_damage_by_count, round_ended>);
    STATIC_REQUIRE(not instruction_compatible_with<absorb_damage_by_count, void>);
    STATIC_REQUIRE(instruction_compatible_with<damage_instruction, damage_effect>);
    STATIC_REQUIRE(not instruction_compatible_with<damage_instruction, round_ended>);

    STATIC_REQUIRE(std::constructible_from<damage_instruction, draw_cards>);
    STATIC_REQUIRE(std::constructible_from<damage_instruction, absorb_damage_by_count>);
    STATIC_REQUIRE(not std::constructible_from<round_instruction, absorb_damage_by_count>);
    STATIC_REQUIRE(std::constructible_from<context_free_instruction, draw_cards>);
    STATIC_REQUIRE(std::constructible_from<damage_instruction, context_free_instruction>);
    STATIC_REQUIRE(not std::constructible_from<context_free_instruction, absorb_damage_by_count>);
    STATIC_REQUIRE(not std::constructible_from<damage_instruction, int>);
    STATIC_REQUIRE(std::constructible_from<onpay_instruction, draw_cards>);
    STATIC_REQUIRE(not std::constructible_from<damage_instruction, round_instruction>);
    STATIC_REQUIRE(std::copy_constructible<damage_instruction>);

    const std::array<damage_instruction, 2> static_program{
        damage_instruction{ draw_cards{ .count = 1 } },
        damage_instruction{ absorb_damage_by_count{ .maximum_count = 2 } }
    };
    CHECK(static_program.size() == 2);

    std::vector<damage_instruction> sequence;
    sequence.emplace_back(draw_cards{ .count = 3 });
    sequence.emplace_back(absorb_damage_by_count{ .maximum_count = 4 });
    CHECK(sequence.size() == 2);
}
