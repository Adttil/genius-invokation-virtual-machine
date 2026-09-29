#include "compile_boundary_fixture.hpp"

#include <cstdint>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace givm_test::executor::compile_boundary
{
TEST_CASE("command sequence wrappers compile and invoke across translation units", "[executor][library][linkage]")
{
    const auto form = GENERATE(sequence_form::span, sequence_form::array, sequence_form::vector,
        sequence_form::tuple, sequence_form::list, sequence_form::subset_variant, sequence_form::variadic);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    CAPTURE(form, mode);
    auto compiled = make_program(form, mode);
    if(not compiled)
    {
        INFO(givm::error_string(compiled.error()));
        REQUIRE(compiled.has_value());
    }
    REQUIRE(compiled.has_value());
    const auto& [library, ids] = *compiled;
    const auto card_a = ids.get_id<givm::card_definition>("CompileBoundaryCardA");
    const auto card_b = ids.get_id<givm::card_definition>("CompileBoundaryCardB");
    const givm::character_id character{ givm::player_id{ 0 }, 0 };
    givm::table table{ {}, { .active_character = character } };
    load_deck(table, library, {
        .cards = { card_a, card_b },
        .characters = { ids.get_id<givm::character_view>("CompileBoundaryCharacter") }
    }, {});

    givm::executor execution;
    auto random = [] { return std::uint32_t{ 0 }; };
    auto state = execution.start(library, table).resume(library, table, random);
    if(mode == givm::compile_mode::observed)
    {
        REQUIRE(state == givm::execution_state::round_started);
        CHECK(table.state().round_number == 1);
        CHECK(table[character].state().energy == 0);
        state = execution.view_in<givm::execution_state::round_started>().resume(library, table, random);
    }
    REQUIRE(state == givm::execution_state::finished);
    CHECK(execution.view_in<givm::execution_state::finished>().result() == givm::game_result::player_0_win);
    CHECK(table.state().round_number == 1);
    CHECK(table[character].state().energy == 2);
    const auto player = table[character.player_id];
    CHECK(player.state().dice[givm::elemental_dice::pyro] == 3);
    CHECK(player.deck_card_count() == 0);
    std::vector<givm::definition_id<givm::card_definition>> hand;
    for(const auto card : player.hand_cards()) hand.push_back(card.definition_id());
    CHECK(hand == std::vector{ card_a, card_b });
}
}
