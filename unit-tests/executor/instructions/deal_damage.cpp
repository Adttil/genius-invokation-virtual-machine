#include <cstdint>
#include <limits>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"
#include "../test_character_source.hpp"

using namespace givm;

namespace
{
    enum class observed_event
    {
        calculation,
        effect,
        reaction_will_occur,
        after_reaction,
        after_damage
    };

    struct damage_log
    {
        std::vector<observed_event> order;
        std::uint32_t calculation_bonus = 0;
        std::uint16_t multiplier_numerator = 1;
        std::uint16_t multiplier_denominator = 1;
        std::uint32_t effect_reduction = 0;
        std::uint32_t after_damage_value = 0;
        elemental_reaction reaction = elemental_reaction::none;
        element_application_cause reaction_cause = element_application_cause::effect;
    };

    struct damage_observer_source
    {
        using definition_category = character_view;

        struct definition_type
        {
            damage_log* log;
        };

        damage_log* log;

        constexpr std::string_view name() const noexcept
        {
            return "DamageObserver";
        }

        constexpr definition_type compile(definition_compile_context&) const noexcept
        {
            return { log };
        }

        static handler_program_entry_t<character_initialization> handle(
            const definition_type&, const character_view&, character_initialization& event,
            const card_table&, random_fn&)
        {
            event.state = { .max_health = 10, .health = 10 };
            return handler_program_entry_t<character_initialization>::null();
        }

        static program_entry<damage_calculation> handle(
            const definition_type& data,
            const character_view&,
            damage_calculation& event,
            const card_table&,
            random_fn&
        )
        {
            data.log->order.push_back(observed_event::calculation);
            event.value += data.log->calculation_bonus;
            event.multiplier_numerator = data.log->multiplier_numerator;
            event.multiplier_denominator = data.log->multiplier_denominator;
            return program_entry<damage_calculation>::null();
        }

        static program_entry<damage_effect> handle(
            const definition_type& data,
            const character_view&,
            damage_effect& event,
            const card_table&,
            random_fn&
        )
        {
            data.log->order.push_back(observed_event::effect);
            event.value = event.value < data.log->effect_reduction
                ? 0
                : event.value - data.log->effect_reduction;
            return program_entry<damage_effect>::null();
        }

        static program_entry<elemental_reaction_will_occur> handle(
            const definition_type& data,
            const character_view&,
            elemental_reaction_will_occur& event,
            const card_table&,
            random_fn&
        )
        {
            data.log->order.push_back(observed_event::reaction_will_occur);
            data.log->reaction = event.reaction;
            data.log->reaction_cause = event.cause;
            return program_entry<elemental_reaction_will_occur>::null();
        }

        static program_entry<after_elemental_reaction> handle(
            const definition_type& data,
            const character_view&,
            after_elemental_reaction&,
            const card_table&,
            random_fn&
        )
        {
            data.log->order.push_back(observed_event::after_reaction);
            return program_entry<after_elemental_reaction>::null();
        }

        static program_entry<after_damage> handle(
            const definition_type& data,
            const character_view&,
            after_damage& event,
            const card_table&,
            random_fn&
        )
        {
            data.log->order.push_back(observed_event::after_damage);
            data.log->after_damage_value = event.value;
            return program_entry<after_damage>::null();
        }
    };

    struct zero_random
    {
        std::uint32_t operator()() const noexcept
        {
            return 0;
        }
    };

}

