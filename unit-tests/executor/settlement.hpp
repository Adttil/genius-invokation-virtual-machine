#include <array>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../table/test_definition_library.hpp"
#include "test_character_source.hpp"

namespace givm_test::executor::settlement
{
namespace
{
    constexpr givm::character_id own{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id opponent{ givm::player_id{ 1 }, 0 };
    constexpr std::array own_targets{ own };

    enum class scenario { multiple, checkpoints, nested_pause, root_inline, forbid_segment, forbid_settle, reuse, truncate };
    struct log_state
    {
        std::vector<std::uint32_t> responses;
        std::vector<std::uint32_t> energy;
        std::vector<std::uint32_t> at_response;
    };

    struct skill_source
    {
        using definition_category = givm::skill_view;
        struct definition_type
        {
            scenario kind;
            log_state* log;
            givm::normal_effect main;
            givm::immediate_effect immediate;
            givm::normal_effect leaf;
            givm::normal_effect relay;
        };
        scenario kind;
        log_state* log;
        std::string_view name() const { return "SettlementSkill"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            using namespace givm;
            const auto leaf = kind == scenario::nested_pause
                ? context.add_normal_effect(replace_cards{ .player = player_id{ 0 } }, modify_energy{},
                    return_response{ .index = 5 })
                : context.add_normal_effect(modify_energy{}, return_response{ .index = 5 });
            const auto relay = context.add_normal_effect(defer_program{}, return_response{ .index = 27 });
            dice_counts dice;
            dice[elemental_dice::omni] = 1;
            const add_dice observe{ .player = relative_player::self, .dice = dice };
            normal_effect main;
            immediate_effect immediate;
            switch(kind)
            {
            case scenario::checkpoints:
                main = context.add_normal_effect(modify_energy{}, defer_program{}, end_segment{}, modify_energy{},
                    settle{}, observe, modify_energy{}, defer_program{ fixed_defer_invoke(leaf, modify_energy_input{ own_targets, 16 }) },
                    settle{}, observe, return_response{});
                break;
            case scenario::nested_pause:
            {
                const auto tail = context.add_normal_effect(
                    modify_energy{ .target = { relative_player::self, 0 }, .delta = -4 });
                main = context.add_normal_effect(end_segment{}, settle{}, settle{},
                    defer_program{}, end_segment{}, end_segment{},
                    defer_program{ fixed_defer_invoke(tail) }, end_segment{}, settle{},
                    modify_energy{ .target = { relative_player::self, 0 }, .delta = 16 },
                    defer_program{ fixed_defer_invoke(tail) }, settle{}, end_segment{}, settle{}, return_response{});
                break;
            }
            case scenario::forbid_segment:
                immediate = context.add_immediate_effect(end_segment{});
                break;
            case scenario::forbid_settle:
                immediate = context.add_immediate_effect(settle{});
                break;
            case scenario::truncate:
                main = context.add_normal_effect(return_response{ .index = 9 },
                    set_energy{ .target = { static_cast<relative_player>(99), 0 } }, defer_program{});
                break;
            default:
                main = context.add_normal_effect(defer_program{}, return_response{});
                break;
            }
            if(kind == scenario::root_inline) immediate = context.add_immediate_effect(defer_program{}, return_response{});
            return { kind, log, main, immediate, leaf, relay };
        }

        static givm::normal_effect handle(const definition_type& data, givm::round_started&,
            givm::handle_context<givm::skill_view>& context, std::uint32_t index = 0)
        {
            using namespace givm;
            data.log->responses.push_back(index);
            data.log->at_response.push_back(context.table()[own].state().energy);
            CHECK(&context.entity().table() == &context.table());
            if(data.kind == scenario::truncate)
                return index == 0 ? context.invoke(data.main) : normal_effect{};
            if(data.kind == scenario::reuse && index == 0)
                return context.invoke(data.leaf, modify_energy_input{ own_targets, 1 });
            if(index != (data.kind == scenario::reuse ? 5u : 0u)) return {};

            const auto first = defer_invoke(data.leaf, modify_energy_input{ own_targets, 2 });
            if(data.kind == scenario::checkpoints)
            {
                return context.invoke(data.main,
                    modify_energy_input{ own_targets, 1 }, first,
                    modify_energy_input{ own_targets, 4 }, modify_energy_input{ own_targets, 8 },
                    return_response_input{ 6 });
            }
            if(data.kind == scenario::nested_pause)
            {
                const std::vector<character_id> targets(4096, own);
                return context.invoke(data.main,
                    defer_invoke(data.relay, defer_invoke(data.leaf, modify_energy_input{ targets, 1 })),
                    return_response_input{ 4 });
            }
            return context.invoke(data.main, first,
                return_response_input{ data.kind == scenario::reuse ? 10u : 4u });
        }

        static givm::immediate_effect handle(const definition_type& data, givm::damage_calculation&,
            givm::handle_context<givm::skill_view, givm::event_category::immediate>& context, std::uint32_t = 0)
        {
            using namespace givm;
            if(data.kind == scenario::forbid_segment || data.kind == scenario::forbid_settle)
                return context.invoke(data.immediate);
            if(data.kind != scenario::root_inline) return {};
            return context.invoke(data.immediate, defer_invoke(data.leaf, modify_energy_input{ own_targets, -1 }),
                return_response_input{});
        }

        static givm::normal_effect handle(const definition_type& data, givm::dice_added& event,
            givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            if(event.player == own.player_id) data.log->energy.push_back(context.table()[own].state().energy);
            return {};
        }

        static givm::normal_effect handle(const definition_type& data, givm::round_end_declared&,
            givm::handle_context<givm::skill_view>& context, std::uint32_t index = 0)
        {
            if(data.kind != scenario::root_inline) return {};
            data.log->at_response.push_back(context.table()[own].state().energy);
            if(index != 0) return {};
            return context.invoke(data.main, givm::defer_invoke(data.leaf, givm::modify_energy_input{ own_targets, 0 }),
                givm::return_response_input{ 7 });
        }
    };

    struct character_source
    {
        using definition_category = givm::character_view;
        struct definition_type { givm::definition_id<givm::skill_view> skill; };
        std::string_view name() const { return "SettlementCharacter"; }
        auto skill_dependencies() const { return std::array{ std::string_view{ "SettlementSkill" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_id<givm::skill_view>("SettlementSkill") };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .max_energy = 10000, .health = 10 };
        }
        static givm::definition_id<givm::skill_view> query(const definition_type& data, const givm::character_initial_skill& query)
        {
            return query.skill_index == 0 ? data.skill : givm::definition_id<givm::skill_view>{};
        }
    };

    givm::table make_table(const givm::definition_library& library, const givm::issued_id_map& ids)
    {
        givm::table result{ { .self_player = givm::player_id{ 0 } },
            { .active_character = own }, {} };
        const auto id = ids.get_id<givm::character_view>("SettlementCharacter");
        load_deck(result, library, { .characters = { id } }, {});
        return result;
    }
}

TEST_CASE("response continuation waits for deferred programs and ignores their return values", "[settlement]")
{
    log_state log;
    skill_source skill{ scenario::multiple, &log };
    character_source character;
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, skill, character);
    auto table = make_table(library, ids);
    givm::executor execution;
    REQUIRE(execution.start(library, table).resume(library, table, givm_test::zero_random) == givm::execution_state::finished);
    CHECK(log.responses == std::vector<std::uint32_t>{ 0, 4 });
    CHECK(log.at_response == std::vector<std::uint32_t>{ 0, 2 });
    CHECK(table[own].state().energy == 2);
}

TEST_CASE("explicit settlement preserves later inputs and separates multiple batches", "[settlement]")
{
    log_state log;
    skill_source skill{ scenario::checkpoints, &log };
    character_source character;
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, skill, character);
    auto table = make_table(library, ids);
    givm::executor execution;
    REQUIRE(execution.start(library, table).resume(library, table, givm_test::zero_random) == givm::execution_state::finished);
    CHECK(log.responses == std::vector<std::uint32_t>{ 0, 6 });
    CHECK(log.energy == std::vector<std::uint32_t>{ 15, 31 });
    CHECK(table[own].state().energy == 31);
}

