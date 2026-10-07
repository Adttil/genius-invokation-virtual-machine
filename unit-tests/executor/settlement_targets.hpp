#include <algorithm>
#include <array>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "test_character_source.hpp"

namespace givm_test::executor::settlement_targets
{
namespace
{
    using namespace givm;
    constexpr character_id character(std::size_t player, std::size_t index)
    {
        return { player_id{ static_cast<std::uint32_t>(player) }, static_cast<std::uint32_t>(index) };
    }

    struct flow_log
    {
        bool batch = false;
        bool dynamic = false;
        bool target_modes = false;
        bool nested = false;
        bool nested_kills = false;
        bool nested_started = false;
        bool revive_in_range = false;
        bool revived = false;
        std::vector<character_id> hits;
        std::vector<after_damage> damages;
        std::vector<std::uint32_t> heals;
        std::vector<character_id> changes;
        std::vector<std::array<character_id, 2>> active_at_change;
        std::vector<int> order;
        bool dead_at_continuation = false;
    };

    struct observer_source
    {
        static constexpr auto category = givm::definition_category::character;
        struct definition_type
        {
            flow_log* log;
            normal_effect batch;
            normal_effect targets;
            normal_effect nested;
            immediate_effect revive;
            normal_effect marker;
        };
        flow_log* log;
        std::string_view name() const { return "TargetObserver"; }
        definition_type compile(definition_compile_context& context) const
        {
            const auto batch = log->dynamic
                ? context.add_normal_effect(deal_damage{}, givm::heal{}, deal_damage{}, givm::heal{}, set_energy{})
                : context.add_normal_effect(
                    deal_damage{ .target = { relative_player::opponent, 1 }, .value = 1, .type = damage_type::physical },
                    deal_damage{ .target = { relative_player::opponent, 1 }, .value = 2, .type = damage_type::pyro },
                    givm::heal{ .target = { relative_player::opponent, 1 }, .value = 1 },
                    givm::heal{ .target = { relative_player::opponent, 1 }, .value = 2 },
                    set_energy{ .target = { relative_player::self, 0 }, .value = 2 });
            return { log, batch, context.add_normal_effect(deal_damage{}),
                context.add_normal_effect(deal_damage{}, return_response{ 1 }),
                context.add_immediate_effect(givm::heal{}), context.add_normal_effect(return_response{ 1 }) };
        }
        static character_state query(const definition_type&, const character_initial_state&)
        {
            return { .max_health = 10, .max_energy = 3, .health = 10 };
        }
        static normal_effect handle(const definition_type& data, round_started&,
            handle_context<skill_view>& context, std::uint32_t = 0)
        {
            if(data.log->target_modes)
            {
                const std::array inputs{
                    damage{ .source = context.entity().id(), .target = relative_character_target{ relative_player::opponent },
                        .value = 2, .type = damage_type::physical },
                    damage{ .source = context.entity().id(), .target = character(1, 0), .value = 2, .type = damage_type::physical },
                    damage{ .source = context.entity().id(), .target = relative_character_target{ relative_player::opponent, 0,
                        character_selection::others }, .value = 1, .type = damage_type::physical },
                    damage{ .source = context.entity().id(), .target = relative_character_target{ relative_player::opponent, 0,
                        character_selection::prioritized }, .value = 2, .type = damage_type::physical }
                };
                return context.invoke(data.targets, deal_damage_input{ inputs });
            }
            if(not data.log->batch) return {};
            if(not data.log->dynamic) return context.invoke(data.batch);
            std::vector inputs{
                damage{ .source = context.entity().id(), .target = character(1, 1), .value = 1, .type = damage_type::physical },
                damage{ .source = context.entity().id(), .target = character(1, 1), .value = 2, .type = damage_type::pyro }
            };
            std::vector heals{
                heal_input::item{ context.entity().id(), character(1, 1), 1 },
                heal_input::item{ context.entity().id(), character(1, 1), 2 }
            };
            auto packed = pack_inputs(deal_damage_input{ inputs }, heal_input{ heals },
                deal_damage_input{}, heal_input{}, set_energy_input{ context.entity().character().id(), 2 });
            inputs.clear();
            heals.clear();
            return context.invoke(data.batch, packed);
        }
        static immediate_effect handle(const definition_type& data, damage_preparation& event,
            handle_context<skill_view, event_category::immediate>& context, std::uint32_t = 0)
        {
            data.log->hits.push_back(event.target);
            if(data.log->revive_in_range && not data.log->revived)
            {
                data.log->revived = true;
                return context.invoke(data.revive, heal_input{ std::array{
                    heal_input::item{ context.entity().id(), character(1, 1), 3, healing_kind::revive } } });
            }
            return {};
        }
        static normal_effect handle(const definition_type& data, after_damage& event,
            handle_context<skill_view>& context, std::uint32_t index = 0)
        {
            if(index != 0)
            {
                data.log->order.push_back(1);
                const auto active = *context.table()[player_id{ 1 }].state().active_character;
                data.log->dead_at_continuation = not context.table()[active].state().alive;
                return {};
            }
            data.log->damages.push_back(event);
            data.log->order.push_back(0);
            if(data.log->nested && event.target == character(1, 0) && not data.log->nested_started)
            {
                data.log->nested_started = true;
                return context.invoke(data.nested, deal_damage_input{ std::array{
                    damage{ .source = context.entity().id(), .target = character(0, 1),
                        .value = data.log->nested_kills ? 1u : 0u, .type = damage_type::physical } } });
            }
            return {};
        }
        static normal_effect handle(const definition_type& data, healed& event,
            handle_context<skill_view>&, std::uint32_t = 0)
        {
            data.log->heals.push_back(event.value);
            return {};
        }
        static normal_effect handle(const definition_type& data, active_character_changed& event,
            handle_context<skill_view>& context, std::uint32_t index = 0)
        {
            const auto player = event.current.player_id().index();
            data.log->order.push_back(static_cast<int>((index == 0 ? 2 : 4) + player));
            if(index != 0) return {};
            data.log->changes.push_back(event.current);
            data.log->active_at_change.push_back({
                *context.table()[player_id{ 0 }].state().active_character,
                *context.table()[player_id{ 1 }].state().active_character });
            return context.invoke(data.marker);
        }
    };