TEST_CASE("deal_damage settles handler adjustments, reactions and saturation", "[deal_damage]")
{
    damage_log log;
    std::uint32_t value = 4;
    damage_type type = damage_type::physical;
    character_state initial{ .max_health = 20, .health = 20 };
    std::uint32_t expected_damage = 4;
    SECTION("calculation precedes multiplier and effect reduction")
    {
        log.calculation_bonus = 1;
        log.multiplier_numerator = 3;
        log.multiplier_denominator = 2;
        log.effect_reduction = 3;
    }
    SECTION("elemental reaction adds damage")
    {
        value = 3;
        type = damage_type::pyro;
        initial.aura = element_aura::cryo;
        expected_damage = 5;
    }
    SECTION("overkill clamps health")
    {
        value = 100;
        expected_damage = 100;
    }
    SECTION("reaction and multiplier saturate")
    {
        value = std::numeric_limits<std::uint32_t>::max() - 1;
        type = damage_type::pyro;
        initial = { .max_health = std::numeric_limits<std::uint32_t>::max(),
                    .health = std::numeric_limits<std::uint32_t>::max(), .aura = element_aura::cryo };
        log.multiplier_numerator = 2;
        expected_damage = std::numeric_limits<std::uint32_t>::max();
    }
    const damage_observer_source observer{ &log };
    const test::initialized_character_source victim{ "Victim", initial };
    constexpr character_id source{ player_id{ 0 }, 0 };
    constexpr character_id damaged{ player_id{ 1 }, 0 };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{
            initialize_characters{ player_id{ 0 } }, initialize_characters{ player_id{ 1 } },
            deal_damage{ .source = source, .target = damaged, .value = value, .type = type },
            end_game{ .result = game_result::both_loss }
        }, std::tuple{}, observer, victim
    );
    card_table table;
    table.load_deck(player_id{ 0 }, { .characters = { ids.get_id<character_view>(observer.name()) } });
    const auto victim_id = ids.get_id<character_view>(victim.name());
    table.load_deck(player_id{ 1 }, { .characters = { victim_id, victim_id } });
    executor target;
    target.enter_entry(library);
    zero_random random;
    REQUIRE(target.run(library, table, random) == execution_state::finished);
    CHECK(target.view_in<execution_state::finished>().result() == game_result::both_loss);
    CHECK(table[damaged].state().health == (expected_damage >= initial.health ? 0 : initial.health - expected_damage));
    CHECK(log.after_damage_value == expected_damage);
    CHECK(table[damaged].state().aura == element_aura::none);
    if(type == damage_type::pyro)
    {
        CHECK(log.reaction == elemental_reaction::melt);
        CHECK(log.reaction_cause == element_application_cause::damage);
        CHECK(log.order == std::vector{ observed_event::calculation, observed_event::effect,
            observed_event::reaction_will_occur, observed_event::after_reaction, observed_event::after_damage });
    }
    else
    {
        CHECK(log.order == std::vector{ observed_event::calculation, observed_event::effect, observed_event::after_damage });
    }
}

TEST_CASE("damage observation precedes elemental settlement and copies resume independently", "[deal_damage][observation]")
{
    const auto initial_aura = GENERATE(element_aura::none, element_aura::cryo);
    damage_log log{ .effect_reduction = 1 };
    const damage_observer_source observer{ &log };
    const test::initialized_character_source victim{
        "Victim", { .max_health = 10, .health = 10, .aura = initial_aura }
    };
    constexpr character_id source{ player_id{ 0 }, 0 };
    constexpr character_id damaged{ player_id{ 1 }, 0 };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{
            initialize_characters{ player_id{ 0 } }, initialize_characters{ player_id{ 1 } },
            deal_damage{ .source = source, .target = damaged, .value = 3, .type = damage_type::pyro,
                .flags = damage_flag_bits::skill_damage },
            end_game{ .result = game_result::both_loss }
        }, std::tuple{}, observer, victim
    );
    card_table table;
    table.load_deck(player_id{ 0 }, { .characters = { ids.get_id<character_view>(observer.name()) } });
    table.load_deck(player_id{ 1 }, { .characters = { ids.get_id<character_view>(victim.name()) } });
    auto normal_table = table;
    executor normal;
    normal.enter_entry(library);
    zero_random random;
    REQUIRE(normal.run(library, normal_table, random) == execution_state::finished);
    const auto normal_order = log.order;
    log.order.clear();

    executor observed;
    observed.enter_entry(library);
    REQUIRE(observed.step(library, table, random) == execution_state::health_reduced);
    const auto health = observed.view_in<execution_state::health_reduced>();
    const auto expected_damage = initial_aura == element_aura::cryo ? 4u : 2u;
    CHECK(health.source() == damage_source_id{ source });
    CHECK(health.target() == damaged);
    CHECK(health.value() == expected_damage);
    CHECK(health.type() == damage_type::pyro);
    CHECK(health.flags().contains(damage_flag_bits::skill_damage));
    CHECK(table[damaged].state().health == 10 - expected_damage);
    CHECK(table[damaged].state().aura == initial_aura);
    CHECK(log.order == std::vector{ observed_event::calculation, observed_event::effect });
    auto copy = observed;
    auto copied_table = table;

    REQUIRE(observed.step(library, table, random) == execution_state::finished);
    CHECK(log.order == normal_order);
    CHECK(table[damaged].state().health == normal_table[damaged].state().health);
    CHECK(table[damaged].state().aura == normal_table[damaged].state().aura);
    CHECK(copied_table[damaged].state().aura == initial_aura);
    log.order = { observed_event::calculation, observed_event::effect };
    REQUIRE(copy.step(library, copied_table, random) == execution_state::finished);
    CHECK(log.order == normal_order);
    CHECK(copied_table[damaged].state().health == table[damaged].state().health);
    CHECK(copied_table[damaged].state().aura == table[damaged].state().aura);
}

