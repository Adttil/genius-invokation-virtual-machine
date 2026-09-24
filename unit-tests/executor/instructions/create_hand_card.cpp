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

namespace
{
    struct created_card_source
    {
        using definition_category = givm::card_definition;
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
        using definition_category = givm::card_definition;
        struct definition_type
        {
            creation_log* log;
            givm::definition_id<givm::card_definition> card;
            givm::program_entry create;
            givm::program_entry nested;
        };
        creation_log* log;

        constexpr std::string_view name() const { return "CreationDriver"; }
        auto card_dependencies() const { return std::array{ std::string_view{ "CreatedCard" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto card = context.resolve_id<givm::card_definition>("CreatedCard");
            const auto create = log->dynamic ? givm::create_hand_card{}
                : givm::create_hand_card{ .player = givm::relative_player::opponent, .definition = card };
            const auto nested = log->dynamic ? givm::create_hand_card{}
                : givm::create_hand_card{ .definition = card };
            return { log, card,
                context.add_program(std::tuple{ create,
                    givm::draw_cards{ .count = 1, .player = givm::relative_player::opponent }, create }),
                context.add_program(std::tuple{ givm::replace_cards{ givm::player_id{ 1 } }, nested }) };
        }
        static givm::program_entry handle(const definition_type& data, const givm::deck_card_view&,
            givm::test_event&, givm::handle_context& context)
        {
            if(data.log->dynamic)
                return context.invoke(data.create,
                    givm::hand_card_creation{ .player = givm::player_id{ 0 }, .definition = data.card },
                    givm::hand_card_creation{ .player = givm::player_id{ 0 }, .definition = data.card });
            return context.invoke(data.create);
        }
        template<class TEvent>
            requires(std::same_as<TEvent, givm::card_drawn> || std::same_as<TEvent, givm::hand_card_added>)
        static givm::program_entry handle(const definition_type& data, const givm::deck_card_view&,
            TEvent& event, givm::handle_context& context)
        {
            const auto card = context.table()[event.card];
            CHECK(card.is_valid());
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
                        givm::hand_card_creation{ .player = givm::player_id{ 1 }, .definition = data.card });
                return context.invoke(data.nested);
            }
            return {};
        }
        template<class TEvent>
            requires(std::same_as<TEvent, givm::hand_card_discarded> || std::same_as<TEvent, givm::deck_card_discarded>)
        static givm::program_entry handle(const definition_type& data, const givm::deck_card_view&,
            TEvent&, givm::handle_context&)
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
        using definition_category = givm::attachment_view;
        struct definition_type { entry_order_log* log; };
        entry_order_log* log;

        constexpr std::string_view name() const { return AnyEntry ? "AnyEntry" : "DrawOnly"; }
        definition_type compile(givm::definition_compile_context&) const { return { log }; }
        template<class TEvent>
            requires(std::same_as<TEvent, givm::card_drawn>
                || (AnyEntry && std::same_as<TEvent, givm::hand_card_added>))
        static givm::program_entry handle(const definition_type& data, const givm::attachment_view&,
            TEvent&, givm::handle_context&)
        {
            data.log->responses.push_back(AnyEntry ? 'A' : 'D');
            return {};
        }
    };

    struct entry_order_driver_source
    {
        using definition_category = givm::card_definition;
        struct definition_type { givm::program_entry effect; };
        entry_order_log* log;

        constexpr std::string_view name() const { return "EntryOrderDriver"; }
        auto attachment_dependencies() const
        {
            return std::array{ std::string_view{ "AnyEntry" }, std::string_view{ "DrawOnly" } };
        }
        auto card_dependencies() const { return std::array{ std::string_view{ "CreatedCard" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto first = context.resolve_id<givm::attachment_view>(log->reverse ? "AnyEntry" : "DrawOnly");
            const auto second = context.resolve_id<givm::attachment_view>(log->reverse ? "DrawOnly" : "AnyEntry");
            return { context.add_program(std::tuple{
                givm::add_attachment{ .player = givm::relative_player::opponent, .definition = first },
                givm::add_attachment{ .player = givm::relative_player::opponent, .definition = second },
                givm::draw_cards{ .count = 1, .player = givm::relative_player::opponent },
                givm::create_hand_card{ .player = givm::relative_player::opponent,
                    .definition = context.resolve_id<givm::card_definition>("CreatedCard") } }) };
        }
        static givm::program_entry handle(const definition_type& data, const givm::deck_card_view&,
            givm::test_event&, givm::handle_context& context)
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
        std::tuple{ givm::test_command{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
        created_card_source{}, creation_driver_source{ &log });
    const auto card = ids.get_id<givm::card_definition>("CreatedCard");
    givm::table table{ {}, { .hand_limit = full ? 1u : 10u }, {} };
    load_deck(table, library, { .cards = { card } },
        { .cards = { ids.get_id<givm::card_definition>("CreationDriver") } });
    givm::executor executor;
    executor.enter_entry(library);
    auto random = [] { return std::uint32_t{ 0 }; };
    std::size_t pauses = 0;
    for(;;)
    {
        const auto state = executor.step(library, table, random);
        if(state == givm::execution_state::finished) break;
        if(state != givm::execution_state::card_selection) continue;
        ++pauses;
        CHECK(log.notifications == std::vector<char>{ 'A' });
        CHECK(table[givm::player_id{ 0 }].hand_card_count() == 1);
        CHECK(table[givm::player_id{ 1 }].hand_card_count() == 0);
        auto copied_table = table;
        auto copied_executor = executor;
        table = std::move(copied_table);
        executor = std::move(copied_executor);
        const auto view = executor.view_in<givm::execution_state::card_selection>();
        CHECK(view.selection_validate(table, {}));
        view.select({});
    }
    CHECK(pauses == 1);
    CHECK(log.discards == 0);
    CHECK(log.notifications == (full ? std::vector<char>{ 'A', 'A' } : std::vector<char>{ 'A', 'A', 'D', 'A' }));
    REQUIRE(log.cards.size() == (full ? 2u : 4u));
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
        std::tuple{ givm::test_command{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
        created_card_source{}, entry_order_driver_source{ &log }, entry_attachment_source<false>{ &log },
        entry_attachment_source<true>{ &log }, givm::test::initialized_character_source{});
    const givm::character_id active{ givm::player_id{ 0 }, 0 };
    givm::table table{ {}, { .active_character = active }, {} };
    load_deck(table, library,
        { .cards = { ids.get_id<givm::card_definition>("CreatedCard") },
            .characters = { ids.get_id<givm::character_view>("Character") } },
        { .cards = { ids.get_id<givm::card_definition>("EntryOrderDriver") } });
    givm::executor executor;
    executor.enter_entry(library);
    auto random = [] { return std::uint32_t{ 0 }; };
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(log.responses == (log.reverse ? std::vector<char>{ 'A', 'D', 'A' } : std::vector<char>{ 'D', 'A', 'A' }));
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 2);
}
