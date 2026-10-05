#include <array>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"
#include "../test_character_source.hpp"

namespace givm_test::executor_instructions::dying
{
namespace
{
    constexpr givm::character_id attacker{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id victim{ givm::player_id{ 1 }, 0 };

    struct dying_log
    {
        bool revive = false;
        bool pause = false;
        bool pause_defeat = false;
        std::vector<int> order;
        std::vector<givm::character_id> defeated;
        std::vector<std::uint32_t> after_health;
        std::size_t attachment_completions = 0;
    };

    struct revival_attachment
    {
        using definition_category = givm::attachment_view;
        struct definition_type { dying_log* log; givm::program_entry entry; };
        dying_log* log;
        std::string_view name() const { return "RevivalAttachment"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto entry = log->pause
                ? context.add_program(std::tuple{ givm::replace_cards{ givm::player_id{ 0 } }, givm::heal{} })
                : context.add_program(std::tuple{ givm::heal{} });
            return { log, entry };
        }
        static givm::program_entry handle(const definition_type& data,
            givm::character_will_be_defeated& event, givm::handle_context<givm::attachment_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity();
            CHECK(self.character().id() == event.target);
            CHECK(self.character().state().health == 0);
            CHECK(self.character().state().energy == 2);
            CHECK(self.is_valid());
            data.log->order.push_back(2);
            if(not data.log->revive) return {};
            return context.invoke(data.entry, givm::heal_input{ .source = self.id(), .target = event.target, .value = 2 });
        }
        static givm::program_entry handle(const definition_type& data,
            givm::after_damage&, givm::handle_context<givm::attachment_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity();
            ++data.log->attachment_completions;
            CHECK(self.character().state().health == 2);
            return {};
        }
        static givm::program_entry handle(const definition_type&,
            givm::character_defeated&, givm::handle_context<givm::attachment_view>&, std::uint32_t = 0)
        {
            FAIL("The defeated character's removed attachments must not receive the defeat notification");
            return {};
        }
    };

    struct dying_observer
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            dying_log* log;
            givm::program_entry attach;
            givm::definition_id<givm::attachment_view> attachment;
            givm::program_entry defeat;
        };
        dying_log* log;
        std::string_view name() const { return "DyingObserver"; }
        auto attachment_dependencies() const { return std::array{ std::string_view{ "RevivalAttachment" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_program(std::tuple{ givm::attach{} }),
                context.resolve_id<givm::attachment_view>("RevivalAttachment"),
                log->pause_defeat ? context.add_program(std::tuple{ givm::replace_cards{ givm::player_id{ 0 } } })
                    : givm::program_entry{} };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data,
            givm::round_started&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            return context.invoke(data.attach, givm::attach_input{
                .target = victim, .definition = data.attachment, .state = { 1 } });
        }
        static givm::program_entry handle(const definition_type& data,
            givm::character_will_be_defeated& event, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            CHECK(event.target == victim);
            const auto target = context.table()[event.target];
            CHECK(target.state().health == 0);
            CHECK(target.state().energy == 2);
            CHECK(std::ranges::distance(target.attachments()) == 1);
            data.log->order.push_back(1);
            return {};
        }
        static givm::program_entry handle(const definition_type& data,
            givm::character_defeated& event, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            CHECK(event.target == victim);
            const auto target = context.table()[event.target];
            CHECK(target.state().health == 0);
            CHECK(target.state().energy == 0);
            CHECK(target.state().aura == givm::element_aura::none);
            CHECK(target.attachments().empty());
            CHECK(data.log->after_health.empty());
            data.log->defeated.push_back(event.target);
            if(data.log->pause_defeat) return context.invoke(data.defeat);
            return {};
        }
        static givm::program_entry handle(const definition_type& data,
            givm::after_damage& event, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            data.log->after_health.push_back(context.table()[event.target].state().health);
            return {};
        }
    };

    struct zero_random { std::uint32_t operator()() const { return 0; } };
}

TEST_CASE("dying broadcasts allow the target's attachment to revive before defeat and cleanup", "[dying]")
{
    const bool observed = GENERATE(false, true);
    const bool revive = GENERATE(false, true);
    const bool has_reserve = GENERATE(false, true);
    dying_log log{ .revive = revive };
    const revival_attachment attachment{ &log };
    const auto observer = givm::test::with_passive_skill(dying_observer{ &log });
    const givm::test::initialized_character_source target{ "DyingTarget",
        { .max_health = 10, .max_energy = 3, .health = 1, .energy = 2 } };
    const std::array damages{ givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 },
        .value = 1, .type = givm::damage_type::physical } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::start_round{}, givm::deal_damage{ .damages = damages },
            givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, observer, target, attachment);
    const auto target_id = ids.get_id<givm::character_view>(target.name());
    givm::linked_deck defenders{ .characters = { target_id } };
    if(has_reserve) defenders.characters.push_back(target_id);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } }, defenders);
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    auto state = executor.advance(library, table, random);
    if(observed)
    {
        REQUIRE(state == givm::execution_state::health_reduced);
        CHECK(log.order.empty());
        CHECK(table[victim].state().health == 0);
        CHECK(table[victim].state().energy == 2);
        CHECK(std::ranges::distance(table[victim].attachments()) == 1);
        state = executor.advance(library, table, random);
    }
    REQUIRE(state == givm::execution_state::finished);
    CHECK(log.order == std::vector<int>{ 1, 2 });
    const bool terminated = not revive && not has_reserve;
    CHECK(executor.view_in<givm::execution_state::finished>().result()
        == (terminated ? givm::game_result::player_0_win : givm::game_result::both_loss));
    CHECK(table[victim].state().health == (revive ? 2 : 0));
    CHECK(table[victim].state().energy == (revive || terminated ? 2 : 0));
    CHECK(std::ranges::distance(table[victim].attachments()) == (revive || terminated ? 1 : 0));
    CHECK(log.attachment_completions == (revive ? 1 : 0));
    CHECK(log.defeated == (not revive && has_reserve ? std::vector{ victim } : std::vector<givm::character_id>{}));
    CHECK(log.after_health == (terminated ? std::vector<std::uint32_t>{} : std::vector<std::uint32_t>{ revive ? 2u : 0u }));
}

