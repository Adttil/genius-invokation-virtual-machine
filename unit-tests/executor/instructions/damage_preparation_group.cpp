#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"
#include "../test_character_source.hpp"

namespace
{
    constexpr givm::character_id source{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id target(std::size_t index) { return { givm::player_id{ 1 }, index }; }
    constexpr givm::relative_character_target fixed_source{ givm::relative_player::self, 0 };
    constexpr givm::relative_damage_target fixed_target(std::int32_t index) { return { givm::relative_player::opponent, index }; }

    enum class phase { preparation, reaction, calculation, effect, completion };
    struct preparation_log
    {
        bool infuse_anemo = false;
        bool bonus_on_existing_pyro = false;
        phase nested_phase = phase::preparation;
        bool nested_invoked = false;
        std::vector<std::size_t> dying;
        std::vector<std::pair<phase, std::size_t>> order;
        std::vector<givm::elemental_reaction> reactions;
        std::vector<givm::element_aura> original_auras;
        std::vector<givm::element_aura> current_auras;
        std::vector<givm::damage_type> types;
        std::vector<std::uint32_t> values;
        std::vector<std::vector<std::uint32_t>> completion_health;
    };

    struct preparation_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            preparation_log* log;
            givm::program_entry entry;
            givm::program_entry nested;
            std::array<givm::damage, 2> inputs;
            std::size_t input_count;
        };
        preparation_log* log;
        std::span<const givm::damage> inputs{};
        std::string_view name() const { return "PreparationObserver"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            definition_type result{ .log = log,
                .nested = context.add_program(std::tuple{ givm::deal_damage{} }), .input_count = inputs.size() };
            if(not inputs.empty())
            {
                result.entry = context.add_program(std::tuple{ givm::deal_damage{ .input_count = inputs.size() } });
                std::ranges::copy(inputs, result.inputs.begin());
            }
            return result;
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 20, .health = 20 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::test_event&, givm::handle_context& context)
        {
            return data.input_count == 1 ? context.invoke(data.entry, data.inputs[0])
                : context.invoke(data.entry, data.inputs[0], data.inputs[1]);
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::damage_preparation& event, givm::handle_context&)
        {
            data.log->order.emplace_back(phase::preparation, event.target.index);
            if(data.log->infuse_anemo && event.type == givm::damage_type::physical)
                event.type = givm::damage_type::anemo;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::elemental_reaction_will_occur& event, givm::handle_context&)
        {
            data.log->order.emplace_back(phase::reaction, event.target.index);
            data.log->reactions.push_back(event.reaction);
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::damage_calculation& event, givm::handle_context& context)
        {
            data.log->order.emplace_back(phase::calculation, event.target.index);
            data.log->original_auras.push_back(event.reacted_aura);
            data.log->current_auras.push_back(context.table()[event.target].state().aura);
            if(data.log->bonus_on_existing_pyro && event.reacted_aura == givm::element_aura::pyro)
                event.value += 2;
            return invoke_nested(data, event.target, phase::calculation, context);
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::damage_effect& event, givm::handle_context& context)
        {
            data.log->order.emplace_back(phase::effect, event.target.index);
            data.log->types.push_back(event.type);
            data.log->values.push_back(event.value);
            return invoke_nested(data, event.target, phase::effect, context);
        }
        static givm::program_entry invoke_nested(const definition_type& data, givm::character_id current,
            phase current_phase, givm::handle_context& context)
        {
            if(data.log->nested_phase != current_phase || data.log->nested_invoked) return {};
            data.log->nested_invoked = true;
            return context.invoke(data.nested, givm::damage{
                .source = source, .target = current, .value = 3, .type = givm::damage_type::physical });
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::character_will_be_defeated& event, givm::handle_context&)
        {
            data.log->dying.push_back(event.target.index);
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::after_damage& event, givm::handle_context& context)
        {
            data.log->order.emplace_back(phase::completion, event.target.index);
            std::vector<std::uint32_t> health;
            for(const auto character : context.table()[givm::player_id{ 1 }].characters())
                health.push_back(character.state().health);
            data.log->completion_health.push_back(std::move(health));
            return {};
        }
    };

    struct zero_random { std::uint32_t operator()() const { return 0; } };

    std::vector<std::size_t> targets_at(const preparation_log& log, phase wanted)
    {
        std::vector<std::size_t> result;
        for(const auto& [current, index] : log.order)
            if(current == wanted) result.push_back(index);
        return result;
    }

    void check_preparation_precedes_numbers(const preparation_log& log)
    {
        bool calculating = false;
        bool completing = false;
        for(const auto& [current, index] : log.order)
        {
            if(current == phase::calculation || current == phase::effect)
            {
                calculating = true;
                CHECK_FALSE(completing);
            }
            else if(current == phase::completion) completing = true;
            else CHECK_FALSE(calculating);
        }
    }

    std::vector<std::size_t> run(givm::executor& executor, const givm::definition_library& library,
        givm::table& table, bool observed)
    {
        zero_random random;
        std::vector<std::size_t> targets;
        for(;;)
        {
            const auto state = executor.step(library, table, random);
            if(state == givm::execution_state::finished) return targets;
            REQUIRE(observed);
            REQUIRE(state == givm::execution_state::health_reduced);
            targets.push_back(executor.view_in<givm::execution_state::health_reduced>().target().index);
        }
    }

    givm::table active_table()
    {
        return { { .self_player = givm::player_id{ 0 } }, { .active_character = source }, { .active_character = target(0) } };
    }
}

