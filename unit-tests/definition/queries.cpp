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
            bool elemental_tuning_allowed;
        };

        std::string_view source_name;
        query_counts* counts;
        std::uint8_t base_cost;
        std::size_t minimum_remaining_cards;
        bool elemental_tuning_allowed = true;

        std::string_view name() const noexcept { return source_name; }

        definition_type compile(givm::definition_compile_context&) const
        {
            return { counts, static_cast<std::uint8_t>(base_cost + 1), minimum_remaining_cards,
                elemental_tuning_allowed };
        }

        static givm::card_state query(const definition_type& definition, const givm::card_initial_state&)
        {
            ++definition.counts->initial_cost;
            return {
                .cost = { .dice_requirement = { .any = definition.cost }, .speed = givm::action_speed::fast },
                .elemental_tuning_allowed = definition.elemental_tuning_allowed
            };
        }

        static givm::target_validation query(const definition_type& definition, const givm::card_target_validation& parameters)
        {
            ++definition.counts->target_validation;
            const auto player = parameters.table[parameters.card.id().player_id];
            return parameters.target_count == 0 && player.deck_card_count() >= definition.minimum_remaining_cards
                ? givm::target_validation::valid_complete : givm::target_validation::invalid;
        }
    };

    struct modifying_status_source
    {
        using definition_category = givm::status_definition;

        struct definition_type
        {
            std::uint32_t* calls;
            std::uint8_t cost_per_count;
        };

        std::uint32_t* calls;
        std::uint8_t cost_per_count;

        std::string_view name() const noexcept { return "ModifyingStatus"; }

        definition_type compile(givm::definition_compile_context&) const
        {
            return { calls, cost_per_count };
        }

        static void query(const definition_type& definition, const givm::card_state_modification& parameters)
        {
            ++*definition.calls;
            parameters.state.cost.dice_requirement.any +=
                static_cast<std::uint8_t>(definition.cost_per_count * parameters.status.count);
            parameters.state.elemental_tuning_allowed = parameters.status.count == 0;
        }
    };
}

TEST_CASE("empty queries cache each compiled definition and survive library copies and moves", "[definition][query]")
{
    STATIC_REQUIRE(std::is_empty_v<givm::card_initial_state>);
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

    CHECK(library.query(first_id, givm::card_initial_state{}).cost.dice_requirement.any == 2);
    CHECK(library[first_id].query(givm::card_initial_state{}).cost.dice_requirement.any == 2);
    CHECK(library.query(second_id, givm::card_initial_state{}).cost.dice_requirement.any == 4);

    auto copied = library;
    auto moved = std::move(copied);
    CHECK(moved[first_id].query(givm::card_initial_state{}).cost.dice_requirement.any == 2);
    CHECK(moved[second_id].query(givm::card_initial_state{}).cost.dice_requirement.any == 4);
    auto assigned = library;
    assigned = moved;
    CHECK(assigned[first_id].query(givm::card_initial_state{}).cost.dice_requirement.any == 2);
    assigned = std::move(moved);
    CHECK(assigned[second_id].query(givm::card_initial_state{}).cost.dice_requirement.any == 4);
    CHECK(counts.initial_cost == 2);
    CHECK(counts.target_validation == 0);
}

TEST_CASE("card state modifications use status state and survive library copies", "[definition][query]")
{
    const givm::test::named_definition_source<givm::status_definition> default_source{ "DefaultStatus" };
    std::uint32_t calls = 0;
    const modifying_status_source modifying_source{ &calls, 2 };
    const auto [library, ids] = givm::test::compile_definitions(default_source, modifying_source);
    const auto default_id = ids.get_id<givm::status_definition>(default_source.name());
    const auto modifying_id = ids.get_id<givm::status_definition>(modifying_source.name());
    givm::card_state state{
        .cost = { .dice_requirement = { .same = 1, .any = 3 },
            .speed = givm::action_speed::combat, .energy = 2 },
        .elemental_tuning_allowed = false
    };
    const givm::status_state status{ .count = 2 };

    CHECK(calls == 0);
    library[default_id].query(givm::card_state_modification{ state, status });
    CHECK(state.cost.dice_requirement.same == 1);
    CHECK(state.cost.dice_requirement.any == 3);
    CHECK(state.cost.speed == givm::action_speed::combat);
    CHECK(state.cost.energy == 2);
    CHECK_FALSE(state.elemental_tuning_allowed);

    library[modifying_id].query(givm::card_state_modification{ state, status });
    CHECK(state.cost.dice_requirement.any == 7);
    CHECK_FALSE(state.elemental_tuning_allowed);
    CHECK(status.count == 2);

    const auto copied = library;
    const givm::status_state empty_status{ .count = 0 };
    copied[modifying_id].query(givm::card_state_modification{ state, empty_status });
    CHECK(state.cost.dice_requirement.any == 7);
    CHECK(state.elemental_tuning_allowed);
    copied[modifying_id].query(givm::card_state_modification{ state, status });
    CHECK(state.cost.dice_requirement.any == 11);
    CHECK_FALSE(state.elemental_tuning_allowed);
    CHECK(calls == 3);
}