    std::size_t finish(executor_driver& execution, const definition_library& library, givm::table& table)
    {
        std::size_t choices = 0;
        for(;;)
        {
            const auto state = execution.advance(library, table, zero_random);
            if(state == execution_state::finished) return choices;
            if(state == execution_state::health_reduced) continue;
            REQUIRE(state == execution_state::active_character_selection);
            ++choices;
            const auto view = execution.view_in<execution_state::active_character_selection>();
            for(const auto candidate : table[view.player()].characters())
                if(candidate.state().alive && candidate.state().health != 0)
                {
                    execution.submitted(view.select(library, table, zero_random, candidate.id()));
                    break;
                }
        }
    }
}

TEST_CASE("dynamic damage and healing batches share the fixed sequence settlement behavior", "[settlement][batch-input]")
{
    const auto mode = GENERATE(compile_mode::normal, compile_mode::observed);
    const bool dynamic = GENERATE(false, true);
    flow_log log{ .batch = true, .dynamic = dynamic };
    const auto observer = givm::test::with_passive_skill(observer_source{ &log });
    const givm::test::initialized_character_source victim{ "Victim", { .max_health = 20, .health = 20 } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ start_round{}, settle{}, end_game{ game_result::both_loss } }, std::tuple{}, observer, victim);
    givm::table table{ { .self_player = player_id{ 0 } }, { .active_character = character(0, 0) },
        { .active_character = character(1, 0) } };
    const auto victim_id = ids.get_id<givm::definition_category::character>(victim.name());
    load_deck(table, library, { .characters = { ids.get_id<givm::definition_category::character>(observer.name()) } },
        { .characters = { victim_id, victim_id } });
    executor_driver execution;
    execution.start(library, table);
    CHECK(finish(execution, library, table) == 0);
    CHECK(log.hits == std::vector{ character(1, 1), character(1, 1) });
    CHECK(log.heals == std::vector<std::uint32_t>{ 1, 2 });
    REQUIRE(log.damages.size() == 1);
    CHECK(log.damages[0].value == 3);
    CHECK(log.damages[0].type.count() == 2);
    CHECK(table[character(1, 1)].state().health == 20);
    CHECK(table[character(0, 0)].state().energy == 2);
}

