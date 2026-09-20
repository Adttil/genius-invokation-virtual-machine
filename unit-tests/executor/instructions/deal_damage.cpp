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
        givm::elemental_reaction reaction = givm::elemental_reaction::none;
        givm::element_application_cause reaction_cause = givm::element_application_cause::effect;
    };

    struct damage_observer_source
    {
        using definition_category = givm::character_view;

        struct definition_type
        {
            damage_log* log;
        };

        damage_log* log;

        constexpr std::string_view name() const noexcept
        {
            return "DamageObserver";
        }

        constexpr definition_type compile(givm::definition_compile_context&) const noexcept
        {
            return { log };
        }

        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }

        static givm::program_entry handle(
            const definition_type& data,
            const givm::character_view&,
            givm::damage_calculation& event,
            givm::handle_context&)
        {
            data.log->order.push_back(observed_event::calculation);
            event.value += data.log->calculation_bonus;
            event.multiplier_numerator = data.log->multiplier_numerator;
            event.multiplier_denominator = data.log->multiplier_denominator;
            return {};
        }

        static givm::program_entry handle(
            const definition_type& data,
            const givm::character_view&,
            givm::damage_effect& event,
            givm::handle_context&)
        {
            data.log->order.push_back(observed_event::effect);
            event.value = event.value < data.log->effect_reduction
                ? 0
                : event.value - data.log->effect_reduction;
            return {};
        }

        static givm::program_entry handle(
            const definition_type& data,
            const givm::character_view&,
            givm::elemental_reaction_will_occur& event,
            givm::handle_context&)
        {
            data.log->order.push_back(observed_event::reaction_will_occur);
            data.log->reaction = event.reaction;
            data.log->reaction_cause = event.cause;
            return {};
        }

        static givm::program_entry handle(
            const definition_type& data,
            const givm::character_view&,
            givm::after_elemental_reaction&,
            givm::handle_context&)
        {
            data.log->order.push_back(observed_event::after_reaction);
            return {};
        }

        static givm::program_entry handle(
            const definition_type& data,
            const givm::character_view&,
            givm::after_damage& event,
            givm::handle_context&)
        {
            data.log->order.push_back(observed_event::after_damage);
            data.log->after_damage_value = event.value;
            return {};
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
    givm::damage_type type = givm::damage_type::physical;
    givm::character_state initial{ .max_health = 20, .health = 20 };
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
        type = givm::damage_type::pyro;
        initial.aura = givm::element_aura::cryo;
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
        type = givm::damage_type::pyro;
        initial = { .max_health = std::numeric_limits<std::uint32_t>::max(),
                    .health = std::numeric_limits<std::uint32_t>::max(), .aura = givm::element_aura::cryo };
        log.multiplier_numerator = 2;
        expected_damage = std::numeric_limits<std::uint32_t>::max();
    }
    const damage_observer_source observer{ &log };
    const givm::test::initialized_character_source victim{ "Victim", initial };
    constexpr givm::character_id source{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id damaged{ givm::player_id{ 1 }, 0 };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        givm::compile_mode::normal,
        std::tuple{
            givm::deal_damage{ .source = source, .target = damaged, .value = value, .type = type },
            givm::end_game{ .result = givm::game_result::both_loss }
        }, std::tuple{}, observer, victim
    );
    givm::table table;
    const auto victim_id = ids.get_id<givm::character_view>(victim.name());
    load_deck(table, library,
        { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { victim_id, victim_id } });
    givm::executor target;
    target.enter_entry(library);
    zero_random random;
    REQUIRE(target.step(library, table, random) == givm::execution_state::finished);
    CHECK(target.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
    CHECK(table[damaged].state().health == (expected_damage >= initial.health ? 0 : initial.health - expected_damage));
    CHECK(log.after_damage_value == expected_damage);
    CHECK(table[damaged].state().aura == givm::element_aura::none);
    if(type == givm::damage_type::pyro)
    {
        CHECK(log.reaction == givm::elemental_reaction::melt);
        CHECK(log.reaction_cause == givm::element_application_cause::damage);
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
    const auto initial_aura = GENERATE(givm::element_aura::none, givm::element_aura::cryo);
    damage_log log{ .effect_reduction = 1 };
    const damage_observer_source observer{ &log };
    const givm::test::initialized_character_source victim{
        "Victim", { .max_health = 10, .health = 10, .aura = initial_aura }
    };
    constexpr givm::character_id source{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id damaged{ givm::player_id{ 1 }, 0 };
    const auto compile_program = [&](givm::compile_mode mode)
    {
        return givm::test::compile_definitions_with_program(
            mode,
            std::tuple{
                givm::deal_damage{ .source = source, .target = damaged, .value = 3, .type = givm::damage_type::pyro,
                    .flags = givm::damage_flag_bits::skill_damage },
                givm::end_game{ .result = givm::game_result::both_loss }
            }, std::tuple{}, observer, victim
        );
    };
    const auto [library, ids] = compile_program(givm::compile_mode::observed);
    const auto normal_compilation = compile_program(givm::compile_mode::normal);
    givm::table table;
    load_deck(table, library,
        { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { ids.get_id<givm::character_view>(victim.name()) } });
    auto normal_table = table;
    givm::executor normal;
    normal.enter_entry(normal_compilation.library);
    zero_random random;
    REQUIRE(normal.step(normal_compilation.library, normal_table, random) == givm::execution_state::finished);
    const auto normal_order = log.order;
    log.order.clear();

    givm::executor observed;
    observed.enter_entry(library);
    REQUIRE(observed.step(library, table, random) == givm::execution_state::health_reduced);
    const auto health = observed.view_in<givm::execution_state::health_reduced>();
    const auto expected_damage = initial_aura == givm::element_aura::cryo ? 4u : 2u;
    CHECK(health.source() == givm::damage_source_id{ source });
    CHECK(health.target() == damaged);
    CHECK(health.value() == expected_damage);
    CHECK(health.type() == givm::damage_type::pyro);
    CHECK(health.flags().contains(givm::damage_flag_bits::skill_damage));
    CHECK(table[damaged].state().health == 10 - expected_damage);
    CHECK(table[damaged].state().aura == initial_aura);
    CHECK(log.order == std::vector{ observed_event::calculation, observed_event::effect });
    auto copy = observed;
    auto copied_table = table;

    REQUIRE(observed.step(library, table, random) == givm::execution_state::finished);
    CHECK(log.order == normal_order);
    CHECK(table[damaged].state().health == normal_table[damaged].state().health);
    CHECK(table[damaged].state().aura == normal_table[damaged].state().aura);
    CHECK(copied_table[damaged].state().aura == initial_aura);
    log.order = { observed_event::calculation, observed_event::effect };
    REQUIRE(copy.step(library, copied_table, random) == givm::execution_state::finished);
    CHECK(log.order == normal_order);
    CHECK(copied_table[damaged].state().health == table[damaged].state().health);
    CHECK(copied_table[damaged].state().aura == table[damaged].state().aura);
}

TEST_CASE("lethal damage reports overkill and ends the game before later instructions", "[deal_damage][game-result]")
{
    const bool observed = GENERATE(false, true);
    const auto damaged_player = GENERATE(givm::player_id{ 0 }, givm::player_id{ 1 });
    const auto source_health = GENERATE(0u, 10u);
    const givm::test::initialized_character_source attacker{ "Attacker", { .max_health = 10, .health = source_health } };
    const givm::test::initialized_character_source victim{ "Victim", { .max_health = 10, .health = 1 } };
    const givm::character_id source{ other_player(damaged_player), 0 };
    const givm::character_id damaged{ damaged_player, 0 };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{
            givm::deal_damage{ .source = source, .target = damaged, .value = 999, .type = givm::damage_type::physical },
            givm::start_round{}, givm::end_game{ .result = givm::game_result::both_loss }
        }, std::tuple{}, attacker, victim
    );
    givm::table table;
    const givm::linked_deck attacking_deck{ .characters = { ids.get_id<givm::character_view>(attacker.name()) } };
    const givm::linked_deck defending_deck{ .characters = { ids.get_id<givm::character_view>(victim.name()) } };
    load_deck(table, library,
        damaged_player == givm::player_id{ 0 } ? defending_deck : attacking_deck,
        damaged_player == givm::player_id{ 1 } ? defending_deck : attacking_deck);
    givm::executor target;
    target.enter_entry(library);
    zero_random random;
    auto state = target.step(library, table, random);
    if(observed)
    {
        REQUIRE(state == givm::execution_state::health_reduced);
        CHECK(target.view_in<givm::execution_state::health_reduced>().value() == 999);
        CHECK(table[damaged].state().health == 0);
        state = target.step(library, table, random);
    }
    REQUIRE(state == givm::execution_state::finished);
    const auto expected = source_health == 0 ? givm::game_result::both_loss
        : damaged_player == givm::player_id{ 0 } ? givm::game_result::player_1_win : givm::game_result::player_0_win;
    CHECK(target.view_in<givm::execution_state::finished>().result() == expected);
    CHECK(table.state().round_number == 0);
}

TEST_CASE("zero damage skips health observation and preserves element application and events", "[deal_damage][observation]")
{
    damage_log log;
    givm::damage_type type = givm::damage_type::physical;
    std::uint32_t value = 0;
    SECTION("physical damage") {}
    SECTION("elemental damage") { type = givm::damage_type::pyro; }
    SECTION("effect reduces damage to zero") { value = 3; log.effect_reduction = 3; }
    const damage_observer_source observer{ &log };
    const givm::test::initialized_character_source victim{ "Victim" };
    constexpr givm::character_id source{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id damaged{ givm::player_id{ 1 }, 0 };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        givm::compile_mode::observed,
        std::tuple{
            givm::deal_damage{ .source = source, .target = damaged, .value = value, .type = type },
            givm::end_game{ .result = givm::game_result::both_loss }
        }, std::tuple{}, observer, victim
    );
    givm::table table;
    load_deck(table, library,
        { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { ids.get_id<givm::character_view>(victim.name()) } });
    givm::executor target;
    target.enter_entry(library);
    zero_random random;
    REQUIRE(target.step(library, table, random) == givm::execution_state::finished);
    CHECK(table[damaged].state().health == 10);
    CHECK(table[damaged].state().aura == (type == givm::damage_type::pyro ? givm::element_aura::pyro : givm::element_aura::none));
    CHECK(log.order == std::vector{ observed_event::calculation, observed_event::effect, observed_event::after_damage });
    CHECK(log.after_damage_value == 0);
}
