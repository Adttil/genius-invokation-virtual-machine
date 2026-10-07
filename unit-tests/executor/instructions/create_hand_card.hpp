#include <cstddef>
#include <array>
#include <concepts>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"
#include "../test_character_source.hpp"

namespace givm_test::executor_instructions::create_hand_card
{
constexpr std::array<std::size_t, 1> draw_positions_1{ 0 };

namespace
{
    struct created_card_source
    {
        static constexpr auto category = givm::definition_category::card;
        struct definition_type {};

        constexpr std::string_view name() const { return "CreatedCard"; }
        definition_type compile(givm::definition_compile_context&) const { return {}; }
        static givm::card_state query(const definition_type&, const givm::card_initial_state&)
        {
            return { .cost = { .speed = givm::action_speed::fast, .energy = 3 },
                .elemental_tuning_allowed = false };
        }
    };

    struct creation_log
    {
        bool dynamic;
        std::vector<char> notifications;
        std::vector<givm::hand_card_id> cards;
        std::size_t discards = 0;
    };

    struct creation_driver_source
    {
        static constexpr auto category = givm::definition_category::card;
        struct definition_type
        {
            creation_log* log;
            givm::optional_definition_id<givm::definition_category::card> card;
            givm::normal_effect create;
            givm::normal_effect nested;
        };
        creation_log* log;

