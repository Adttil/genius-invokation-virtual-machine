#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"

namespace
{
    constexpr givm::character_id patient{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id other{ givm::player_id{ 0 }, 1 };

    struct healing_log
    {
        std::uint32_t initial_health = 4;
        std::uint32_t value = 2;
        std::uint32_t bonus = 0;
        bool dynamic = false;
        bool pause = false;
        bool relative_input = false;
        std::int32_t target_offset = 0;
        givm::character_selection selection = givm::character_selection::character;
        std::vector<std::uint32_t> requested;
        std::vector<std::uint32_t> actual;
        std::vector<givm::character_id> healing_targets;
        std::vector<givm::character_id> healed_targets;
        std::vector<std::array<std::uint32_t, 2>> healed_snapshots;
    };

    struct healing_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            healing_log* log;
            givm::program_entry heal;
            givm::program_entry pause;
        };
        healing_log* log;
        std::string_view name() const { return "HealingSource"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto command = log->dynamic ? givm::heal{} : givm::heal{
                .source = givm::relative_character_target{ givm::relative_player::self, 0 },
                .target = givm::relative_character_target{ givm::relative_player::self,
                    log->target_offset, log->selection },
                .value = log->value };
            return { log, context.add_program(std::tuple{ command }), log->pause
                ? context.add_program(std::tuple{ givm::replace_cards{ givm::player_id{ 0 } } }) : givm::program_entry{} };
        }
        static givm::character_state query(const definition_type& data, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = data.log->initial_health };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::test_event&, givm::handle_context& context)
        {
            if(data.log->dynamic)
            {
                if(data.log->relative_input)
                    return context.invoke(data.heal, givm::heal_input{ patient,
                        givm::relative_character_target{ givm::relative_player::self,
                            data.log->target_offset, data.log->selection }, data.log->value });
                return context.invoke(data.heal, givm::heal_input{ patient, patient, data.log->value });
            }
            return context.invoke(data.heal);
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::healing& event, givm::handle_context& context)
        {
            CHECK(std::get<givm::character_id>(event.source) == patient);
            data.log->requested.push_back(event.value);
            data.log->healing_targets.push_back(event.target);
            event.value += data.log->bonus;
            return data.pause ? context.invoke(data.pause) : givm::program_entry{};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::healed& event, givm::handle_context& context)
        {
            CHECK(std::get<givm::character_id>(event.source) == patient);
            data.log->actual.push_back(event.value);
            data.log->healed_targets.push_back(event.target);
            data.log->healed_snapshots.push_back({ context.table()[patient].state().health,
                context.table()[other].state().health });
            return data.pause ? context.invoke(data.pause) : givm::program_entry{};
        }
    };

    struct bare_character_source
    {
        using definition_category = givm::character_view;
        struct definition_type { std::uint32_t initial_health; };
        std::uint32_t initial_health;
        std::string_view name() const { return "BareCharacter"; }
        definition_type compile(givm::definition_compile_context&) const { return { initial_health }; }
        static givm::character_state query(const definition_type& data, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = data.initial_health };
        }
    };

    auto compile_healing(healing_log& log, givm::compile_mode mode, std::uint32_t other_health = 4)
    {
        return givm::test::compile_definitions_with_program(mode,
            std::tuple{ givm::test_command{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
            givm::test::with_passive_skill(healing_source{ &log }), bare_character_source{ other_health });
    }

    void load_healing(givm::table& table, const givm::definition_library& library, const givm::issued_id_map& ids)
    {
        load_deck(table, library, { .characters = { ids.get_id<givm::character_view>("HealingSource"),
            ids.get_id<givm::character_view>("BareCharacter") } }, {});
    }
}

TEST_CASE("range healing applies every target before healed notifications", "[heal][group]")
{
    healing_log log{ .initial_health = GENERATE(4u, 10u), .value = 2, .bonus = 1,
        .dynamic = GENERATE(false, true), .relative_input = true,
        .target_offset = GENERATE(0, 1, -1),
        .selection = GENERATE(givm::character_selection::all, givm::character_selection::others) };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto [library, ids] = compile_healing(log, mode);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = patient } };
    load_healing(table, library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    auto random = [] { return std::uint32_t{ 0 }; };
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    const auto anchor = log.target_offset == 0 ? patient : other;
    const auto remaining = anchor == patient ? other : patient;
    const auto expected_targets = log.selection == givm::character_selection::all
        ? std::vector{ anchor, remaining } : std::vector{ remaining };
    const std::array<std::uint32_t, 2> expected_health{
        std::ranges::any_of(expected_targets, [](auto id) { return id == patient; })
            ? std::min(log.initial_health + 3u, 10u) : log.initial_health,
        std::ranges::any_of(expected_targets, [](auto id) { return id == other; }) ? 7u : 4u };
    std::vector<std::uint32_t> expected_recovery;
    for(const auto target : expected_targets)
        expected_recovery.push_back(target == patient ? std::min(3u, 10u - log.initial_health) : 3u);
    CHECK(table[patient].state().health == expected_health[0]);
    CHECK(table[other].state().health == expected_health[1]);
    CHECK(log.requested == std::vector<std::uint32_t>(expected_targets.size(), 2u));
    CHECK(log.actual == expected_recovery);
    CHECK(log.healing_targets == expected_targets);
    CHECK(log.healed_targets == expected_targets);
    CHECK(log.healed_snapshots == std::vector<std::array<std::uint32_t, 2>>(expected_targets.size(), expected_health));
}

