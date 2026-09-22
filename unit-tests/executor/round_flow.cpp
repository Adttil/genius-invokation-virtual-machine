#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/executor.hpp>

#include "../table/test_definition_library.hpp"

namespace
{
    struct round_log
    {
        std::vector<std::uint32_t> preparations;
        std::vector<std::array<std::uint32_t, 2>> dice_before_roll;
        std::vector<std::uint32_t> notifications;
        std::vector<std::array<std::uint32_t, 2>> dice_at_notification;
    };
    struct round_observer
    {
        using definition_category = givm::character_view;
        struct definition_type { round_log* log; };
        round_log* log;
        std::string_view name() const { return "RoundObserver"; }
        definition_type compile(givm::definition_compile_context&) const { return { log }; }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::dice_roll_preparation&, givm::handle_context& context)
        {
            data.log->preparations.push_back(context.table().state().round_number);
            data.log->dice_before_roll.push_back({ context.table()[givm::player_id{ 0 }].state().dice.total(),
                context.table()[givm::player_id{ 1 }].state().dice.total() });
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::round_started&, givm::handle_context& context)
        {
            data.log->notifications.push_back(context.table().state().round_number);
            data.log->dice_at_notification.push_back({ context.table()[givm::player_id{ 0 }].state().dice.total(),
                context.table()[givm::player_id{ 1 }].state().dice.total() });
            return {};
        }
    };
    struct counting_random
    {
        std::size_t calls = 0;
        std::uint32_t operator()() { ++calls; return 0; }
    };
}

TEST_CASE("automatic round advancement clears dice before rolling and checks the limit before the next body", "[round][executor]")
{
    const bool observed = GENERATE(false, true);
    round_log log;
    const auto observer = givm::test::with_passive_skill(round_observer{ &log });
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::start_dice_roll_phase{ .count = 3, .reroll_count = { 0, 0 } } },
        std::tuple{ givm::start_dice_roll_phase{ .count = 2, .reroll_count = { 0, 0 } },
            givm::start_round{}, givm::replace_cards{ givm::player_id{ 0 } } }, observer);
    givm::table table{ givm::table_state{ .max_rounds = 2 } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } }, {});
    givm::executor executor;
    executor.enter_entry(library);
    counting_random random;
    const auto first_state = executor.step(library, table, random);
    REQUIRE(first_state == (observed ? givm::execution_state::round_started : givm::execution_state::card_selection));
    CHECK(table.state().round_number == 1);
    if(observed)
    {
        CHECK(log.notifications.empty());
        CHECK(log.preparations == std::vector<std::uint32_t>{ 0 });
        CHECK(random.calls == 6);
        for(const auto player : table.players()) CHECK(player.state().dice.total() == 3);
    }
    else
    {
        CHECK(log.notifications == std::vector<std::uint32_t>{ 1 });
        CHECK(random.calls == 10);
    }
    auto copied_executor = executor;
    auto copied_table = table;
    const auto saved_log = log;
    const auto saved_random = random;
    const auto finish = [&](givm::executor& running, givm::table& current)
    {
        log = saved_log;
        random = saved_random;
        auto state = first_state;
        std::size_t observations = 0;
        std::size_t inputs = 0;
        while(state != givm::execution_state::finished)
        {
            if(state == givm::execution_state::round_started)
            {
                REQUIRE(observed);
                ++observations;
                const auto round = current.state().round_number;
                CHECK(round == observations);
                CHECK(log.preparations.size() == round);
                CHECK(log.notifications.size() == round - 1);
                for(const auto player : current.players()) CHECK(player.state().dice.total() == (round == 1 ? 3 : 2));
            }
            else
            {
                REQUIRE(state == givm::execution_state::card_selection);
                ++inputs;
                CHECK(current.state().round_number == inputs);
                CHECK(log.notifications.size() == inputs);
                for(const auto player : current.players()) CHECK(player.state().dice.total() == 2);
                running.view_in<givm::execution_state::card_selection>().select({});
            }
            state = running.step(library, current, random);
        }
        CHECK(observations == (observed ? 3 : 0));
        CHECK(inputs == 2);
        CHECK(current.state().round_number == 3);
        CHECK(running.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
        CHECK(log.preparations == std::vector<std::uint32_t>{ 0, 1, 2 });
        CHECK(log.dice_before_roll == std::vector<std::array<std::uint32_t, 2>>{ { 0, 0 }, { 0, 0 }, { 0, 0 } });
        CHECK(log.notifications == std::vector<std::uint32_t>{ 1, 2 });
        CHECK(log.dice_at_notification == std::vector<std::array<std::uint32_t, 2>>{ { 2, 2 }, { 2, 2 } });
        CHECK(random.calls == 14);
        for(const auto player : current.players()) CHECK(player.state().dice.total() == 2);
    };
    finish(executor, table);
    CHECK(copied_table.state().round_number == 1);
    finish(copied_executor, copied_table);
}

TEST_CASE("start round commands only broadcast and never count rounds clear dice or introduce observations", "[round][start_round]")
{
    const bool observed = GENERATE(false, true);
    round_log log;
    const auto observer = givm::test::with_passive_skill(round_observer{ &log });
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::start_dice_roll_phase{ .count = 3, .reroll_count = { 0, 0 } },
            givm::start_round{}, givm::start_round{}, givm::end_game{ givm::game_result::player_0_win } },
        std::tuple{}, observer);
    givm::table table{ givm::table_state{ .max_rounds = 0 } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } }, {});
    givm::executor executor;
    executor.enter_entry(library);
    counting_random random;
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(executor.view_in<givm::execution_state::finished>().result() == givm::game_result::player_0_win);
    CHECK(table.state().round_number == 0);
    CHECK(log.notifications == std::vector<std::uint32_t>{ 0, 0 });
    CHECK(log.dice_at_notification == std::vector<std::array<std::uint32_t, 2>>{ { 3, 3 }, { 3, 3 } });
    CHECK(log.preparations == std::vector<std::uint32_t>{ 0 });
    CHECK(random.calls == 6);
    for(const auto player : table.players()) CHECK(player.state().dice.total() == 3);
}