TEST_CASE("dying response inputs survive suspension and independent executor copies", "[dying][copy]")
{
    const bool observed = GENERATE(false, true);
    dying_log log{ .revive = true, .pause = true };
    const revival_attachment attachment{ &log };
    const auto observer = givm::test::with_passive_skill(dying_observer{ &log });
    const givm::test::initialized_character_source target{ "DyingTarget",
        { .max_health = 10, .max_energy = 3, .health = 1, .energy = 2 } };
    const std::array damages{ givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 },
        .value = 1, .type = givm::damage_type::physical } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::start_round{}, givm::deal_damage{ .damages = damages },
            givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, observer, target, attachment);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { ids.get_id<givm::character_view>(target.name()) } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    auto state = executor.advance(library, table, random);
    if(observed)
    {
        REQUIRE(state == givm::execution_state::health_reduced);
        state = executor.advance(library, table, random);
    }
    REQUIRE(state == givm::execution_state::card_selection);
    CHECK(log.order == std::vector<int>{ 1, 2 });
    CHECK(log.after_health.empty());
    CHECK(table[victim].state().health == 0);
    auto copied_executor = executor;
    auto copied_table = table;
    executor.submitted(executor.view_in<givm::execution_state::card_selection>().select(library, table, random, {}));
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::finished);
    CHECK(table[victim].state().health == 2);
    CHECK(copied_table[victim].state().health == 0);
    copied_executor.submitted(copied_executor.view_in<givm::execution_state::card_selection>().select(library, copied_table, random, {}));
    REQUIRE(copied_executor.advance(library, copied_table, random) == givm::execution_state::finished);
    CHECK(copied_table[victim].state().health == 2);
    CHECK(copied_table[victim].state().energy == 2);
    CHECK(log.order == std::vector<int>{ 1, 2 });
    CHECK(log.after_health == std::vector<std::uint32_t>{ 2, 2 });
    CHECK(log.attachment_completions == 2);
    CHECK(log.defeated.empty());
}

TEST_CASE("confirmed defeat notifications follow cleanup and resume before damage completion", "[dying][defeat][copy]")
{
    const bool observed = GENERATE(false, true);
    dying_log log{ .pause_defeat = true };
    const revival_attachment attachment{ &log };
    const auto observer = givm::test::with_passive_skill(dying_observer{ &log });
    const givm::test::initialized_character_source target{ "DyingTarget",
        { .max_health = 10, .max_energy = 3, .health = 1, .energy = 2, .aura = givm::element_aura::hydro } };
    const std::array damages{ givm::fixed_damage{
        .source = givm::relative_character_target{ givm::relative_player::self, 0 },
        .target = givm::relative_character_target{ givm::relative_player::opponent, 0 },
        .value = 1, .type = givm::damage_type::physical } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::start_round{}, givm::deal_damage{ .damages = damages },
            givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, observer, target, attachment);
    const auto target_id = ids.get_id<givm::character_view>(target.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = attacker }, { .active_character = victim } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { target_id, target_id } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    auto state = executor.advance(library, table, random);
    if(observed)
    {
        REQUIRE(state == givm::execution_state::health_reduced);
        CHECK(log.defeated.empty());
        CHECK(table[victim].state().aura == givm::element_aura::hydro);
        state = executor.advance(library, table, random);
    }
    REQUIRE(state == givm::execution_state::card_selection);
    CHECK(log.order == std::vector<int>{ 1, 2 });
    CHECK(log.defeated == std::vector{ victim });
    CHECK(log.after_health.empty());
    CHECK(table[victim].state().energy == 0);
    CHECK(table[victim].state().aura == givm::element_aura::none);
    CHECK(table[victim].attachments().empty());
    auto copied_executor = executor;
    auto copied_table = table;
    executor.submitted(executor.view_in<givm::execution_state::card_selection>().select(library, table, random, {}));
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::finished);
    CHECK(log.after_health == std::vector<std::uint32_t>{ 0 });
    copied_executor.submitted(copied_executor.view_in<givm::execution_state::card_selection>().select(library, copied_table, random, {}));
    REQUIRE(copied_executor.advance(library, copied_table, random) == givm::execution_state::finished);
    CHECK(log.after_health == std::vector<std::uint32_t>{ 0, 0 });
    CHECK(log.defeated == std::vector{ victim });
    CHECK(log.attachment_completions == 0);
}
}