TEST_CASE("nested deferred scopes restore parent segments and rebuild after copied pauses", "[settlement]")
{
    log_state log;
    skill_source skill{ scenario::nested_pause, &log };
    character_source character;
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, skill, character);
    auto table = make_table(library, ids);
    givm::executor execution;
    REQUIRE(execution.start(library, table).resume(library, table, givm_test::zero_random) == givm::execution_state::card_selection);
    CHECK(table[own].state().energy == 0);
    CHECK(log.responses == std::vector<std::uint32_t>{ 0 });
    auto branch = execution;
    auto branch_table = table;
    REQUIRE(execution.view_in<givm::execution_state::card_selection>().select(library, table, givm_test::zero_random, {})
        == givm::execution_state::finished);
    REQUIRE(branch.view_in<givm::execution_state::card_selection>().select(library, branch_table, givm_test::zero_random, {})
        == givm::execution_state::finished);
    // Both queued segments finish in order, then the parent continues after settle
    // and appends another child to the rebuilt domain: 4096 - 4 + 16 - 4.
    CHECK(table[own].state().energy == 4104);
    CHECK(branch_table[own].state().energy == 4104);
    CHECK(log.responses == std::vector<std::uint32_t>{ 0, 4, 4 });
    CHECK(log.at_response == std::vector<std::uint32_t>{ 0, 4104, 4104 });
}

