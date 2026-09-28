#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"

namespace givm_test::executor_instructions::draw_cards
{
namespace
{
    struct draw_log
    {
        bool dynamic = false;
        givm::relative_player player = givm::relative_player::self;
        std::vector<std::size_t> positions;
        std::vector<std::pair<std::size_t, std::size_t>> selections;
        bool pause = false;
        std::vector<givm::deck_card_id> input;
        std::vector<std::uint32_t> drawn;
        std::vector<std::size_t> owners;
        std::vector<std::array<std::size_t, 2>> deck_counts;
        std::vector<std::array<std::size_t, 2>> hand_counts;
        std::size_t unexpected_notifications = 0;
        std::size_t pauses = 0;
    };

    struct draw_card_source
    {
        using definition_category = givm::card_definition;
        struct definition_type { std::uint32_t value; };
        std::string_view source_name;
        std::uint32_t value;

        std::string_view name() const { return source_name; }
        definition_type compile(givm::definition_compile_context&) const { return { value }; }
        static givm::card_state query(const definition_type& data, const givm::card_initial_state&)
        {
            return { .cost = { .speed = givm::action_speed::fast, .energy = data.value },
                .elemental_tuning_allowed = false };
        }
    };

    struct draw_driver_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            draw_log* log;
            givm::program_entry draw;
            givm::program_entry pause;
        };
        draw_log* log;

        std::string_view name() const { return "DrawDriver"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto draw = log->dynamic
                ? context.add_program(std::tuple{ givm::draw_cards{}, givm::draw_cards{} })
                : context.add_program(std::tuple{ givm::draw_cards{ .player = log->player, .positions = log->positions } });
            return { log, draw,
                context.add_program(std::tuple{ givm::replace_cards{ givm::player_id{ 0 } } }) };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::round_started&, givm::handle_context& context)
        {
            if(data.log->dynamic)
                return context.invoke(data.draw, givm::draw_cards_input{ data.log->input }, givm::draw_cards_input{});
            return context.invoke(data.draw);
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::card_drawn& event, givm::handle_context& context)
        {
            const auto card = context.table()[event.card];
            CHECK(card.is_valid());
            CHECK(card.state().cost.speed == givm::action_speed::fast);
            CHECK_FALSE(card.state().elemental_tuning_allowed);
            data.log->drawn.push_back(card.state().cost.energy);
            data.log->owners.push_back(card.player().id().index);
            const auto first = context.table()[givm::player_id{ 0 }];
            const auto second = context.table()[givm::player_id{ 1 }];
            data.log->deck_counts.push_back({ first.deck_card_count(), second.deck_card_count() });
            data.log->hand_counts.push_back({ first.hand_card_count(), second.hand_card_count() });
            if(data.log->pause && data.log->drawn.size() == 1) return context.invoke(data.pause);
            return {};
        }
        template<class TEvent>
            requires(std::same_as<TEvent, givm::hand_card_added> || std::same_as<TEvent, givm::hand_card_discarded>
                || std::same_as<TEvent, givm::deck_card_discarded>)
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            TEvent&, givm::handle_context&)
        {
            ++data.log->unexpected_notifications;
            return {};
        }
    };

    inline givm::table run_draw(draw_log& log, givm::compile_mode mode,
        std::array<std::uint32_t, 2> hand_limits = { 10, 10 })
    {
        const auto driver = givm::test::with_passive_skill(draw_driver_source{ &log });
        const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
            std::tuple{ givm::start_round{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
            driver, draw_card_source{ "A", 1 }, draw_card_source{ "B", 2 }, draw_card_source{ "C", 3 },
            draw_card_source{ "D", 4 }, draw_card_source{ "E", 5 });
        const auto a = ids.get_id<givm::card_definition>("A");
        const auto b = ids.get_id<givm::card_definition>("B");
        const auto c = ids.get_id<givm::card_definition>("C");
        const auto d = ids.get_id<givm::card_definition>("D");
        const auto e = ids.get_id<givm::card_definition>("E");
        givm::table table{ {}, { .hand_limit = hand_limits[0] }, { .hand_limit = hand_limits[1] } };
        load_deck(table, library,
            { .cards = { a, b, c, d, e }, .characters = { ids.get_id<givm::character_view>(driver.name()) } },
            { .cards = { a, b, c, d, e } });
        std::array<std::vector<givm::deck_card_id>, 2> decks;
        for(std::size_t player = 0; player != decks.size(); ++player)
            for(const auto card : table[givm::player_id{ player }].deck_cards()) decks[player].push_back(card.id());
        for(const auto& [player, index] : log.selections) log.input.push_back(decks[player][index]);
        givm_test::executor_driver executor;
        executor.start(library, table);
        auto random = [] { return std::uint32_t{ 0 }; };
        for(;;)
        {
            const auto state = executor.advance(library, table, random);
            if(state == givm::execution_state::finished) break;
            if(state != givm::execution_state::card_selection) continue;
            ++log.pauses;
            CHECK(log.drawn.size() == 1);
            auto table_copy = table;
            auto executor_copy = executor;
            table = std::move(table_copy);
            executor = std::move(executor_copy);
            const auto view = executor.view_in<givm::execution_state::card_selection>();
            CHECK(view.selection_validate(table, {}));
            executor.submitted(view.select(library, table, random, {}));
        }
        CHECK(log.unexpected_notifications == 0);
        for(const auto id : log.input) CHECK_FALSE(table[id].is_valid());
        return table;
    }

    inline std::vector<std::uint32_t> deck_values(const givm::table& table, givm::player_id player)
    {
        std::vector<std::uint32_t> result;
        for(const auto card : table[player].deck_cards()) result.push_back(card.state().cost.energy);
        return result;
    }

    inline std::vector<std::uint32_t> hand_values(const givm::table& table, givm::player_id player)
    {
        std::vector<std::uint32_t> result;
        for(const auto card : table[player].hand_cards()) result.push_back(card.state().cost.energy);
        return result;
    }
}

