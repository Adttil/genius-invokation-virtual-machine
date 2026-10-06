#include <array>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../table/test_definition_library.hpp"
#include "test_character_source.hpp"

namespace givm_test::executor::settlement_single
{
namespace
{
    constexpr givm::player_id owner{ 0 };
    constexpr givm::character_id actor{ owner, 0 };
    constexpr givm::character_id opponent{ givm::player_id{ 1 }, 0 };

    struct call_log
    {
        bool prepared = false;
        bool remove = false;
        bool switch_during_effect = false;
        std::vector<std::uint32_t> indices;
        std::vector<std::uint32_t> energy;
    };

    struct summon_source
    {
        using definition_category = givm::summon_view;
        struct definition_type { call_log* log; givm::program_entry body; givm::program_entry removal; };
        call_log* log;
        std::string_view name() const { return "SingleSettlementSummon"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            using namespace givm;
            const auto delayed = context.add_program(modify_energy{ .target = { relative_player::self, 0 }, .delta = 2 },
                return_response{ .index = 15 });
            return { log,
                context.add_program(modify_energy{}, defer_program{ fixed_defer_invoke(delayed) }, return_response{ .index = 7 }),
                context.add_program(remove_summon{}, return_response{ .index = 7 }) };
        }
        static givm::summon_state query(const definition_type&, const givm::summon_state_limit&) { return { 20, 20 }; }
        static givm::program_entry handle(const definition_type& data, givm::resummoning& event,
            givm::handle_context<givm::summon_view>& context, std::uint32_t index)
        {
            using namespace givm;
            data.log->indices.push_back(index);
            data.log->energy.push_back(context.table()[actor].state().energy);
            CHECK(event.state.value == 7);
            CHECK(event.state.usages == 3);
            if(index != 0) return {};
            if(data.log->remove)
            {
                const std::array ids{ context.entity().id() };
                return context.invoke(data.removal, remove_summon_input{ ids });
            }
            const std::vector<character_id> targets(4096, actor);
            return context.invoke(data.body, modify_energy_input{ targets, 1 });
        }
    };

    struct prepared_source
    {
        using definition_category = givm::attachment_view;
        struct definition_type { call_log* log; givm::program_entry body; };
        call_log* log;
        std::string_view name() const { return "SingleSettlementPreparation"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            if(log->switch_during_effect)
                return { log, context.add_program(givm::modify_energy{},
                    givm::set_active_character{ .target = { givm::relative_player::self, 1 } },
                    givm::return_response{ .index = 7 }) };
            return { log, context.add_program(givm::modify_energy{}, givm::return_response{ .index = 7 }) };
        }
        static givm::program_entry handle(const definition_type& data, givm::prepared_skill_effect& event,
            givm::handle_context<givm::attachment_view>& context, std::uint32_t index)
        {
            using namespace givm;
            CHECK_FALSE(context.entity().is_valid());
            CHECK(event.attachment == context.entity().id());
            data.log->indices.push_back(index);
            data.log->energy.push_back(context.table()[actor].state().energy);
            if(index != 0)
            {
                CHECK(event.speed == (data.log->switch_during_effect ? action_speed::combat : action_speed::fast));
                return {};
            }
            const std::vector<character_id> targets(4096, actor);
            event.speed = data.log->switch_during_effect ? action_speed::combat : action_speed::fast;
            return context.invoke(data.body, modify_energy_input{ targets, 1 });
        }
    };

