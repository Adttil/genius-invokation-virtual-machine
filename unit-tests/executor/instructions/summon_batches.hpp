#include <algorithm>
#include <array>
#include <cstdint>
#include <ranges>
#include <span>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"

namespace givm_test::executor_instructions::summon_batches
{
namespace
{
    struct batch_log
    {
        bool set_then_remove = false;
        bool nested = false;
        bool nested_started = false;
        bool empty = false;
        bool restore_later = false;
        std::size_t phase = 0;
        std::vector<givm::summon_id> original;
        std::vector<givm::summon_id> removed;
        std::vector<std::vector<std::uint32_t>> usages_at_removal;
        std::vector<std::uint32_t> values_at_removal;
    };

    struct batch_summon_source
    {
        static constexpr auto category = givm::definition_category::summon;
        struct definition_type {};
        bool ordinary;
        std::string_view name() const { return ordinary ? "BatchOrdinary" : "BatchPersistent"; }
        auto tags() const
        {
            return std::array<std::string_view, 1>{ ordinary ? "remove_at_zero_usages" : "persistent" };
        }
        definition_type compile(givm::definition_compile_context&) const { return {}; }
        static givm::summon_state query(const definition_type&, const givm::summon_state_limit&)
        {
            return { 20, 5 };
        }
    };

    struct batch_driver_source
    {
        static constexpr auto category = givm::definition_category::character;
        struct definition_type
        {
            batch_log* log;
            givm::optional_definition_id<givm::definition_category::summon> ordinary;
            givm::normal_effect setup;
            givm::normal_effect action;
            givm::normal_effect nested;
            givm::normal_effect restore;
        };
        batch_log* log;
        std::string_view name() const { return "SummonBatchDriver"; }
        auto summon_dependencies() const
        {
            return std::array<std::string_view, 2>{ "BatchOrdinary", "BatchPersistent" };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto ordinary = context.resolve_id<givm::definition_category::summon>("BatchOrdinary");
            const auto persistent = context.resolve_id<givm::definition_category::summon>("BatchPersistent");
            std::vector<givm::add_summon> setup;
            for(std::uint32_t index = 0; index != 8; ++index)
                setup.push_back({ index < 6 ? givm::relative_player::self : givm::relative_player::opponent,
                    index == 1 || index == 7 ? persistent : ordinary,
                    { index + 1, index == 1 || index == 7 ? 5u : 1u } });
            const auto action = log->set_then_remove
                ? context.add_normal_effect(std::tuple{ givm::set_summon_state{}, givm::remove_summon{} })
                : context.add_normal_effect(std::tuple{ givm::modify_summon_state{} });
            const auto nested = context.add_normal_effect(std::tuple{ givm::remove_summon{},
                givm::add_summon{ givm::relative_player::self, ordinary, { 19, 2 } } });
            return { log, ordinary, context.add_normal_effect(setup), action, nested,
                context.add_normal_effect(std::tuple{ givm::set_summon_state{} }) };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::round_started&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            if(data.log->phase++ == 0) return context.invoke(data.setup);
            std::vector<givm::set_summon_state_input::change> changes;
            for(const auto player : { givm::player_id{ 0 }, givm::player_id{ 1 } })
                for(const auto summon : context.table()[player].summons())
                {
                    data.log->original.push_back(summon.id());
                    if(summon.definition_id() == data.ordinary)
                        changes.push_back({ summon.id(), { summon.state().value, 0 } });
                }
            const std::span<const givm::summon_id> targets = data.log->empty
                ? std::span<const givm::summon_id>{} : std::span<const givm::summon_id>{ data.log->original };
            if(data.log->set_then_remove)
            {
                if(data.log->empty) changes.clear();
                return context.invoke(data.action, givm::set_summon_state_input{ changes },
                    givm::remove_summon_input{ targets });
            }
            return context.invoke(data.action, givm::modify_summon_state_input{ targets, 0, -1 });
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::summon_removed& event, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            CHECK_FALSE(context.table()[event.summon].is_valid());
            data.log->removed.push_back(event.summon);
            data.log->values_at_removal.push_back(context.table()[event.summon].state().value);
            std::vector<std::uint32_t> usages;
            for(const auto id : data.log->original) usages.push_back(context.table()[id].state().usages);
            data.log->usages_at_removal.push_back(usages);
            if(data.log->restore_later && not data.log->nested_started)
            {
                data.log->nested_started = true;
                const std::array changes{
                    givm::set_summon_state_input::change{ data.log->original[1], { 2, 2 } } };
                return context.invoke(data.restore, givm::set_summon_state_input{ changes });
            }
            if(data.log->nested && not data.log->nested_started)
            {
                data.log->nested_started = true;
                const std::array target{ data.log->original[2] };
                const auto targets = context.table()[target[0]].is_valid() ? std::span{ target } : std::span<const givm::summon_id>{};
                return context.invoke(data.nested, givm::remove_summon_input{ targets });
            }
            return {};
        }
    };

