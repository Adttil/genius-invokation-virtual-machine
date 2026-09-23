#include <concepts>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"

namespace
{
    struct discard_log
    {
        bool dynamic;
        bool self_effects;
        bool notification_effects;
        std::vector<char> order;
        std::vector<std::size_t> hand_counts;
        std::vector<std::size_t> deck_counts;
        std::vector<givm::deck_card_id> effect_cards;
    };

    struct discard_source
    {
        using definition_category = givm::card_definition;
        struct definition_type
        {
            discard_log* log;
            givm::program_entry discard;
            givm::program_entry effect;
            givm::program_entry notification;
        };
        discard_log* log;
        constexpr std::string_view name() const { return "DiscardCard"; }
        constexpr auto card_dependencies() const { return std::array{ name() }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto discard = log->dynamic
                ? context.add_program(std::tuple{ givm::discard_hand_card{}, givm::discard_deck_cards{} })
                : context.add_program(std::tuple{
                    givm::discard_hand_card{ .player = givm::relative_player::opponent,
                        .definition = context.resolve_id<givm::card_definition>(name()) },
                    givm::discard_deck_cards{ .count = 2, .player = givm::relative_player::opponent } });
            return { log, discard, context.add_program(std::tuple{
                givm::replace_cards{ givm::player_id{ 1 } }, givm::draw_cards{ .count = 1 } }),
                context.add_program(std::tuple{ givm::replace_cards{ givm::player_id{ 1 } } }) };
        }
        static givm::card_state query(const definition_type&, const givm::card_initial_state&)
        {
            return { .cost = { .energy = 3 } };
        }
        static givm::program_entry handle(const definition_type& data, const givm::deck_card_view& self,
            givm::test_event&, givm::handle_context& context)
        {
            if(self.player().id() != givm::player_id{ 1 }) return {};
            if(not data.log->dynamic) return context.invoke(data.discard);
            const auto hand_card = context.table()[givm::player_id{ 0 }].hand_cards().front().id();
            return context.invoke(data.discard,
                givm::hand_card_discard_effect{ hand_card },
                givm::deck_card_discard{ .player = givm::player_id{ 0 }, .count = 2 });
        }
        static givm::program_entry handle(const definition_type& data, const givm::hand_card_view& self,
            givm::hand_card_discard_effect& event, givm::handle_context& context)
        {
            CHECK(self.id() == event.card);
            CHECK_FALSE(self.is_valid());
            CHECK(self.state().cost.energy == 3);
            data.log->order.push_back('h');
            return data.log->self_effects ? context.invoke(data.effect) : givm::program_entry{};
        }
        static givm::program_entry handle(const definition_type& data, const givm::deck_card_view& self,
            givm::deck_card_discard_effect& event, givm::handle_context& context)
        {
            CHECK(self.id() == event.card);
            CHECK_FALSE(self.is_valid());
            CHECK(self.state().cost.energy == 3);
            if(data.log->effect_cards.empty())
                CHECK(context.table()[givm::player_id{ 0 }].deck_card_count() == (data.log->self_effects ? 2 : 3));
            data.log->effect_cards.push_back(event.card);
            data.log->order.push_back('d');
            return data.log->self_effects ? context.invoke(data.effect) : givm::program_entry{};
        }
        template<class TView, class TEvent>
            requires((std::same_as<TView, givm::hand_card_view> || std::same_as<TView, givm::deck_card_view>)
                && (std::same_as<TEvent, givm::hand_card_discarded> || std::same_as<TEvent, givm::deck_card_discarded>))
        static givm::program_entry handle(const definition_type& data, const TView& self,
            TEvent& event, givm::handle_context& context)
        {
            const auto discarded = context.table()[event.card];
            CHECK_FALSE(discarded.is_valid());
            CHECK(discarded.state().cost.energy == 3);
            CHECK(discarded.definition_id() == self.definition_id());
            CHECK(self.is_valid());
            if(self.player().id() == givm::player_id{ 1 })
            {
                data.log->order.push_back(std::same_as<TEvent, givm::hand_card_discarded> ? 'H' : 'D');
                const auto player = context.table()[givm::player_id{ 0 }];
                data.log->hand_counts.push_back(player.hand_card_count());
                data.log->deck_counts.push_back(player.deck_card_count());
                if(data.log->notification_effects) return context.invoke(data.notification);
            }
            return {};
        }
    };