    struct driver_source
    {
        using definition_category = givm::character_view;
        struct definition_type { givm::program_entry setup; };
        call_log* log;
        std::string_view name() const { return "SingleSettlementDriver"; }
        auto summon_dependencies() const { return std::array{ std::string_view{ "SingleSettlementSummon" } }; }
        auto attachment_dependencies() const { return std::array{ std::string_view{ "SingleSettlementPreparation" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            using namespace givm;
            if(log->prepared)
                return { context.add_program(add_attachment{
                    .definition = context.resolve_id<attachment_view>("SingleSettlementPreparation") }) };
            const auto definition = context.resolve_id<summon_view>("SingleSettlementSummon");
            return { context.add_program(add_summon{ .definition = definition, .state = { 1, 1 } },
                summon{ .definition = definition, .state = { 7, 3 } }) };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .max_energy = 10000, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data, givm::round_started&,
            givm::handle_context<givm::skill_view>& context, std::uint32_t)
        {
            return context.invoke(data.setup);
        }
    };

    givm::table make_table(const givm::definition_library& library, const givm::issued_id_map& ids)
    {
        givm::table table{ { .self_player = owner },
            { .active_character = actor, .can_plunge = true }, { .active_character = opponent } };
        load_deck(table, library,
            { .characters = { ids.get_id<givm::character_view>("SingleSettlementDriver") } },
            { .characters = { ids.get_id<givm::character_view>("SingleSettlementOpponent") } });
        return table;
    }
}

TEST_CASE("single response retains its original event and settles before continuation", "[settlement][single-response]")
{
    call_log log;
    log.remove = GENERATE(false, true);
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
        givm::test::with_passive_skill(driver_source{ &log }), summon_source{ &log }, prepared_source{ &log },
        givm::test::initialized_character_source{ "SingleSettlementOpponent" });
    auto table = make_table(library, ids);
    givm::executor execution;
    REQUIRE(execution.start(library, table).resume(library, table, givm_test::zero_random) == givm::execution_state::finished);
    if(log.remove)
    {
        CHECK(log.indices == std::vector<std::uint32_t>{ 0 });
        CHECK(std::ranges::empty(table[owner].summons()));
    }
    else
    {
        CHECK(log.indices == std::vector<std::uint32_t>{ 0, 7 });
        CHECK(log.energy == std::vector<std::uint32_t>{ 0, 4098 });
    }
}

TEST_CASE("consumed prepared effect continues and preserves its selected speed", "[settlement][single-response]")
{
    call_log log{ .prepared = true };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::begin_action{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
        givm::test::with_passive_skill(driver_source{ &log }), summon_source{ &log }, prepared_source{ &log },
        givm::test::initialized_character_source{ "SingleSettlementOpponent" });
    auto table = make_table(library, ids);
    givm::executor execution;
    REQUIRE(execution.start(library, table).resume(library, table, givm_test::zero_random) == givm::execution_state::action_selection);
    CHECK(log.indices == std::vector<std::uint32_t>{ 0, 7 });
    CHECK(log.energy == std::vector<std::uint32_t>{ 0, 4096 });
    CHECK(table.state().active_player == owner);
    CHECK(table[owner].state().can_plunge);
}

TEST_CASE("prepared combat effect keeps plunge eligibility gained by its character switch", "[settlement][single-response]")
{
    call_log log{ .prepared = true, .switch_during_effect = true };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::begin_action{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
        givm::test::with_passive_skill(driver_source{ &log }), summon_source{ &log }, prepared_source{ &log },
        givm::test::initialized_character_source{ "SingleSettlementOpponent" });
    givm::table table{ { .self_player = owner },
        { .active_character = actor, .can_plunge = true }, { .active_character = opponent } };
    const auto ordinary = ids.get_id<givm::character_view>("SingleSettlementOpponent");
    load_deck(table, library,
        { .characters = { ids.get_id<givm::character_view>("SingleSettlementDriver"), ordinary } },
        { .characters = { ordinary } });
    givm::executor execution;
    REQUIRE(execution.start(library, table).resume(library, table, givm_test::zero_random) == givm::execution_state::action_selection);
    CHECK(log.indices == std::vector<std::uint32_t>{ 0, 7 });
    CHECK(log.energy == std::vector<std::uint32_t>{ 0, 4096 });
    CHECK(table[owner].state().active_character == givm::character_id{ owner, 1 });
    CHECK(table.state().active_player == opponent.player_id);
    CHECK(table[owner].state().can_plunge);
}
}