TEST_CASE("lethal damage reports overkill and ends the game before later instructions", "[deal_damage][game-result]")
{
    const bool observed = GENERATE(false, true);
    const auto damaged_player = GENERATE(player_id{ 0 }, player_id{ 1 });
    const auto source_health = GENERATE(0u, 10u);
    const test::initialized_character_source attacker{ "Attacker", { .max_health = 10, .health = source_health } };
    const test::initialized_character_source victim{ "Victim", { .max_health = 10, .health = 1 } };
    const character_id source{ other_player(damaged_player), 0 };
    const character_id damaged{ damaged_player, 0 };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{
            initialize_characters{ player_id{ 0 } }, initialize_characters{ player_id{ 1 } },
            deal_damage{ .source = source, .target = damaged, .value = 999, .type = damage_type::physical },
            start_round{}, end_game{ .result = game_result::both_loss }
        }, std::tuple{}, attacker, victim
    );
    card_table table;
    table.load_deck(source.player_id, { .characters = { ids.get_id<character_view>(attacker.name()) } });
    table.load_deck(damaged.player_id, { .characters = { ids.get_id<character_view>(victim.name()) } });
    executor target;
    target.enter_entry(library);
    zero_random random;
    auto state = observed ? target.step(library, table, random) : target.run(library, table, random);
    if(observed)
    {
        REQUIRE(state == execution_state::health_reduced);
        CHECK(target.view_in<execution_state::health_reduced>().value() == 999);
        CHECK(table[damaged].state().health == 0);
        state = target.step(library, table, random);
    }
    REQUIRE(state == execution_state::finished);
    const auto expected = source_health == 0 ? game_result::both_loss
        : damaged_player == player_id{ 0 } ? game_result::player_1_win : game_result::player_0_win;
    CHECK(target.view_in<execution_state::finished>().result() == expected);
    CHECK(table.state().round_number == 0);
}

TEST_CASE("zero damage skips health observation and preserves element application and events", "[deal_damage][observation]")
{
    damage_log log;
    damage_type type = damage_type::physical;
    std::uint32_t value = 0;
    SECTION("physical damage") {}
    SECTION("elemental damage") { type = damage_type::pyro; }
    SECTION("effect reduces damage to zero") { value = 3; log.effect_reduction = 3; }
    const damage_observer_source observer{ &log };
    const test::initialized_character_source victim{ "Victim" };
    constexpr character_id source{ player_id{ 0 }, 0 };
    constexpr character_id damaged{ player_id{ 1 }, 0 };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{
            initialize_characters{ player_id{ 0 } }, initialize_characters{ player_id{ 1 } },
            deal_damage{ .source = source, .target = damaged, .value = value, .type = type },
            end_game{ .result = game_result::both_loss }
        }, std::tuple{}, observer, victim
    );
    card_table table;
    table.load_deck(player_id{ 0 }, { .characters = { ids.get_id<character_view>(observer.name()) } });
    table.load_deck(player_id{ 1 }, { .characters = { ids.get_id<character_view>(victim.name()) } });
    executor target;
    target.enter_entry(library);
    zero_random random;
    REQUIRE(target.step(library, table, random) == execution_state::finished);
    CHECK(table[damaged].state().health == 10);
    CHECK(table[damaged].state().aura == (type == damage_type::pyro ? element_aura::pyro : element_aura::none));
    CHECK(log.order == std::vector{ observed_event::calculation, observed_event::effect, observed_event::after_damage });
    CHECK(log.after_damage_value == 0);
}