TEST_CASE("ordinary damage and standby ranges do not inherit prioritized targeting", "[settlement][damage-target]")
{
    const auto mode = GENERATE(compile_mode::normal, compile_mode::observed);
    const bool dynamic = GENERATE(false, true);
    flow_log log{ .target_modes = dynamic };
    const auto observer = givm::test::with_passive_skill(observer_source{ &log });
    const givm::test::initialized_character_source victim{ "Victim", { .max_health = 10, .health = 10 } };
    const givm::test::initialized_character_source dead{ "Dead", { .max_health = 10, .health = 0, .alive = false } };
    std::vector<any_command> commands;
    if(dynamic) commands = { start_round{} };
    else commands = {
        deal_damage{ .target = { relative_player::opponent, 0 }, .value = 2, .type = damage_type::physical },
        deal_damage{ .target = { relative_player::opponent, 0 }, .value = 2, .type = damage_type::physical },
        deal_damage{ .target = { relative_player::opponent, 0, character_selection::others }, .value = 1, .type = damage_type::physical },
        deal_damage{ .target = { relative_player::opponent, 0, character_selection::prioritized }, .value = 2, .type = damage_type::physical }
    };
    commands.emplace_back(settle{});
    commands.emplace_back(end_game{ game_result::both_loss });
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode, commands, std::tuple{}, observer, victim, dead);
    givm::table table{ { .self_player = player_id{ 0 } }, { .active_character = character(0, 0) },
        { .active_character = character(1, 0) } };
    const auto victim_id = ids.get_id<givm::definition_category::character>(victim.name());
    load_deck(table, library, { .characters = { ids.get_id<givm::definition_category::character>(observer.name()) } },
        { .characters = { ids.get_id<givm::definition_category::character>(dead.name()), victim_id, victim_id } });
    executor_driver execution;
    execution.start(library, table);
    CHECK(finish(execution, library, table) == 0);
    CHECK(log.hits == std::vector{ character(1, 1), character(1, 2), character(1, 1) });
    CHECK(table[player_id{ 1 }].state().active_character == character(1, 0));
    CHECK(table[character(1, 1)].state().health == 7);
    CHECK(table[character(1, 2)].state().health == 9);
}

TEST_CASE("zero health characters still receive damage and reactions before segment sealing", "[settlement][damage-target][dying]")
{
    const auto mode = GENERATE(compile_mode::normal, compile_mode::observed);
    flow_log log;
    const auto observer = givm::test::with_passive_skill(observer_source{ &log });
    const givm::test::initialized_character_source victim{ "Victim", { .max_health = 10, .health = 1 } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode, std::tuple{
        deal_damage{ .target = { relative_player::opponent, 0 }, .value = 1, .type = damage_type::hydro },
        deal_damage{ .target = { relative_player::opponent, 0 }, .value = 1, .type = damage_type::pyro },
        deal_damage{ .target = { relative_player::opponent, 0 }, .value = 1, .type = damage_type::physical },
        settle{}, end_game{ game_result::both_loss } }, std::tuple{}, observer, victim);
    givm::table table{ { .self_player = player_id{ 0 } }, { .active_character = character(0, 0) },
        { .active_character = character(1, 0) } };
    const auto victim_id = ids.get_id<givm::definition_category::character>(victim.name());
    load_deck(table, library, { .characters = { ids.get_id<givm::definition_category::character>(observer.name()) } },
        { .characters = { victim_id, victim_id } });
    executor_driver execution;
    execution.start(library, table);
    CHECK(finish(execution, library, table) == 1);
    CHECK(log.hits == std::vector(3, character(1, 0)));
    REQUIRE(log.damages.size() == 1);
    CHECK(log.damages[0].value == 5);
    CHECK(log.damages[0].reaction[elemental_reaction::vaporize]);
    CHECK(log.damages[0].defeated);
}

TEST_CASE("a range does not acquire characters revived by its first inline response", "[settlement][damage-target][range]")
{
    const auto mode = GENERATE(compile_mode::normal, compile_mode::observed);
    flow_log log{ .revive_in_range = true };
    const auto observer = givm::test::with_passive_skill(observer_source{ &log });
    const givm::test::initialized_character_source victim{ "Victim", { .max_health = 10, .health = 10 } };
    const givm::test::initialized_character_source dead{ "Dead", { .max_health = 10, .health = 0, .alive = false } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode, std::tuple{
        deal_damage{ .target = { relative_player::opponent, 0, character_selection::all }, .value = 1, .type = damage_type::physical },
        settle{}, end_game{ game_result::both_loss } }, std::tuple{}, observer, victim, dead);
    givm::table table{ { .self_player = player_id{ 0 } }, { .active_character = character(0, 0) },
        { .active_character = character(1, 0) } };
    const auto victim_id = ids.get_id<givm::definition_category::character>(victim.name());
    load_deck(table, library, { .characters = { ids.get_id<givm::definition_category::character>(observer.name()) } },
        { .characters = { victim_id, ids.get_id<givm::definition_category::character>(dead.name()), victim_id } });
    executor_driver execution;
    execution.start(library, table);
    CHECK(finish(execution, library, table) == 0);
    CHECK(log.hits == std::vector{ character(1, 0), character(1, 2) });
    CHECK(table[character(1, 1)].state().alive);
    CHECK(table[character(1, 1)].state().health == 3);
}