TEST_CASE("healing modifies the request then reports actual recovery including zero", "[heal]")
{
    healing_log log;
    log.dynamic = GENERATE(false, true);
    if(log.dynamic) log.relative_input = GENERATE(false, true);
    log.initial_health = GENERATE(0u, 5u, 10u);
    log.value = GENERATE(0u, 2u, std::numeric_limits<std::uint32_t>::max() - 3);
    log.bonus = GENERATE(0u, 3u);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto [library, ids] = compile_healing(log, mode);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } } };
    load_healing(table, library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    auto random = [] { return std::uint32_t{ 0 }; };
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    const auto actual = std::min(log.value + log.bonus, 10 - log.initial_health);
    CHECK(table[patient].state().health == log.initial_health + actual);
    CHECK(log.requested == std::vector{ log.value });
    CHECK(log.actual == std::vector{ actual });
    CHECK(log.healing_targets == std::vector{ patient });
    CHECK(log.healed_targets == std::vector{ patient });
}

TEST_CASE("range healing excludes defeated characters and permits an empty range", "[heal][group]")
{
    healing_log log{ .value = 2, .bonus = 1, .dynamic = GENERATE(false, true), .relative_input = true,
        .selection = GENERATE(givm::character_selection::all, givm::character_selection::others) };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto [library, ids] = compile_healing(log, mode, 0);
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = patient } };
    load_healing(table, library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    auto random = [] { return std::uint32_t{ 0 }; };
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(table[other].state().health == 0);
    if(log.selection == givm::character_selection::all)
    {
        CHECK(table[patient].state().health == 7);
        CHECK(log.requested == std::vector{ 2u });
        CHECK(log.actual == std::vector{ 3u });
        CHECK(log.healing_targets == std::vector{ patient });
        CHECK(log.healed_targets == std::vector{ patient });
        CHECK(log.healed_snapshots == std::vector<std::array<std::uint32_t, 2>>{ { 7, 0 } });
    }
    else
    {
        CHECK(table[patient].state().health == 4);
        CHECK(log.requested.empty());
        CHECK(log.actual.empty());
        CHECK(log.healing_targets.empty());
        CHECK(log.healed_targets.empty());
    }
}

TEST_CASE("healing broadcasts resume and copy before and after recovery", "[heal][resume]")
{
    healing_log log{ .value = 5, .bonus = 1, .dynamic = GENERATE(false, true), .pause = true };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto [library, ids] = compile_healing(log, mode);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } } };
    load_healing(table, library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    auto random = [] { return std::uint32_t{ 0 }; };
    REQUIRE(executor.step(library, table, random) == givm::execution_state::card_selection);
    CHECK(table[patient].state().health == 4);
    CHECK(log.requested == std::vector{ 5u });
    CHECK(log.actual.empty());
    auto before_executor = executor;
    auto before_table = table;
    for(auto [running, current] : { std::pair{ &executor, &table }, std::pair{ &before_executor, &before_table } })
    {
        running->view_in<givm::execution_state::card_selection>().select({});
        REQUIRE(running->step(library, *current, random) == givm::execution_state::card_selection);
        CHECK((*current)[patient].state().health == 10);
        auto after_executor = *running;
        auto after_table = *current;
        for(auto [finishing, final] : { std::pair{ running, current }, std::pair{ &after_executor, &after_table } })
        {
            finishing->view_in<givm::execution_state::card_selection>().select({});
            REQUIRE(finishing->step(library, *final, random) == givm::execution_state::finished);
            CHECK((*final)[patient].state().health == 10);
        }
    }
    CHECK(log.requested == std::vector{ 5u });
    CHECK(log.actual == std::vector{ 6u, 6u });
}

TEST_CASE("range healing resumes and copies during calculations and notifications", "[heal][group][resume]")
{
    healing_log log{ .value = 2, .bonus = 1, .dynamic = GENERATE(false, true),
        .pause = true, .relative_input = true, .selection = givm::character_selection::all };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto [library, ids] = compile_healing(log, mode);
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = patient } };
    load_healing(table, library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    auto random = [] { return std::uint32_t{ 0 }; };

    REQUIRE(executor.step(library, table, random) == givm::execution_state::card_selection);
    CHECK(table[patient].state().health == 4);
    CHECK(table[other].state().health == 4);
    executor.view_in<givm::execution_state::card_selection>().select({});
    REQUIRE(executor.step(library, table, random) == givm::execution_state::card_selection);
    CHECK(table[patient].state().health == 7);
    CHECK(table[other].state().health == 4);
    CHECK(log.requested == std::vector{ 2u, 2u });
    CHECK(log.actual.empty());
    auto before_executor = executor;
    auto before_table = table;
    for(auto [running, current] : { std::pair{ &executor, &table }, std::pair{ &before_executor, &before_table } })
    {
        running->view_in<givm::execution_state::card_selection>().select({});
        REQUIRE(running->step(library, *current, random) == givm::execution_state::card_selection);
        CHECK((*current)[patient].state().health == 7);
        CHECK((*current)[other].state().health == 7);
        auto after_executor = *running;
        auto after_table = *current;
        for(auto [finishing, final] : { std::pair{ running, current }, std::pair{ &after_executor, &after_table } })
        {
            finishing->view_in<givm::execution_state::card_selection>().select({});
            REQUIRE(finishing->step(library, *final, random) == givm::execution_state::card_selection);
            finishing->view_in<givm::execution_state::card_selection>().select({});
            REQUIRE(finishing->step(library, *final, random) == givm::execution_state::finished);
            CHECK((*final)[patient].state().health == 7);
            CHECK((*final)[other].state().health == 7);
        }
    }
    CHECK(log.healing_targets == std::vector{ patient, other });
    CHECK(log.healed_targets == std::vector{ patient, other, other, patient, other, other });
    CHECK(log.actual == std::vector<std::uint32_t>(6, 3u));
    CHECK(log.healed_snapshots == std::vector<std::array<std::uint32_t, 2>>(6, { 7, 7 }));
}
