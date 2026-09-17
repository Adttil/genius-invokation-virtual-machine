#include <cstddef>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <givm/executor.hpp>

#include "../table/test_definition_library.hpp"

namespace
{
    struct query_counts
    {
        std::uint32_t initial_cost = 0;
        std::uint32_t target_validation = 0;
    };

    struct queried_card_source
    {
        using definition_category = givm::card_definition;

        struct definition_type
        {
            query_counts* counts;
            std::uint8_t cost;
            std::size_t minimum_remaining_cards;
        };

        std::string_view source_name;
        query_counts* counts;
        std::uint8_t base_cost;
        std::size_t minimum_remaining_cards;

        std::string_view name() const noexcept { return source_name; }

        definition_type compile(givm::definition_compile_context&) const
        {
            return { counts, static_cast<std::uint8_t>(base_cost + 1), minimum_remaining_cards };
        }

        static givm::action_cost_requirement query(const definition_type& definition, const givm::card_initial_cost&)
        {
            ++definition.counts->initial_cost;
            return { .dice_requirement = { .any = definition.cost }, .speed = givm::action_speed::fast };
        }

        static givm::target_validation query(const definition_type& definition, const givm::card_target_validation& parameters)
        {
            ++definition.counts->target_validation;
            const auto player = parameters.table[parameters.card.id().player_id];
            return parameters.target_count == 0 && player.deck_card_count() >= definition.minimum_remaining_cards
                ? givm::target_validation::valid_complete : givm::target_validation::invalid;
        }
    };
}

TEST_CASE("empty queries cache each compiled definition and survive library copies and moves", "[definition][query]")
{
    STATIC_REQUIRE(std::is_empty_v<givm::card_initial_cost>);
    STATIC_REQUIRE(std::is_empty_v<givm::character_initial_state>);
    STATIC_REQUIRE_FALSE(std::is_empty_v<givm::card_target_validation>);

    query_counts counts;
    const queried_card_source first{ "FirstQueriedCard", &counts, 1, 0 };
    const queried_card_source second{ "SecondQueriedCard", &counts, 3, 0 };
    givm::definition_source_library sources;
    REQUIRE(sources.add(first, second));
    CHECK(counts.initial_cost == 0);
    const auto [library, ids] = compile(sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    const auto first_id = ids.get_id<givm::card_definition>(first.name());
    const auto second_id = ids.get_id<givm::card_definition>(second.name());
    REQUIRE(counts.initial_cost == 2);
    CHECK(counts.target_validation == 0);

    CHECK(library.query(first_id, givm::card_initial_cost{}).dice_requirement.any == 2);
    CHECK(library[first_id].query(givm::card_initial_cost{}).dice_requirement.any == 2);
    CHECK(library.query(second_id, givm::card_initial_cost{}).dice_requirement.any == 4);

    auto copied = library;
    auto moved = std::move(copied);
    CHECK(moved[first_id].query(givm::card_initial_cost{}).dice_requirement.any == 2);
    CHECK(moved[second_id].query(givm::card_initial_cost{}).dice_requirement.any == 4);
    auto assigned = library;
    assigned = moved;
    CHECK(assigned[first_id].query(givm::card_initial_cost{}).dice_requirement.any == 2);
    assigned = std::move(moved);
    CHECK(assigned[second_id].query(givm::card_initial_cost{}).dice_requirement.any == 4);
    CHECK(counts.initial_cost == 2);
    CHECK(counts.target_validation == 0);
}

TEST_CASE("nonempty queries use current table state and compiled definition data", "[definition][query]")
{
    query_counts counts;
    const queried_card_source source{ "RemainingDeckCard", &counts, 1, 2 };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        givm::compile_mode::normal,
        std::tuple{ givm::draw_cards{ .count = 1 }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, source);
    const auto id = ids.get_id<givm::card_definition>(source.name());
    givm::table table;
    table.load_deck(givm::player_id{ 0 }, { .cards = { id, id, id } });
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor first_draw;
    first_draw.enter_entry(library);
    REQUIRE(first_draw.step(library, table, random) == givm::execution_state::finished);
    const auto card = (*table[givm::player_id{ 0 }].hand_cards().begin()).id();
    CHECK(counts.target_validation == 0);
    CHECK(library.query(id, givm::card_target_validation{ table[card], table, {}, 0 })
        == givm::target_validation::valid_complete);
    CHECK(counts.target_validation == 1);

    givm::executor second_draw;
    second_draw.enter_entry(library);
    REQUIRE(second_draw.step(library, table, random) == givm::execution_state::finished);
    CHECK(library[id].query(givm::card_target_validation{ table[card], table, {}, 0 })
        == givm::target_validation::invalid);
    CHECK(counts.target_validation == 2);
    CHECK(counts.initial_cost == 1);
}

TEST_CASE("missing queries use their operation specific defaults", "[definition][query]")
{
    const givm::test::named_definition_source<givm::card_definition> card_source{ "DefaultQueryCard" };
    const givm::test::named_definition_source<givm::character_view> character_source{ "DefaultQueryCharacter" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        givm::compile_mode::normal,
        std::tuple{ givm::draw_cards{ .count = 1 }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, card_source, character_source);
    const auto card_definition = ids.get_id<givm::card_definition>(card_source.name());
    const auto character_definition = ids.get_id<givm::character_view>(character_source.name());
    const auto state = library[character_definition].query(givm::character_initial_state{});
    CHECK(state.max_health == 0);
    CHECK(state.max_energy == 0);
    CHECK(state.health == 0);
    CHECK(state.energy == 0);
    CHECK(state.aura == givm::element_aura::none);
    const auto cost = library[card_definition].query(givm::card_initial_cost{});
    CHECK(cost.dice_requirement.fixed.total() == 0);
    CHECK(cost.dice_requirement.same == 0);
    CHECK(cost.dice_requirement.any == 0);
    CHECK(cost.speed == givm::action_speed::fast);

    givm::table table;
    table.load_deck(givm::player_id{ 0 }, { .cards = { card_definition } });
    givm::executor draw;
    draw.enter_entry(library);
    auto random = []() -> std::uint32_t { return 0; };
    REQUIRE(draw.step(library, table, random) == givm::execution_state::finished);
    const auto card = *table[givm::player_id{ 0 }].hand_cards().begin();
    CHECK(library.query(card_definition, givm::card_target_validation{ card, table, {}, 0 })
        == givm::target_validation::valid_complete);
    CHECK(library.query(card_definition, givm::card_target_validation{ card, table, {}, 1 })
        == givm::target_validation::invalid);
    CHECK(library.query(card_definition, givm::card_target_validation{ card, table, {}, 2 })
        == givm::target_validation::invalid);
}