TEST_CASE("only a child settlement with its own defeat checks the parent's defeated active", "[settlement][defeat-selection][nested]")
{
    const bool kills = GENERATE(false, true);
    flow_log log{ .nested = true, .nested_kills = kills };
    const auto observer = givm::test::with_passive_skill(observer_source{ &log });
    const givm::test::initialized_character_source fragile{ "Fragile", { .max_health = 10, .health = 1 } };
    const givm::test::initialized_character_source victim{ "Victim", { .max_health = 10, .health = 10 } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(compile_mode::normal, std::tuple{
        deal_damage{ .target = { relative_player::opponent, 0 }, .value = 1, .type = damage_type::physical },
        settle{}, end_game{ game_result::both_loss } }, std::tuple{}, observer, fragile, victim);
    givm::table table{ { .self_player = player_id{ 0 } }, { .active_character = character(0, 0) },
        { .active_character = character(1, 0) } };
    const auto fragile_id = ids.get_id<givm::definition_category::character>(fragile.name());
    load_deck(table, library, { .characters = { ids.get_id<givm::definition_category::character>(observer.name()), fragile_id } },
        { .characters = { fragile_id, ids.get_id<givm::definition_category::character>(victim.name()) } });
    executor_driver execution;
    execution.start(library, table);
    REQUIRE(execution.advance(library, table, zero_random) == execution_state::active_character_selection);
    CHECK(log.dead_at_continuation == not kills);
    CHECK((std::find(log.order.begin(), log.order.end(), 1) == log.order.end()) == kills);
    const auto view = execution.view_in<execution_state::active_character_selection>();
    CHECK(view.player() == player_id{ 1 });
    execution.submitted(view.select(library, table, zero_random, character(1, 1)));
    CHECK(finish(execution, library, table) == 0);
    CHECK(log.dead_at_continuation == not kills);
    CHECK(std::count(log.order.begin(), log.order.end(), 1) == 1);
}

TEST_CASE("both defeated active choices are collected before switches and acting-player notifications", "[settlement][defeat-selection][both]")
{
    const auto mode = GENERATE(compile_mode::normal, compile_mode::observed);
    const std::size_t first = GENERATE(0u, 1u);
    flow_log log;
    const auto observer = givm::test::with_passive_skill(observer_source{ &log });
    const givm::test::initialized_character_source fragile{ "Fragile", { .max_health = 10, .health = 1 } };
    const givm::test::initialized_character_source victim{ "Victim", { .max_health = 10, .health = 10 } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode, std::tuple{
        deal_damage{ .target = { relative_player::opponent, 0 }, .value = 1, .type = damage_type::physical },
        deal_damage{ .target = { relative_player::self, 0 }, .value = 1, .type = damage_type::physical },
        settle{}, end_game{ game_result::both_loss } }, std::tuple{}, observer, fragile, victim);
    givm::table table{ { .active_player = player_id{ static_cast<std::uint32_t>(first) }, .self_player = player_id{ 0 } },
        { .active_character = character(0, 0) }, { .active_character = character(1, 0) } };
    const auto fragile_id = ids.get_id<givm::definition_category::character>(fragile.name());
    load_deck(table, library, { .characters = { fragile_id, ids.get_id<givm::definition_category::character>(observer.name()) } },
        { .characters = { fragile_id, ids.get_id<givm::definition_category::character>(victim.name()) } });
    executor_driver execution;
    execution.start(library, table);
    auto state = execution.advance(library, table, zero_random);
    while(state == execution_state::health_reduced) state = execution.advance(library, table, zero_random);
    REQUIRE(state == execution_state::active_character_selection);
    const auto first_view = execution.view_in<execution_state::active_character_selection>();
    CHECK(first_view.player() == player_id{ 0 });
    execution.submitted(first_view.select(library, table, zero_random, character(0, 1)));
    REQUIRE(execution.advance(library, table, zero_random) == execution_state::active_character_selection);
    CHECK(log.changes.empty());
    CHECK(table[player_id{ 0 }].state().active_character == character(0, 0));
    CHECK(table[player_id{ 1 }].state().active_character == character(1, 0));
    auto copied_execution = execution;
    auto copied_table = table;
    execution = std::move(copied_execution);
    table = std::move(copied_table);
    const auto second_view = execution.view_in<execution_state::active_character_selection>();
    CHECK(second_view.player() == player_id{ 1 });
    execution.submitted(second_view.select(library, table, zero_random, character(1, 1)));
    CHECK(finish(execution, library, table) == 0);
    CHECK(log.changes == std::vector{ character(first, 1), character(1 - first, 1) });
    REQUIRE(log.active_at_change.size() == 2);
    CHECK(log.active_at_change[0][first] == character(first, 1));
    CHECK(log.active_at_change[0][1 - first] == character(1 - first, 0));
    CHECK(log.active_at_change[1] == std::array{ character(0, 1), character(1, 1) });
    const auto begin = std::find(log.order.begin(), log.order.end(), static_cast<int>(2 + first));
    CHECK(std::vector(begin, log.order.end()) == std::vector<int>{ static_cast<int>(2 + first), static_cast<int>(4 + first),
        static_cast<int>(3 - first), static_cast<int>(5 - first) });
}

TEST_CASE("the entire segment resolves dying before a simultaneous team defeat ends the game", "[settlement][dying][both-loss]")
{
    const auto mode = GENERATE(compile_mode::normal, compile_mode::observed);
    const givm::test::initialized_character_source fragile{ "Fragile", { .max_health = 10, .health = 1 } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode, std::tuple{
        deal_damage{ .target = { relative_player::self, 0 }, .value = 1, .type = damage_type::physical },
        deal_damage{ .target = { relative_player::opponent, 0 }, .value = 1, .type = damage_type::physical },
        settle{}, end_game{ game_result::player_0_win } }, std::tuple{}, fragile);
    givm::table table{ { .self_player = player_id{ 0 } }, { .active_character = character(0, 0) },
        { .active_character = character(1, 0) } };
    const auto id = ids.get_id<givm::definition_category::character>(fragile.name());
    load_deck(table, library, { .characters = { id } }, { .characters = { id } });
    executor_driver execution;
    execution.start(library, table);
    CHECK(finish(execution, library, table) == 0);
    CHECK(execution.view_in<execution_state::finished>().result() == game_result::both_loss);
    CHECK_FALSE(table[character(0, 0)].state().alive);
    CHECK_FALSE(table[character(1, 0)].state().alive);
}

TEST_CASE("segment boundaries keep dead active positions until an explicit settlement completes", "[settlement][damage-target][checkpoint]")
{
    const auto mode = GENERATE(compile_mode::normal, compile_mode::observed);
    flow_log log;
    const auto observer = givm::test::with_passive_skill(observer_source{ &log });
    const givm::test::initialized_character_source fragile{ "Fragile", { .max_health = 10, .health = 1 } };
    const givm::test::initialized_character_source victim{ "Victim", { .max_health = 10, .health = 10 } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode, std::tuple{
        deal_damage{ .target = { relative_player::opponent, 0 }, .value = 1, .type = damage_type::physical },
        end_segment{},
        deal_damage{ .target = { relative_player::opponent, 0 }, .value = 2, .type = damage_type::physical },
        deal_damage{ .target = { relative_player::opponent, 0, character_selection::prioritized }, .value = 1, .type = damage_type::physical },
        settle{},
        deal_damage{ .target = { relative_player::opponent, 0 }, .value = 1, .type = damage_type::physical },
        settle{}, end_game{ game_result::both_loss } }, std::tuple{}, observer, fragile, victim);
    givm::table table{ { .self_player = player_id{ 0 } }, { .active_character = character(0, 0) },
        { .active_character = character(1, 0) } };
    load_deck(table, library, { .characters = { ids.get_id<givm::definition_category::character>(observer.name()) } },
        { .characters = { ids.get_id<givm::definition_category::character>(fragile.name()), ids.get_id<givm::definition_category::character>(victim.name()) } });
    executor_driver execution;
    execution.start(library, table);
    CHECK(finish(execution, library, table) == 1);
    CHECK(log.hits == std::vector{ character(1, 0), character(1, 1), character(1, 1) });
    CHECK(table[character(1, 1)].state().health == 8);
    CHECK(log.changes == std::vector{ character(1, 1) });
}
}