    struct discard_damage_log
    {
        givm::deck_card_id card;
        givm::definition_id<givm::card_definition> definition;
        std::vector<char> order;
    };

    struct discard_damage_card
    {
        using definition_category = givm::card_definition;
        struct definition_type
        {
            discard_damage_log* log;
            givm::program_entry effect;
        };
        discard_damage_log* log;
        constexpr std::string_view name() const { return "DiscardDamageCard"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_program(std::tuple{ givm::apply_element{}, givm::deal_damage{} }) };
        }
        static givm::program_entry handle(const definition_type& data, const givm::deck_card_view& self,
            givm::deck_card_discard_effect& event, givm::handle_context& context)
        {
            CHECK_FALSE(self.is_valid());
            CHECK(self.id() == event.card);
            data.log->card = self.id();
            data.log->order.push_back('E');
            const auto target = context.table()[other_player(self.player().id())].characters().front().id();
            return context.invoke(data.effect,
                givm::element_application{ .source = self.id(), .target = target, .element = givm::element::cryo },
                givm::damage{ .source = self.id(), .target = target, .value = 2, .type = givm::damage_type::physical });
        }
    };

    struct discard_damage_observer
    {
        using definition_category = givm::character_view;
        struct definition_type { discard_damage_log* log; };
        discard_damage_log* log;
        constexpr std::string_view name() const { return "DiscardDamageObserver"; }
        definition_type compile(givm::definition_compile_context&) const { return { log }; }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            givm::after_damage& event, givm::handle_context& context)
        {
            REQUIRE(std::holds_alternative<givm::deck_card_id>(event.source));
            const auto card = std::get<givm::deck_card_id>(event.source);
            CHECK(card == data.log->card);
            CHECK_FALSE(context.table()[card].is_valid());
            CHECK(context.table()[card].definition_id() == data.log->definition);
            CHECK(event.target == self.id());
            CHECK(event.value == 2);
            CHECK(self.state().health == 8);
            CHECK(data.log->order == std::vector<char>{ 'E' });
            data.log->order.push_back('A');
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            givm::deck_card_discarded& event, givm::handle_context& context)
        {
            CHECK(event.card == data.log->card);
            CHECK(context.table()[event.card].definition_id() == data.log->definition);
            CHECK(self.state().health == 8);
            CHECK(data.log->order == std::vector<char>{ 'E', 'A' });
            data.log->order.push_back('D');
            return {};
        }
    };

    inline givm::execution_state advance(givm::executor& executor, const givm::definition_library& library, givm::table& table)
    {
        auto random = [] { return std::uint32_t{ 0 }; };
        for(;;)
        {
            const auto state = executor.step(library, table, random);
            if(state == givm::execution_state::card_selection || state == givm::execution_state::finished
                || state == givm::execution_state::deck_cards_discarded) return state;
        }
    }
}

TEST_CASE("discard batches leave together then run each effect and notification in order", "[discard]")
{
    discard_log log{ GENERATE(false, true), GENERATE(false, true), GENERATE(false, true) };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const discard_source source{ &log };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::draw_cards{ .count = 2 }, givm::test_command{}, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, source);
    const auto card = ids.get_id<givm::card_definition>(source.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } } };
    load_deck(table, library, { .cards = { card, card, card, card, card, card, card } }, { .cards = { card } });
    givm::executor executor;
    executor.enter_entry(library);
    std::size_t pauses = 0;
    std::vector<givm::deck_card_id> observed_cards;
    for(;;)
    {
        const auto state = advance(executor, library, table);
        if(state == givm::execution_state::finished) break;
        if(state == givm::execution_state::deck_cards_discarded)
        {
            const auto cards = executor.view_in<givm::execution_state::deck_cards_discarded>().cards();
            REQUIRE(cards.size() == 2);
            CHECK(observed_cards.empty());
            observed_cards.assign(cards.begin(), cards.end());
            for(auto id : cards) CHECK_FALSE(table[id].is_valid());
            CHECK(log.order == std::vector<char>{ 'h', 'H' });
        }
        else
        {
            ++pauses;
            CHECK(executor.view_in<givm::execution_state::card_selection>().selection_validate(table, {}));
        }
        auto table_copy = table;
        auto executor_copy = executor;
        table = std::move(table_copy);
        executor = std::move(executor_copy);
        if(state == givm::execution_state::card_selection)
            executor.view_in<givm::execution_state::card_selection>().select({});
        else
        {
            const auto cards = executor.view_in<givm::execution_state::deck_cards_discarded>().cards();
            CHECK(std::vector<givm::deck_card_id>{ cards.begin(), cards.end() } == observed_cards);
        }
    }
    CHECK(pauses == (log.self_effects ? 3 : 0) + (log.notification_effects ? 3 : 0));
    CHECK(log.order == std::vector<char>{ 'h', 'H', 'd', 'D', 'd', 'D' });
    CHECK(log.hand_counts == (log.self_effects ? std::vector<std::size_t>{ 2, 3, 4 } : std::vector<std::size_t>{ 1, 1, 1 }));
    CHECK(log.deck_counts == (log.self_effects ? std::vector<std::size_t>{ 4, 1, 0 } : std::vector<std::size_t>{ 5, 3, 3 }));
    if(mode == givm::compile_mode::observed) CHECK(observed_cards == log.effect_cards);
    else CHECK(observed_cards.empty());
    for(auto id : log.effect_cards) CHECK_FALSE(table[id].is_valid());
    table.clean_up();
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == log.hand_counts.back());
    CHECK(table[givm::player_id{ 0 }].deck_card_count() == log.deck_counts.back());
}