TEST_CASE("deck loading and card insertion use cached initial card states", "[definition][query]")
{
    query_counts counts;
    const queried_card_source first{ "UntunableCard", &counts, 1, 0, false };
    const queried_card_source second{ "TunableCard", &counts, 3, 0 };
    givm::definition_source_library sources;
    REQUIRE(sources.add(first, second));
    const auto prepared_ids = sources.make_issued_id_map();
    const auto first_id = prepared_ids.get_id<givm::card_definition>(first.name());
    const auto second_id = prepared_ids.get_id<givm::card_definition>(second.name());
    const auto [library, ids] = compile(sources, std::tuple{
        givm::insert_deck_card{ .player = givm::player_id{ 0 }, .definition = first_id },
        givm::insert_deck_card{ .player = givm::player_id{ 1 }, .definition = second_id },
        givm::draw_cards{ .count = 1 },
        givm::draw_cards{ .count = 1, .player = givm::relative_player::other },
        givm::end_game{ givm::game_result::both_loss }
    }, std::tuple{}, givm::compile_mode::normal);
    REQUIRE(ids.get_id<givm::card_definition>(first.name()) == first_id);
    REQUIRE(ids.get_id<givm::card_definition>(second.name()) == second_id);
    REQUIRE(counts.initial_cost == 2);

    const auto check_state = [&](const auto card)
    {
        const bool is_first = card.definition_id() == first_id;
        CHECK(card.state().cost.dice_requirement.any == (is_first ? 2 : 4));
        CHECK(card.state().cost.speed == givm::action_speed::fast);
        CHECK(card.state().elemental_tuning_allowed == not is_first);
    };
    givm::table table;
    load_deck(table, library, { .cards = { first_id, second_id } }, { .cards = { second_id, first_id } });
    for(const auto player : table.players())
    {
        REQUIRE(player.deck_card_count() == 2);
        for(const auto card : player.deck_cards()) check_state(card);
    }
    CHECK(counts.initial_cost == 2);

    givm::executor executor;
    executor.enter_entry(library);
    auto random = []() -> std::uint32_t { return 0; };
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    for(const auto player : table.players())
    {
        REQUIRE(player.hand_card_count() == 1);
        REQUIRE(player.deck_card_count() == 2);
        const auto drawn = *player.hand_cards().begin();
        CHECK(drawn.definition_id() == (player.id() == givm::player_id{ 0 } ? first_id : second_id));
        check_state(drawn);
        for(const auto card : player.deck_cards()) check_state(card);
    }
    CHECK(counts.initial_cost == 2);
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
    load_deck(table, library, { .cards = { id, id, id } }, {});
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor first_draw;
    first_draw.enter_entry(library);
    REQUIRE(first_draw.step(library, table, random) == givm::execution_state::finished);
    const auto card = (*table[givm::player_id{ 0 }].hand_cards().begin()).id();
    CHECK(counts.target_validation == 0);
    CHECK(library.query(id, givm::card_target_validation{ table[card], table, library, {}, 0 })
        == givm::target_validation::valid_complete);
    CHECK(counts.target_validation == 1);

    givm::executor second_draw;
    second_draw.enter_entry(library);
    REQUIRE(second_draw.step(library, table, random) == givm::execution_state::finished);
    CHECK(library[id].query(givm::card_target_validation{ table[card], table, library, {}, 0 })
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
    const auto cost = library[card_definition].query(givm::card_initial_state{}).cost;
    CHECK(cost.dice_requirement.fixed.total() == 0);
    CHECK(cost.dice_requirement.same == 0);
    CHECK(cost.dice_requirement.any == 0);
    CHECK(cost.speed == givm::action_speed::fast);

    givm::table table;
    load_deck(table, library, { .cards = { card_definition } }, {});
    givm::executor draw;
    draw.enter_entry(library);
    auto random = []() -> std::uint32_t { return 0; };
    REQUIRE(draw.step(library, table, random) == givm::execution_state::finished);
    const auto card = *table[givm::player_id{ 0 }].hand_cards().begin();
    CHECK(library.query(card_definition, givm::card_target_validation{ card, table, library, {}, 0 })
        == givm::target_validation::valid_complete);
    CHECK(library.query(card_definition, givm::card_target_validation{ card, table, library, {}, 1 })
        == givm::target_validation::invalid);
    CHECK(library.query(card_definition, givm::card_target_validation{ card, table, library, {}, 2 })
        == givm::target_validation::invalid);
}