TEST_CASE("starting an executor replaces paused domains without changing an existing copy", "[settlement]")
{
    log_state suspended_log;
    skill_source suspended_skill{ scenario::nested_pause, &suspended_log };
    character_source character;
    const auto [suspended_library, suspended_ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
        suspended_skill, character);
    auto suspended_table = make_table(suspended_library, suspended_ids);
    givm::executor execution;
    REQUIRE(execution.start(suspended_library, suspended_table).resume(
        suspended_library, suspended_table, givm_test::zero_random) == givm::execution_state::card_selection);
    auto preserved = execution;
    auto preserved_table = suspended_table;

    log_state restarted_log;
    skill_source restarted_skill{ scenario::multiple, &restarted_log };
    const auto [restarted_library, restarted_ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::settle{}, givm::end_segment{}, givm::settle{}, givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, restarted_skill, character);
    auto restarted_table = make_table(restarted_library, restarted_ids);
    REQUIRE(execution.start(restarted_library, restarted_table).resume(
        restarted_library, restarted_table, givm_test::zero_random) == givm::execution_state::finished);
    CHECK(restarted_table[own].state().energy == 2);
    CHECK(restarted_log.responses == std::vector<std::uint32_t>{ 0, 4 });
    CHECK(suspended_table[own].state().energy == 0);
    CHECK(suspended_log.responses == std::vector<std::uint32_t>{ 0 });

    REQUIRE(preserved.view_in<givm::execution_state::card_selection>().select(
        suspended_library, preserved_table, givm_test::zero_random, {}) == givm::execution_state::finished);
    CHECK(preserved_table[own].state().energy == 4104);
    CHECK(suspended_log.responses == std::vector<std::uint32_t>{ 0, 4 });
    CHECK(restarted_table[own].state().energy == 2);
    CHECK(restarted_log.responses == std::vector<std::uint32_t>{ 0, 4 });
}

TEST_CASE("ordinary and deferred execution share a compiled entry but not response continuation", "[settlement]")
{
    log_state log;
    skill_source skill{ scenario::reuse, &log };
    character_source character;
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, skill, character);
    auto table = make_table(library, ids);
    givm::executor execution;
    REQUIRE(execution.start(library, table).resume(library, table, givm_test::zero_random) == givm::execution_state::finished);
    CHECK(log.responses == std::vector<std::uint32_t>{ 0, 5, 10 });
    CHECK(log.at_response == std::vector<std::uint32_t>{ 0, 1, 3 });
}

TEST_CASE("return truncates later invalid commands and dynamic input requirements", "[settlement]")
{
    log_state log;
    skill_source skill{ scenario::truncate, &log };
    character_source character;
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, skill, character);
    auto table = make_table(library, ids);
    givm::executor execution;
    REQUIRE(execution.start(library, table).resume(library, table, givm_test::zero_random) == givm::execution_state::finished);
    CHECK(log.responses == std::vector<std::uint32_t>{ 0, 9 });
    CHECK(table[own].state().energy == 0);
}