    givm::table run_batch(batch_log& log, givm::compile_mode mode)
    {
        const batch_summon_source ordinary{ true };
        const batch_summon_source persistent{ false };
        const auto driver = givm::test::with_passive_skill(batch_driver_source{ &log });
        const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
            std::tuple{ givm::start_round{}, givm::settle{}, givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } },
            std::tuple{}, ordinary, persistent, driver);
        const auto ordinary_id = ids.get_id<givm::definition_category::summon>(ordinary.name());
        const auto persistent_id = ids.get_id<givm::definition_category::summon>(persistent.name());
        CHECK(library.remove_at_zero_usages(ordinary_id));
        CHECK_FALSE(library.remove_at_zero_usages(persistent_id));
        const auto copied_library = library;
        CHECK(copied_library.remove_at_zero_usages(ordinary_id));
        CHECK_FALSE(copied_library.remove_at_zero_usages(persistent_id));
        givm::table table{ { .self_player = givm::player_id{ 0 } },
            { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 }, .summon_limit = 8 },
            { .summon_limit = 8 } };
        load_deck(table, library, { .characters = { ids.get_id<givm::definition_category::character>(driver.name()) } }, {});
        givm_test::executor_driver executor;
        executor.start(library, table);
        auto random = [] { return std::uint32_t{ 0 }; };
        REQUIRE(executor.advance(library, table, random) == givm::execution_state::finished);
        CHECK(log.phase == 2);
        return table;
    }
}

TEST_CASE("batch summon changes complete before the first removal notification", "[summon][batch]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    batch_log log{ .set_then_remove = GENERATE(false, true), .nested = GENERATE(false, true) };
    const auto table = run_batch(log, mode);
    REQUIRE(log.original.size() == 8);
    std::vector<givm::summon_id> expected;
    for(std::size_t index = 0; index != log.original.size(); ++index)
        if(log.set_then_remove || (index != 1 && index != 7)) expected.push_back(log.original[index]);
    REQUIRE(log.removed == expected);
    const auto persistent_usages = log.set_then_remove ? 5u : 4u;
    const std::vector<std::uint32_t> expected_usages{ 0, persistent_usages, 0, 0, 0, 0, 0, persistent_usages };
    CHECK(log.usages_at_removal == std::vector(log.removed.size(), expected_usages));
    for(std::size_t index = 0; index != log.removed.size(); ++index)
    {
        const auto original = std::ranges::find_if(log.original,
            [&](const auto id) { return id == log.removed[index]; }) - log.original.begin();
        CHECK(log.values_at_removal[index] == original + 1);
    }
    std::vector<givm::summon_id> survivors;
    for(const auto player : { givm::player_id{ 0 }, givm::player_id{ 1 } })
        for(const auto summon : table[player].summons()) survivors.push_back(summon.id());
    CHECK(survivors.size() == (log.set_then_remove ? 0 : 2) + (log.nested ? 1 : 0));
    if(log.nested)
    {
        const auto created = std::ranges::find_if(survivors, [&](const auto id)
        {
            return std::ranges::find_if(log.original,
                [&](const auto original) { return original == id; }) == log.original.end();
        });
        REQUIRE(created != survivors.end());
        CHECK(table[*created].state().value == 19);
        CHECK(table[*created].state().usages == 2);
    }
}

TEST_CASE("empty summon batches leave every entity unchanged", "[summon][batch]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    batch_log log{ .set_then_remove = GENERATE(false, true), .empty = true };
    const auto table = run_batch(log, mode);
    REQUIRE(log.original.size() == 8);
    CHECK(log.removed.empty());
    for(std::size_t index = 0; index != log.original.size(); ++index)
    {
        REQUIRE(table[log.original[index]].is_valid());
        CHECK(table[log.original[index]].state().value == index + 1);
        CHECK(table[log.original[index]].state().usages == (index == 1 || index == 7 ? 5 : 1));
    }
}

TEST_CASE("batch exhaustion invalidates all exhausted summons before removal responses", "[summon][batch]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    batch_log log{ .restore_later = true };
    const auto table = run_batch(log, mode);
    REQUIRE(log.original.size() == 8);
    CHECK(log.removed == std::vector{ log.original[0], log.original[2], log.original[3], log.original[4],
        log.original[5], log.original[6] });
    CHECK_FALSE(table[log.original[2]].is_valid());
    REQUIRE(table[log.original[1]].is_valid());
    CHECK(table[log.original[1]].state().usages == 2);
    REQUIRE(log.usages_at_removal.size() == 6);
    CHECK(log.usages_at_removal[0][1] == 4);
    CHECK(log.usages_at_removal[1][1] == 2);
}
}