        constexpr std::string_view name() const { return "CreationDriver"; }
        auto card_dependencies() const { return std::array{ std::string_view{ "CreatedCard" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto card = context.resolve_id<givm::definition_category::card>("CreatedCard");
            const auto create = log->dynamic ? givm::create_hand_card{}
                : givm::create_hand_card{ .player = givm::relative_player::opponent, .definition = card };
            const auto nested = log->dynamic ? givm::create_hand_card{}
                : givm::create_hand_card{ .definition = card };
            return { log, card,
                context.add_normal_effect(std::tuple{ create,
                    givm::draw_cards{ .player = givm::relative_player::opponent, .position = 0, .count = 1 }, create }),
                context.add_normal_effect(std::tuple{ givm::replace_cards{ givm::player_id{ 1 } }, nested }) };
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::round_started&, givm::handle_context<givm::deck_card_view>& context, std::uint32_t = 0)
        {
            if(data.log->dynamic)
                return context.invoke(data.create,
                    givm::create_hand_card_input{ .player = givm::player_id{ 0 }, .definition = data.card.get() },
                    givm::create_hand_card_input{ .player = givm::player_id{ 0 }, .definition = data.card.get() });
            return context.invoke(data.create);
        }
        template<class TEvent>
            requires(std::same_as<TEvent, givm::card_drawn> || std::same_as<TEvent, givm::hand_card_added>)
        static givm::normal_effect handle(const definition_type& data,
            TEvent& event, givm::handle_context<givm::deck_card_view>& context, std::uint32_t = 0)
        {
            const auto card = context.table()[event.card];
            CHECK(card.is_valid() == not event.overflow);
            CHECK(card.definition_id() == data.card);
            CHECK(card.state().cost.energy == 3);
            CHECK(card.state().cost.speed == givm::action_speed::fast);
            CHECK_FALSE(card.state().elemental_tuning_allowed);
            data.log->notifications.push_back(std::same_as<TEvent, givm::card_drawn> ? 'D' : 'A');
            data.log->cards.push_back(event.card);
            if(data.log->cards.size() == 1)
            {
                if(data.log->dynamic)
                    return context.invoke(data.nested,
                        givm::create_hand_card_input{ .player = givm::player_id{ 1 }, .definition = data.card.get() });
                return context.invoke(data.nested);
            }
            return {};
        }
        template<class TEvent>
            requires(std::same_as<TEvent, givm::hand_card_discarded> || std::same_as<TEvent, givm::deck_card_discarded>)
        static givm::normal_effect handle(const definition_type& data,
            TEvent&, givm::handle_context<givm::deck_card_view>&, std::uint32_t = 0)
        {
            ++data.log->discards;
            return {};
        }
    };

    struct entry_order_log
    {
        bool reverse;
        std::vector<char> responses;
    };

    template<bool AnyEntry>
    struct entry_attachment_source
    {
        static constexpr auto category = givm::definition_category::attachment;
        struct definition_type { entry_order_log* log; };
        entry_order_log* log;

        constexpr std::string_view name() const { return AnyEntry ? "AnyEntry" : "DrawOnly"; }
        definition_type compile(givm::definition_compile_context&) const { return { log }; }
        template<class TEvent>
            requires(std::same_as<TEvent, givm::card_drawn>
                || (AnyEntry && std::same_as<TEvent, givm::hand_card_added>))
        static givm::normal_effect handle(const definition_type& data,
            TEvent&, givm::handle_context<givm::attachment_view>&, std::uint32_t = 0)
        {
            data.log->responses.push_back(AnyEntry ? 'A' : 'D');
            return {};
        }
    };

    struct entry_order_driver_source
    {
        static constexpr auto category = givm::definition_category::card;
        struct definition_type { givm::normal_effect effect; };
        entry_order_log* log;

        constexpr std::string_view name() const { return "EntryOrderDriver"; }
        auto attachment_dependencies() const
        {
            return std::array{ std::string_view{ "AnyEntry" }, std::string_view{ "DrawOnly" } };
        }
        auto card_dependencies() const { return std::array{ std::string_view{ "CreatedCard" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto first = context.resolve_id<givm::definition_category::attachment>(log->reverse ? "AnyEntry" : "DrawOnly");
            const auto second = context.resolve_id<givm::definition_category::attachment>(log->reverse ? "DrawOnly" : "AnyEntry");
            return { context.add_normal_effect(std::tuple{
                givm::add_attachment{ .player = givm::relative_player::opponent, .definition = first },
                givm::add_attachment{ .player = givm::relative_player::opponent, .definition = second },
                givm::draw_cards{ .player = givm::relative_player::opponent, .position = 0, .count = 1 },
                givm::create_hand_card{ .player = givm::relative_player::opponent,
                    .definition = context.resolve_id<givm::definition_category::card>("CreatedCard") } }) };
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::round_started&, givm::handle_context<givm::deck_card_view>& context, std::uint32_t = 0)
        {
            return context.invoke(data.effect);
        }
    };
}

TEST_CASE("hand card creation initializes cards and resumes nested notifications", "[create_hand_card]")
{
    creation_log log{ GENERATE(false, true) };
    const bool full = GENERATE(false, true);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
        created_card_source{}, creation_driver_source{ &log });
    const auto card = ids.get_id<givm::definition_category::card>("CreatedCard");
    givm::table table{ {}, { .hand_limit = full ? 1u : 10u }, {} };
    load_deck(table, library, { .cards = { card } },
        { .cards = { ids.get_id<givm::definition_category::card>("CreationDriver") } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    auto random = [] { return std::uint32_t{ 0 }; };
    std::size_t pauses = 0;
    for(;;)
    {
        const auto state = executor.advance(library, table, random);
        if(state == givm::execution_state::finished) break;
        if(state != givm::execution_state::card_selection) continue;
        ++pauses;
        CHECK(log.notifications == std::vector<char>{ 'A' });
        CHECK(table[givm::player_id{ 0 }].hand_card_count() == (full ? 1 : 3));
        CHECK(table[givm::player_id{ 1 }].hand_card_count() == 0);
        auto copied_table = table;
        auto copied_executor = executor;
        table = std::move(copied_table);
        executor = std::move(copied_executor);
        const auto view = executor.view_in<givm::execution_state::card_selection>();
        CHECK(view.selection_validate(table, {}));
        executor.submitted(view.select(library, table, random, {}));
    }
    CHECK(pauses == 1);
    CHECK(log.discards == 0);
    CHECK(log.notifications == std::vector<char>{ 'A', 'A', 'D', 'A' });
    REQUIRE(log.cards.size() == 4);
    for(std::size_t index = 0; index < log.cards.size(); ++index)
        CHECK(table[log.cards[index]].player().id() == givm::player_id{ index == 1 ? 1u : 0u });
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == (full ? 1u : 3u));
    CHECK(table[givm::player_id{ 0 }].deck_card_count() == 0);
    CHECK(table[givm::player_id{ 1 }].hand_card_count() == 1);
}

TEST_CASE("draw and general hand entry responders share attachment order", "[create_hand_card]")
{
    entry_order_log log{ GENERATE(false, true) };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
        created_card_source{}, entry_order_driver_source{ &log }, entry_attachment_source<false>{ &log },
        entry_attachment_source<true>{ &log }, givm::test::initialized_character_source{});
    const givm::character_id active{ givm::player_id{ 0 }, 0 };
    givm::table table{ {}, { .active_character = active }, {} };
    load_deck(table, library,
        { .cards = { ids.get_id<givm::definition_category::card>("CreatedCard") },
            .characters = { ids.get_id<givm::definition_category::character>("Character") } },
        { .cards = { ids.get_id<givm::definition_category::card>("EntryOrderDriver") } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    auto random = [] { return std::uint32_t{ 0 }; };
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::finished);
    CHECK(log.responses == (log.reverse ? std::vector<char>{ 'A', 'D', 'A' } : std::vector<char>{ 'D', 'A', 'A' }));
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 2);
}
}