TEST_CASE("root explicit settlement controls deferred inline work across segment ends", "[settlement]")
{
    const bool settle_before_increment = GENERATE(false, true);
    log_state log;
    skill_source skill{ scenario::root_inline, &log };
    character_source character;
    givm::test::initialized_character_source victim{ .source_name = "Victim" };
    const std::array damages{ givm::deal_damage{
        .source = { givm::relative_player::self, 0 }, .target = { givm::relative_player::opponent, 0 },
        .value = 1, .type = givm::damage_type::physical } };
    std::vector<givm::any_command> commands{
        damages[0], givm::end_segment{} };
    if(settle_before_increment) commands.emplace_back(givm::settle{});
    commands.emplace_back(givm::modify_energy{ .target = { givm::relative_player::self, 0 }, .delta = 1 });
    commands.emplace_back(givm::end_segment{});
    commands.emplace_back(givm::settle{});
    commands.emplace_back(givm::settle{});
    commands.emplace_back(givm::end_game{ givm::game_result::both_loss });
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        commands, std::tuple{}, skill, character, victim);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = own }, { .active_character = opponent } };
    load_deck(table, library,
        { .characters = { ids.get_id<givm::character_view>(character.name()) } },
        { .characters = { ids.get_id<givm::character_view>(victim.name()) } });
    givm::executor execution;
    REQUIRE(execution.start(library, table).resume(library, table, givm_test::zero_random) == givm::execution_state::finished);
    CHECK(table[own].state().energy == (settle_before_increment ? 1u : 0u));
    CHECK(table[opponent].state().health == 9);
}

TEST_CASE("root deferred records survive action windows independent settlements and executor copies", "[settlement]")
{
    log_state log;
    skill_source skill{ scenario::root_inline, &log };
    character_source character;
    givm::test::initialized_character_source victim{ .source_name = "Victim" };
    const std::array damages{ givm::deal_damage{
        .source = { givm::relative_player::self, 0 }, .target = { givm::relative_player::opponent, 0 },
        .value = 1, .type = givm::damage_type::physical } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{
            givm::set_energy{ .target = { givm::relative_player::self, 0 }, .value = 1 },
            damages[0], givm::end_segment{}, givm::begin_action{},
            givm::replace_cards{ .player = givm::player_id{ 0 } }, givm::settle{}, givm::end_game{ givm::game_result::both_loss }
        }, std::tuple{}, skill, character, victim);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = own }, { .active_character = opponent } };
    load_deck(table, library,
        { .characters = { ids.get_id<givm::character_view>(character.name()) } },
        { .characters = { ids.get_id<givm::character_view>(victim.name()) } });
    givm::executor execution;
    REQUIRE(execution.start(library, table).resume(library, table, givm_test::zero_random)
        == givm::execution_state::action_selection);
    CHECK(table[own].state().energy == 1);
    CHECK(table[opponent].state().health == 9);
    CHECK(log.at_response.empty());
    auto copied = execution;
    auto copied_table = table;

    const auto finish = [&](givm::executor& current, givm::table& current_table)
    {
        log.at_response.clear();
        REQUIRE(current.view_in<givm::execution_state::action_selection>().declare_round_end(
            library, current_table, givm_test::zero_random) == givm::execution_state::action_selection);
        CHECK(current_table[own].state().energy == 1);
        CHECK(log.at_response == std::vector<std::uint32_t>{ 1, 1 });
        REQUIRE(current.view_in<givm::execution_state::action_selection>().declare_round_end(
            library, current_table, givm_test::zero_random) == givm::execution_state::card_selection);
        // This root pause is after begin_action has returned and before its explicit settle.
        CHECK(current_table[own].state().energy == 1);
        CHECK(log.at_response == std::vector<std::uint32_t>{ 1, 1, 1, 1 });
        REQUIRE(current.view_in<givm::execution_state::card_selection>().select(
            library, current_table, givm_test::zero_random, {}) == givm::execution_state::finished);
        CHECK(current_table[own].state().energy == 0);
        CHECK(current_table[opponent].state().health == 9);
        CHECK(log.at_response == std::vector<std::uint32_t>{ 1, 1, 1, 1 });
    };
    finish(execution, table);
    finish(copied, copied_table);
}

TEST_CASE("immediate effects reject segment ends and settlement during compilation", "[settlement][effect][compile]")
{
    const auto kind = GENERATE(scenario::forbid_segment, scenario::forbid_settle);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    log_state log;
    skill_source skill{ kind, &log };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(skill));
    const auto result = givm::compile(sources, givm_test::basic_sources, std::tuple{}, std::tuple{}, mode);
    REQUIRE_FALSE(result);
    REQUIRE(result.error().size() == 1);
    const auto* error = std::get_if<givm::effect_command_not_allowed>(&result.error().front().reason);
    REQUIRE(error);
    CHECK(error->category == givm::event_category::immediate);
    CHECK(std::string{ error->command } == (kind == scenario::forbid_segment ? "end_segment" : "settle"));
}
}