TEST_CASE("numeric conditions read the original aura even after the whole group has applied its elements", "[deal_damage][preparation-group]")
{
    const bool observed = GENERATE(false, true);
    preparation_log log{ .bonus_on_existing_pyro = true };
    const auto observer = givm::test::with_passive_skill(preparation_source{ &log });
    const givm::test::initialized_character_source bare{ "Bare", { .max_health = 20, .health = 20 } };
    const givm::test::initialized_character_source burning{ "Burning", { .max_health = 20, .health = 20, .aura = givm::element_aura::pyro } };
    const std::array damages{
        givm::fixed_damage{ .source = fixed_source, .target = fixed_target(0), .value = 1, .type = givm::damage_type::pyro },
        givm::fixed_damage{ .source = fixed_source, .target = fixed_target(1), .value = 1, .type = givm::damage_type::pyro }
    };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::deal_damage{ .damages = damages }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, observer, bare, burning);
    auto table = active_table();
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { ids.get_id<givm::character_view>(bare.name()), ids.get_id<givm::character_view>(burning.name()) } });
    givm::executor executor;
    executor.enter_entry(library);
    const auto observations = run(executor, library, table, observed);
    CHECK(log.original_auras == std::vector{ givm::element_aura::none, givm::element_aura::pyro });
    CHECK(log.current_auras == std::vector{ givm::element_aura::pyro, givm::element_aura::pyro });
    CHECK(log.values == std::vector<std::uint32_t>{ 1, 3 });
    CHECK(table[target(0)].state().health == 19);
    CHECK(table[target(1)].state().health == 17);
    CHECK(targets_at(log, phase::preparation) == std::vector<std::size_t>{ 0, 1 });
    CHECK(observations == (observed ? std::vector<std::size_t>{ 0, 1 } : std::vector<std::size_t>{}));
    check_preparation_precedes_numbers(log);
}