TEST_CASE("deck discard count is capped and empty batches have no observation", "[discard]")
{
    const auto count = GENERATE(0u, 1u, 4u);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const givm::test::named_definition_source<givm::card_definition> source{ "PlainCard" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::discard_deck_cards{ .count = count, .player = givm::relative_player::opponent },
            givm::discard_deck_cards{ .count = 2 }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, source);
    const auto card = ids.get_id<givm::card_definition>(source.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } } };
    load_deck(table, library, {}, { .cards = { card, card } });
    givm::executor executor;
    executor.enter_entry(library);
    std::size_t observations = 0;
    while(advance(executor, library, table) == givm::execution_state::deck_cards_discarded)
    {
        ++observations;
        const auto cards = executor.view_in<givm::execution_state::deck_cards_discarded>().cards();
        CHECK(cards.size() == std::min(count, 2u));
        for(auto id : cards)
        {
            CHECK(id.player_id == givm::player_id{ 1 });
            CHECK_FALSE(table[id].is_valid());
        }
    }
    CHECK(observations == (mode == givm::compile_mode::observed && count != 0 ? 1 : 0));
    CHECK(table[givm::player_id{ 1 }].deck_card_count() == 2 - std::min(count, 2u));
}

TEST_CASE("overflow removes cards without invoking discard effects or notifications", "[discard]")
{
    discard_log log{};
    const discard_source source{ &log };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::draw_cards{ .count = 3 }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, source);
    const auto card = ids.get_id<givm::card_definition>(source.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .hand_limit = 1 } };
    load_deck(table, library, { .cards = { card, card, card } }, { .cards = { card } });
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(advance(executor, library, table) == givm::execution_state::finished);
    CHECK(log.order.empty());
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 1);
    CHECK(table[givm::player_id{ 0 }].deck_card_count() == 0);
}

TEST_CASE("discarded deck cards remain valid effect sources through damage and notification", "[discard][damage]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    discard_damage_log log;
    const discard_damage_card source{ &log };
    const auto observer = givm::test::with_passive_skill(discard_damage_observer{ &log });
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::discard_deck_cards{ .count = 1 }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, source, observer);
    log.definition = ids.get_id<givm::card_definition>(source.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } } };
    load_deck(table, library, { .cards = { log.definition } },
        { .characters = { ids.get_id<givm::character_view>(observer.name()) } });
    const auto target = table[givm::player_id{ 1 }].characters().front().id();
    givm::executor executor;
    executor.enter_entry(library);
    auto random = [] { return std::uint32_t{ 0 }; };
    for(;;)
    {
        const auto state = executor.step(library, table, random);
        if(state == givm::execution_state::finished) break;
        if(state == givm::execution_state::deck_cards_discarded)
        {
            CHECK(log.order.empty());
            CHECK(table[target].state().health == 10);
        }
        else
        {
            REQUIRE(state == givm::execution_state::health_reduced);
            CHECK(log.order == std::vector<char>{ 'E' });
            CHECK(table[target].state().health == 8);
        }
    }
    CHECK(table[target].state().health == 8);
    CHECK(table[target].state().aura == givm::element_aura::cryo);
    CHECK(log.order == std::vector<char>{ 'E', 'A', 'D' });
}
