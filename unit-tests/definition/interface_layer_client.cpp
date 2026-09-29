#include "interface_layer_runtime.hpp"

#include <cstdint>
#include <ranges>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace givm_test::interface_layers
{
TEST_CASE("definition authoring compilation and runtime interfaces link independently", "[interface_layers][linkage]")
{
    const bool observed = GENERATE(false, true);
    givm::linked_deck deck;
    const auto library = make_layered_library(deck, observed);
    REQUIRE(deck.characters.size() == 1);
    const givm::character_id character{ givm::player_id{ 0 }, 0 };
    givm::table table{ {}, { .active_character = character } };
    load_deck(table, library, deck, {});
    REQUIRE(std::ranges::distance(table[character].skills()) == 1);
    const auto skill = *table[character].skills().begin();
    CHECK(library[skill.definition_id()].query(givm::skill_initial_cost{}).energy == 1);
    CHECK(library[skill.definition_id()].query(givm::skill_target_validation{
        .skill = skill, .table = table, .library = library }) == givm::target_validation::invalid);

    givm::executor execution;
    auto random = [] { return std::uint32_t{ 0 }; };
    auto state = execution.start(library, table).resume(library, table, random);
    if(observed)
    {
        REQUIRE(state == givm::execution_state::round_started);
        CHECK(table[character].state().energy == 0);
        state = execution.view_in<givm::execution_state::round_started>().resume(library, table, random);
    }
    REQUIRE(state == givm::execution_state::finished);
    CHECK(execution.view_in<givm::execution_state::finished>().result() == givm::game_result::player_0_win);
    CHECK(table[character].state().energy == 2);
    CHECK(table[character.player_id].state().dice[givm::elemental_dice::cryo] == 2);
    CHECK(library[skill.definition_id()].query(givm::skill_target_validation{
        .skill = skill, .table = table, .library = library }) == givm::target_validation::valid_complete);
}
}