TEST_CASE("a reaction prepared for a later defeated target retains its secondary damage", "[deal_damage][preparation-group][reaction]")
{
    const bool observed = GENERATE(false, true);
    preparation_log log;
    const auto observer = givm::test::with_passive_skill(preparation_source{ &log });
    const givm::test::initialized_character_source water{ "Water", { .max_health = 20, .health = 20, .aura = givm::element_aura::hydro } };
    const givm::test::initialized_character_source electro{ "Electro", { .max_health = 20, .health = 20, .aura = givm::element_aura::electro } };
    const givm::test::initialized_character_source fragile{ "Fragile", { .max_health = 20, .health = 1, .aura = givm::element_aura::electro } };
    const std::array damages{ givm::fixed_damage{ .source = fixed_source, .target = fixed_target(0),
        .value = 1, .type = givm::damage_type::anemo } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::deal_damage{ .damages = damages }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, observer, water, electro, fragile);
    auto table = active_table();
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { ids.get_id<givm::character_view>(water.name()), ids.get_id<givm::character_view>(electro.name()),
            ids.get_id<givm::character_view>(fragile.name()) } });
    givm::executor executor;
    executor.enter_entry(library);
    const auto observations = run(executor, library, table, observed);
    CHECK(log.reactions == std::vector{ givm::elemental_reaction::swirl,
        givm::elemental_reaction::electro_charged, givm::elemental_reaction::electro_charged });
    CHECK(targets_at(log, phase::reaction) == std::vector<std::size_t>{ 0, 1, 2 });
    CHECK(targets_at(log, phase::preparation) == std::vector<std::size_t>{ 0 });
    const std::vector<std::size_t> expected{ 0, 1, 2, 0, 0, 1 };
    CHECK(targets_at(log, phase::effect) == expected);
    CHECK(observations == (observed ? expected : std::vector<std::size_t>{}));
    CHECK(table[target(0)].state().health == 17);
    CHECK(table[target(1)].state().health == 17);
    CHECK(table[target(2)].state().health == 0);
    CHECK(table[target(2)].state().aura == givm::element_aura::none);
    for(const auto& health : log.completion_health) CHECK(health == std::vector<std::uint32_t>{ 17, 17, 0 });
    check_preparation_precedes_numbers(log);
}

TEST_CASE("each original wind description prepares once while its range and reaction hits retain that element", "[deal_damage][preparation-group][range]")
{
    const bool observed = GENERATE(false, true);
    const bool dynamic = GENERATE(false, true);
    const bool range = GENERATE(false, true);
    preparation_log log{ .infuse_anemo = true };
    const std::array inputs{
        givm::damage{ .source = source, .target = target(0),
            .selection = range ? givm::damage_target_selection::all : givm::damage_target_selection::character,
            .value = 1, .type = givm::damage_type::physical },
        givm::damage{ .source = source, .target = target(1), .value = 1, .type = givm::damage_type::physical }
    };
    const auto observer = givm::test::with_passive_skill(preparation_source{
        &log, dynamic ? std::span{ inputs }.first(range ? 1 : 2) : std::span<const givm::damage>{} });
    const givm::test::initialized_character_source water{ "Water", { .max_health = 20, .health = 20, .aura = givm::element_aura::hydro } };
    const givm::test::initialized_character_source bare{ "Bare", { .max_health = 20, .health = 20 } };
    const std::array damages{
        givm::fixed_damage{ .source = fixed_source,
            .target = { givm::relative_player::opponent, 0, range ? givm::damage_target_selection::all : givm::damage_target_selection::character },
            .value = 1, .type = givm::damage_type::physical },
        givm::fixed_damage{ .source = fixed_source, .target = fixed_target(1), .value = 1, .type = givm::damage_type::physical }
    };
    std::vector<givm::any_command> program;
    if(dynamic) program.emplace_back(givm::test_command{});
    else program.emplace_back(givm::deal_damage{ .damages = std::span{ damages }.first(range ? 1 : 2) });
    program.emplace_back(givm::end_game{ givm::game_result::both_loss });
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal, program, std::tuple{}, observer, water, bare);
    const auto water_id = ids.get_id<givm::character_view>(water.name());
    const auto other_id = range ? water_id : ids.get_id<givm::character_view>(bare.name());
    auto table = active_table();
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { water_id, other_id, other_id } });
    givm::executor executor;
    executor.enter_entry(library);
    const auto observations = run(executor, library, table, observed);
    CHECK(targets_at(log, phase::preparation) == (range ? std::vector<std::size_t>{ 0 } : std::vector<std::size_t>{ 0, 1 }));
    CHECK(targets_at(log, phase::reaction) == (range ? std::vector<std::size_t>{ 0, 1, 2 } : std::vector<std::size_t>{ 0, 1 }));
    const auto expected = range ? std::vector<std::size_t>{ 0, 1, 2, 1, 2, 0, 2, 0, 1 }
        : std::vector<std::size_t>{ 0, 1, 2, 1, 2, 0 };
    CHECK(targets_at(log, phase::effect) == expected);
    CHECK(targets_at(log, phase::completion) == expected);
    CHECK(observations == (observed ? expected : std::vector<std::size_t>{}));
    const auto one_swirl = std::vector{ givm::damage_type::anemo, givm::damage_type::hydro, givm::damage_type::hydro };
    for(std::size_t i = 0; i < log.types.size(); ++i) CHECK(log.types[i] == one_swirl[i % 3]);
    for(const auto character : table[givm::player_id{ 1 }].characters()) CHECK(character.state().health == (range ? 17 : 18));
    check_preparation_precedes_numbers(log);
}

