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

namespace
{
    constexpr givm::character_id attacker{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id victim{ givm::player_id{ 1 }, 0 };

    struct dying_log
    {
        bool revive = false;
        bool pause = false;
        std::vector<int> order;
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
        static givm::program_entry handle(const definition_type& data, const givm::attachment_view& self,
            givm::character_will_be_defeated& event, givm::handle_context& context)
        {
            CHECK(self.character().id() == event.target);
            CHECK(self.character().state().health == 0);
            CHECK(self.character().state().energy == 2);
            CHECK(self.is_valid());
            data.log->order.push_back(2);
            if(not data.log->revive) return {};
            return context.invoke(data.entry, givm::healing{ .source = self.id(), .target = event.target, .value = 2 });
        }
        static givm::program_entry handle(const definition_type& data, const givm::attachment_view& self,
            givm::after_damage&, givm::handle_context&)
        {
            ++data.log->attachment_completions;
            CHECK(self.character().state().health == 2);
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
        };
        dying_log* log;
        std::string_view name() const { return "DyingObserver"; }
        auto attachment_dependencies() const { return std::array{ std::string_view{ "RevivalAttachment" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_program(std::tuple{ givm::attach{} }),
                context.resolve_id<givm::attachment_view>("RevivalAttachment") };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::test_event&, givm::handle_context& context)
        {
            return context.invoke(data.attach, givm::attachment_application{
                .target = victim, .definition = data.attachment, .state = { 1 } });
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::character_will_be_defeated& event, givm::handle_context& context)
        {
            CHECK(event.target == victim);
            const auto target = context.table()[event.target];
            CHECK(target.state().health == 0);
            CHECK(target.state().energy == 2);
            CHECK(std::ranges::distance(target.attachments()) == 1);
            data.log->order.push_back(1);
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::after_damage& event, givm::handle_context& context)
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
    const std::array damages{ givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_damage_target{ givm::relative_player::opponent, 0 },
        .value = 1, .type = givm::damage_type::physical } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::test_command{}, givm::deal_damage{ .damages = damages },
            givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, observer, target, attachment);
    const auto target_id = ids.get_id<givm::character_view>(target.name());
    givm::linked_deck defenders{ .characters = { target_id } };
    if(has_reserve) defenders.characters.push_back(target_id);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } }, defenders);
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    auto state = executor.step(library, table, random);
    if(observed)
    {
        REQUIRE(state == givm::execution_state::health_reduced);
        CHECK(log.order.empty());
        CHECK(table[victim].state().health == 0);
        CHECK(table[victim].state().energy == 2);
        CHECK(std::ranges::distance(table[victim].attachments()) == 1);
        state = executor.step(library, table, random);
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
    const std::array damages{ givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_damage_target{ givm::relative_player::opponent, 0 },
        .value = 1, .type = givm::damage_type::physical } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::test_command{}, givm::deal_damage{ .damages = damages },
            givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, observer, target, attachment);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { ids.get_id<givm::character_view>(target.name()) } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    auto state = executor.step(library, table, random);
    if(observed)
    {
        REQUIRE(state == givm::execution_state::health_reduced);
        state = executor.step(library, table, random);
    }
    REQUIRE(state == givm::execution_state::card_selection);
    CHECK(log.order == std::vector<int>{ 1, 2 });
    CHECK(log.after_health.empty());
    CHECK(table[victim].state().health == 0);
    auto copied_executor = executor;
    auto copied_table = table;
    executor.view_in<givm::execution_state::card_selection>().select({});
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(table[victim].state().health == 2);
    CHECK(copied_table[victim].state().health == 0);
    copied_executor.view_in<givm::execution_state::card_selection>().select({});
    REQUIRE(copied_executor.step(library, copied_table, random) == givm::execution_state::finished);
    CHECK(copied_table[victim].state().health == 2);
    CHECK(copied_table[victim].state().energy == 2);
    CHECK(log.order == std::vector<int>{ 1, 2 });
    CHECK(log.after_health == std::vector<std::uint32_t>{ 2, 2 });
    CHECK(log.attachment_completions == 2);
}