TEST_CASE("fixed draw positions use the original deck order and preserve remaining cards", "[draw_cards]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto player = GENERATE(givm::relative_player::self, givm::relative_player::opponent);
    const bool reverse = GENERATE(false, true);
    draw_log log{ .player = player, .positions = reverse ? std::vector<std::size_t>{ 3, 99, 1 }
        : std::vector<std::size_t>{ 1, 99, 3 }, .pause = true };
    const auto table = run_draw(log, mode);
    const auto target = givm::player_id{ player == givm::relative_player::self ? 0u : 1u };
    const std::vector<std::uint32_t> expected = reverse ? std::vector<std::uint32_t>{ 2, 4 }
        : std::vector<std::uint32_t>{ 4, 2 };
    CHECK(log.drawn == expected);
    CHECK(log.owners == std::vector<std::size_t>(2, target.index));
    CHECK(hand_values(table, target) == expected);
    CHECK(deck_values(table, target) == std::vector<std::uint32_t>{ 1, 3, 5 });
    CHECK(deck_values(table, other_player(target)) == std::vector<std::uint32_t>{ 1, 2, 3, 4, 5 });
    const std::array<std::size_t, 2> deck_counts = target.index == 0
        ? std::array<std::size_t, 2>{ 3, 5 } : std::array<std::size_t, 2>{ 5, 3 };
    const std::array<std::size_t, 2> hand_counts = target.index == 0
        ? std::array<std::size_t, 2>{ 2, 0 } : std::array<std::size_t, 2>{ 0, 2 };
    CHECK(log.deck_counts == std::vector(2, deck_counts));
    CHECK(log.hand_counts == std::vector(2, hand_counts));
    CHECK(log.pauses == 1);
}

TEST_CASE("dynamic draw IDs keep input order across both players and finish the batch before notifications", "[draw_cards]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool full = GENERATE(false, true);
    draw_log log{ .dynamic = true, .selections = { { 1, 1 }, { 0, 3 }, { 1, 4 }, { 0, 0 } }, .pause = true };
    const auto table = run_draw(log, mode, full ? std::array<std::uint32_t, 2>{ 1, 1 }
        : std::array<std::uint32_t, 2>{ 10, 10 });
    CHECK(log.drawn == (full ? std::vector<std::uint32_t>{ 2, 4 } : std::vector<std::uint32_t>{ 2, 4, 5, 1 }));
    CHECK(log.owners == (full ? std::vector<std::size_t>{ 1, 0 } : std::vector<std::size_t>{ 1, 0, 1, 0 }));
    CHECK(hand_values(table, givm::player_id{ 0 }) == (full ? std::vector<std::uint32_t>{ 4 }
        : std::vector<std::uint32_t>{ 4, 1 }));
    CHECK(hand_values(table, givm::player_id{ 1 }) == (full ? std::vector<std::uint32_t>{ 2 }
        : std::vector<std::uint32_t>{ 2, 5 }));
    CHECK(deck_values(table, givm::player_id{ 0 }) == std::vector<std::uint32_t>{ 2, 3, 5 });
    CHECK(deck_values(table, givm::player_id{ 1 }) == std::vector<std::uint32_t>{ 1, 3, 4 });
    CHECK(log.deck_counts == std::vector(log.drawn.size(), std::array<std::size_t, 2>{ 3, 3 }));
    const std::size_t hand_count = full ? 1 : 2;
    CHECK(log.hand_counts == std::vector(log.drawn.size(), std::array<std::size_t, 2>{ hand_count, hand_count }));
    CHECK(log.pauses == 1);
}

TEST_CASE("empty and entirely overflowing draw batches leave no notifications", "[draw_cards]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto scenario = GENERATE(0, 1, 2, 3);
    draw_log log{ .dynamic = scenario != 1, .positions = { 99 } };
    if(scenario == 2) log.selections = { { 0, 1 }, { 1, 3 } };
    if(scenario == 3)
    {
        log.dynamic = false;
        log.positions = { 0, 1, 2, 3, 4, 5 };
    }
    const auto table = run_draw(log, mode, { 0, 0 });
    CHECK(log.drawn.empty());
    CHECK(log.pauses == 0);
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 0);
    CHECK(table[givm::player_id{ 1 }].hand_card_count() == 0);
    CHECK(table[givm::player_id{ 0 }].deck_card_count() == (scenario == 3 ? 0 : scenario == 2 ? 4 : 5));
    CHECK(table[givm::player_id{ 1 }].deck_card_count() == (scenario == 2 ? 4 : 5));
}
}