TEST_CASE("defeat clears aura applied while preparing a later skipped hit", "[deal_damage][preparation-group][defeat]")
{
    const bool observed = GENERATE(false, true);
    preparation_log log;
    const auto observer = givm::test::with_passive_skill(preparation_source{ &log });
    const givm::test::initialized_character_source fragile{ "Fragile", { .max_health = 20, .max_energy = 3, .health = 1, .energy = 3 } };
    const givm::test::initialized_character_source reserve{ "Reserve", { .max_health = 20, .health = 20 } };
    const std::array damages{
        givm::fixed_damage{ .source = fixed_source, .target = fixed_target(0), .value = 1, .type = givm::damage_type::physical },
        givm::fixed_damage{ .source = fixed_source, .target = fixed_target(0), .value = 1, .type = givm::damage_type::pyro }
    };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::deal_damage{ .damages = damages }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, observer, fragile, reserve);
    auto table = active_table();
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { ids.get_id<givm::character_view>(fragile.name()), ids.get_id<givm::character_view>(reserve.name()) } });
    givm::executor executor;
    executor.enter_entry(library);
    const auto observations = run(executor, library, table, observed);
    CHECK(targets_at(log, phase::preparation) == std::vector<std::size_t>{ 0, 0 });
    CHECK(log.current_auras == std::vector{ givm::element_aura::pyro });
    CHECK(targets_at(log, phase::effect) == std::vector<std::size_t>{ 0 });
    CHECK(table[target(0)].state().health == 0);
    CHECK(table[target(0)].state().energy == 0);
    CHECK(table[target(0)].state().aura == givm::element_aura::none);
    CHECK(table[target(1)].state().health == 20);
    CHECK(observations == (observed ? std::vector<std::size_t>{ 0 } : std::vector<std::size_t>{}));
    check_preparation_precedes_numbers(log);
}

TEST_CASE("damage resumes past a target defeated by its own numeric or shield response", "[deal_damage][preparation-group][nested][defeat]")
{
    const bool observed = GENERATE(false, true);
    const auto nested_phase = GENERATE(phase::calculation, phase::effect);
    preparation_log log{ .nested_phase = nested_phase };
    const auto observer = givm::test::with_passive_skill(preparation_source{ &log });
    const givm::test::initialized_character_source fragile{ "Fragile", { .max_health = 20, .health = 3 } };
    const givm::test::initialized_character_source reserve{ "Reserve", { .max_health = 20, .health = 20 } };
    const std::array damages{
        givm::fixed_damage{ .source = fixed_source, .target = fixed_target(0), .value = 1, .type = givm::damage_type::physical },
        givm::fixed_damage{ .source = fixed_source, .target = fixed_target(1), .value = 2, .type = givm::damage_type::physical }
    };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::deal_damage{ .damages = damages }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, observer, fragile, reserve);
    auto table = active_table();
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { ids.get_id<givm::character_view>(fragile.name()), ids.get_id<givm::character_view>(reserve.name()) } });
    givm::executor executor;
    executor.enter_entry(library);
    const auto observations = run(executor, library, table, observed);
    CHECK(log.dying == std::vector<std::size_t>{ 0 });
    CHECK(targets_at(log, phase::completion) == std::vector<std::size_t>{ 0, 1 });
    CHECK(log.completion_health == std::vector<std::vector<std::uint32_t>>{ { 0, 20 }, { 0, 18 } });
    CHECK(table[target(0)].state().health == 0);
    CHECK(table[target(1)].state().health == 18);
    CHECK(observations == (observed ? std::vector<std::size_t>{ 0, 1 } : std::vector<std::size_t>{}));
}
