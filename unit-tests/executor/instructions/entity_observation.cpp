#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"

namespace
{
    struct counting_random
    {
        std::uint32_t calls = 0;

        std::uint32_t operator()() noexcept
        {
            ++calls;
            return 2;
        }
    };

    struct initialized_character_source
    {
        using definition_category = givm::character_view;
        struct definition_type {};

        constexpr std::string_view name() const noexcept { return "ObservedCharacter"; }
        constexpr definition_type compile(givm::definition_compile_context&) const noexcept { return {}; }

        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 12, .max_energy = 3, .health = 12, .energy = 1 };
        }
    };

    struct creation_program_source
    {
        using definition_category = givm::character_view;
        struct definition_type { givm::program_entry entry; };

        constexpr std::string_view name() const noexcept { return "CreationProgram"; }
        constexpr auto character_dependencies() const noexcept
        {
            return std::array{ std::string_view{ "ObservedCharacter" } };
        }
        constexpr auto card_dependencies() const noexcept
        {
            return std::array{ std::string_view{ "ObservedCard" }, std::string_view{ "OtherObservedCard" } };
        }

        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto character = context.resolve_id<givm::character_view>("ObservedCharacter");
            const auto card = context.resolve_id<givm::card_definition>("ObservedCard");
            const auto other_card = context.resolve_id<givm::card_definition>("OtherObservedCard");
            return { context.add_program(std::tuple{
                givm::enter_character{ .player = givm::player_id{ 1 }, .definition = character },
                givm::insert_deck_card{ .player = givm::player_id{ 1 }, .definition = card },
                givm::insert_deck_card{ .player = givm::player_id{ 1 }, .definition = other_card, .position = 0 }
            }) };
        }

        static givm::program_entry handle(
            const definition_type& data, const givm::character_view&, givm::test_event&, givm::handle_context& context)
        {
            if(data.entry) return context.invoke(data.entry);
            return {};
        }
    };

    struct entity_event_log
    {
        std::vector<givm::hand_card_id> drawn;
        std::vector<std::array<std::size_t, 2>> card_counts_at_drawn;
        std::vector<givm::character_id> active;
    };

    struct empty_program_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            givm::program_entry entry;
            std::uint32_t* calls;
        };
        std::uint32_t* calls;

        constexpr std::string_view name() const noexcept { return "EmptyProgram"; }

        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.add_program(std::tuple{}), calls };
        }

        static givm::program_entry handle(
            const definition_type& data, const givm::character_view&, givm::test_event&, givm::handle_context& context)
        {
            CHECK_FALSE(data.entry.is_null());
            ++*data.calls;
            if(data.entry) return context.invoke(data.entry);
            return {};
        }
    };

    struct entity_observer_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            entity_event_log* log;
            givm::program_entry draw_response;
        };
        entity_event_log* log;
        bool respond_to_draws = false;

        constexpr std::string_view name() const noexcept { return "EntityObserver"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return {
                log,
                respond_to_draws
                    ? context.add_program(std::tuple{ givm::start_round{ .max_rounds = 10 } })
                    : givm::program_entry::null()
            };
        }

        static givm::program_entry handle(
            const definition_type& data, const givm::character_view&, givm::card_drawn& event,
            givm::handle_context& context)
        {
            data.log->drawn.push_back(event.card);
            const auto player = context.table()[event.card.player_id];
            data.log->card_counts_at_drawn.push_back({ player.hand_card_count(), player.deck_card_count() });
            if(data.draw_response) return context.invoke(data.draw_response);
            return {};
        }

        static givm::program_entry handle(
            const definition_type& data, const givm::character_view&, givm::active_character_changed& event,
            givm::handle_context& context)
        {
            CHECK(context.table()[event.current.player_id].state().active_character == event.current);
            data.log->active.push_back(event.current);
            return {};
        }
    };

    struct initial_switch_response_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            entity_event_log* log;
            givm::program_entry entry;
        };
        entity_event_log* log;
        int behavior;

        constexpr std::string_view name() const noexcept { return "InitialSwitchResponse"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            if(behavior == 1)
                return { log, context.add_program(std::tuple{
                    givm::end_game{ .result = givm::game_result::player_0_win }
                }) };
            if(behavior == 2)
                return { log, context.add_program(std::tuple{
                    givm::set_active_character{ .target = givm::character_id{ givm::player_id{ 1 }, 0 } },
                    givm::end_game{ .result = givm::game_result::player_0_win }
                }) };
            return { log, givm::program_entry::null() };
        }

        static givm::program_entry handle(
            const definition_type& data, const givm::character_view&, givm::active_character_changed& event,
            givm::handle_context& context)
        {
            CHECK(context.table()[event.current.player_id].state().active_character == event.current);
            data.log->active.push_back(event.current);
            if(event.current.player_id == givm::player_id{ 0 } and data.entry) return context.invoke(data.entry);
            return {};
        }
    };

    struct switch_back_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            entity_event_log* log;
            givm::program_entry entry;
        };
        entity_event_log* log;

        constexpr std::string_view name() const noexcept { return "SwitchBack"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_program(std::tuple{
                givm::set_active_character{ .target = givm::character_id{ givm::player_id{ 0 }, 0 } }
            }) };
        }
        static givm::program_entry handle(
            const definition_type& data, const givm::character_view&, givm::active_character_changed& event,
            givm::handle_context& context)
        {
            CHECK(context.table()[event.current.player_id].state().active_character == event.current);
            data.log->active.push_back(event.current);
            if(event.current.index == 1 and data.entry) return context.invoke(data.entry);
            return {};
        }
    };

}

TEST_CASE("step passes through creation responses and preserves initialization", "[entity-observation]")
{
    const initialized_character_source character_source;
    const givm::test::named_definition_source<givm::card_definition> card_source{ "ObservedCard" };
    const givm::test::named_definition_source<givm::card_definition> other_card_source{ "OtherObservedCard" };
    const creation_program_source program_source;
    const auto compile_program = [&](givm::compile_mode mode)
    {
        return givm::test::compile_definitions_with_program(
            mode,
            std::tuple{ givm::test_command{} }, std::tuple{ givm::end_game{ .result = givm::game_result::both_loss } },
            program_source, card_source, other_card_source, character_source
        );
    };
    const auto [library, ids] = compile_program(givm::compile_mode::observed);
    const auto normal_compilation = compile_program(givm::compile_mode::normal);
    givm::table table{};
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(program_source.name()) } }, {});
    auto normal_table = table;
    counting_random normal_random;
    givm::executor normal;
    normal.enter_entry(normal_compilation.library);
    REQUIRE(normal.step(normal_compilation.library, normal_table, normal_random) == givm::execution_state::finished);

    counting_random random;
    givm::executor observed;
    observed.enter_entry(library);
    REQUIRE(observed.step(library, table, random) == givm::execution_state::finished);
    auto characters = table[givm::player_id{ 1 }].characters();
    REQUIRE(std::ranges::distance(characters) == 1);
    const auto character = (*characters.begin()).id();
    CHECK(table[character].definition_id().value() == ids.get_id<givm::character_view>(character_source.name()).value());
    CHECK(table[character].state().max_health == 12);
    CHECK(table[character].state().health == 12);
    CHECK(table[character].state().energy == 1);
    CHECK(table[givm::player_id{ 1 }].deck_card_count() == 2);
    CHECK(table[givm::player_id{ 1 }].deck_card_definition(0) == ids.get_id<givm::card_definition>(other_card_source.name()));
    CHECK(table[givm::player_id{ 1 }].deck_card_definition(1) == ids.get_id<givm::card_definition>(card_source.name()));
    CHECK(random.calls == normal_random.calls);
    CHECK(random.calls == 0);
    const auto normal_character = *normal_table[givm::player_id{ 1 }].characters().begin();
    CHECK(table[character].state().health == normal_character.state().health);
    CHECK(table[givm::player_id{ 1 }].deck_card_count() == normal_table[givm::player_id{ 1 }].deck_card_count());
}

TEST_CASE("step passes through an empty response without an observation", "[entity-observation]")
{
    std::uint32_t handler_calls = 0;
    const empty_program_source source{ &handler_calls };
    const auto compile_program = [&](givm::compile_mode mode)
    {
        return givm::test::compile_definitions_with_program(
            mode,
            std::tuple{ givm::test_command{} }, std::tuple{ givm::end_game{ .result = givm::game_result::both_loss } }, source
        );
    };
    const auto [library, ids] = compile_program(givm::compile_mode::observed);
    const auto normal_compilation = compile_program(givm::compile_mode::normal);
    givm::table table{};
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(source.name()) } }, {});
    auto normal_table = table;
    counting_random random;
    givm::executor normal;
    normal.enter_entry(normal_compilation.library);
    REQUIRE(normal.step(normal_compilation.library, normal_table, random) == givm::execution_state::finished);
    REQUIRE(handler_calls == 1);
    handler_calls = 0;

    givm::executor observed;
    observed.enter_entry(library);
    REQUIRE(observed.step(library, table, random) == givm::execution_state::finished);
    CHECK(handler_calls == 1);
    CHECK(random.calls == 0);
}

TEST_CASE("step passes through draws and full-hand discards while preserving broadcasts", "[entity-observation][draw_cards]")
{
    std::size_t initial_hand_count = 1;
    SECTION("one card fits before the hand becomes full") {}
    SECTION("all cards are discarded") { initial_hand_count = 2; }

    entity_event_log log;
    const entity_observer_source observer_source{ &log };
    const givm::test::named_definition_source<givm::card_definition> card_source{ "ObservedCard" };
    const auto compile_program = [&](givm::compile_mode mode)
    {
        return givm::test::compile_definitions_with_program(
            mode,
            std::tuple{
                givm::draw_cards{ .count = static_cast<std::uint32_t>(initial_hand_count), .player = givm::relative_player::current },
                givm::draw_cards{ .count = 5, .player = givm::relative_player::current },
                givm::draw_cards{ .count = 1, .player = givm::relative_player::current }
            },
            std::tuple{ givm::end_game{ .result = givm::game_result::both_loss } }, observer_source, card_source
        );
    };
    const auto [library, ids] = compile_program(givm::compile_mode::observed);
    const auto normal_compilation = compile_program(givm::compile_mode::normal);
    givm::table table{ { .hand_limit = 2 } };
    const auto card_definition = ids.get_id<givm::card_definition>(card_source.name());
    givm::linked_deck deck;
    deck.cards.assign(initial_hand_count + 3, card_definition);
    load_deck(table, library, deck, { .characters = { ids.get_id<givm::character_view>(observer_source.name()) } });
    const auto player = table[givm::player_id{ 0 }];

    auto normal_table = table;
    counting_random random;
    givm::executor normal;
    normal.enter_entry(normal_compilation.library);
    REQUIRE(normal.step(normal_compilation.library, normal_table, random) == givm::execution_state::finished);
    log.drawn.clear();
    log.card_counts_at_drawn.clear();

    givm::executor observed;
    observed.enter_entry(library);
    REQUIRE(observed.step(library, table, random) == givm::execution_state::finished);
    CHECK(player.deck_card_count() == 0);
    CHECK(player.hand_card_count() == 2);
    CHECK(log.drawn.size() == 2);
    std::vector<givm::hand_card_id> hand_ids;
    for(const auto card : player.hand_cards()) hand_ids.push_back(card.id());
    CHECK(log.drawn == hand_ids);
    std::vector<std::array<std::size_t, 2>> expected_counts(initial_hand_count, { initial_hand_count, 3 });
    if(initial_hand_count == 1) expected_counts.push_back({ 2, 0 });
    CHECK(log.card_counts_at_drawn == expected_counts);
    for(const auto card : log.drawn) CHECK(table[card].definition_id() == card_definition);
    CHECK(player.hand_card_count() == normal_table[givm::player_id{ 0 }].hand_card_count());
    CHECK(player.deck_card_count() == normal_table[givm::player_id{ 0 }].deck_card_count());
    CHECK(random.calls == 0);
}

TEST_CASE("single-player active-character observation precedes the table update and skips no-op logs", "[entity-observation][set_active_character]")
{
    entity_event_log log;
    const entity_observer_source observer_source{ &log };
    const givm::test::named_definition_source<givm::character_view> character_source{ "Character" };
    constexpr givm::character_id previous{ .player_id = givm::player_id{ 0 }, .index = 0 };
    constexpr givm::character_id current{ .player_id = givm::player_id{ 0 }, .index = 1 };
    const auto compile_program = [&](givm::compile_mode mode)
    {
        return givm::test::compile_definitions_with_program(
            mode,
            std::tuple{ givm::set_active_character{ previous }, givm::set_active_character{ current }, givm::set_active_character{ current } },
            std::tuple{ givm::end_game{ .result = givm::game_result::both_loss } }, observer_source, character_source
        );
    };
    const auto [library, ids] = compile_program(givm::compile_mode::observed);
    const auto normal_compilation = compile_program(givm::compile_mode::normal);
    givm::table table{};
    const auto definition = ids.get_id<givm::character_view>(character_source.name());
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer_source.name()), definition } }, {});

    auto normal_table = table;
    counting_random random;
    givm::executor normal;
    normal.enter_entry(normal_compilation.library);
    REQUIRE(normal.step(normal_compilation.library, normal_table, random) == givm::execution_state::finished);
    const std::vector normal_events(log.active.begin() + 1, log.active.end());
    log.active.clear();

    givm::executor observed;
    observed.enter_entry(library);
    REQUIRE(observed.step(library, table, random) == givm::execution_state::active_character_changed);
    CHECK(observed.view_in<givm::execution_state::active_character_changed>().character() == previous);
    REQUIRE(observed.step(library, table, random) == givm::execution_state::active_character_changed);
    log.active.clear();
    const auto view = observed.view_in<givm::execution_state::active_character_changed>();
    CHECK(view.character() == current);
    CHECK(table[givm::player_id{ 0 }].state().active_character == previous);
    CHECK(log.active.empty());

    REQUIRE(observed.step(library, table, random) == givm::execution_state::finished);
    CHECK(log.active == std::vector{ current, current });
    CHECK(log.active == normal_events);
    CHECK(table[givm::player_id{ 0 }].state().active_character == normal_table[givm::player_id{ 0 }].state().active_character);
}

TEST_CASE("initial active choices update both players before either response", "[entity-observation][select_active_character_both]")
{
    const auto first_player = GENERATE(givm::player_id{ 0 }, givm::player_id{ 1 });
    const auto behavior = GENERATE(0, 1, 2);
    entity_event_log log;
    const initial_switch_response_source observer{ &log, behavior };
    const givm::test::named_definition_source<givm::character_view> character_source{ "Character" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        givm::compile_mode::observed,
        std::tuple{ givm::select_active_character_both{} }, std::tuple{ givm::end_game{ .result = givm::game_result::both_loss } },
        observer, character_source
    );
    givm::table table{};
    const auto character = ids.get_id<givm::character_view>(character_source.name());
    load_deck(table, library,
        { .characters = { ids.get_id<givm::character_view>(observer.name()), character } },
        { .characters = { character, character } });

    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    REQUIRE(target.step(library, table, random) == givm::execution_state::initial_active_character_selection);
    const givm::character_id first_choice{ first_player, 1 };
    target.view_in<givm::execution_state::initial_active_character_selection>().select(first_choice);
    CHECK_FALSE(table[givm::player_id{ 0 }].state().active_character.has_value());
    CHECK_FALSE(table[givm::player_id{ 1 }].state().active_character.has_value());
    REQUIRE(log.active.empty());
    REQUIRE(target.step(library, table, random) == givm::execution_state::remaining_active_character_selection);
    const auto remaining = target.view_in<givm::execution_state::remaining_active_character_selection>();
    CHECK(remaining.first_selected_character() == first_choice);
    CHECK(remaining.player() == other_player(first_player));
    remaining.select(givm::character_id{ other_player(first_player), 1 });
    CHECK(remaining.first_selected_character() == first_choice);
    CHECK(remaining.player() == other_player(first_player));
    CHECK_FALSE(table[givm::player_id{ 0 }].state().active_character.has_value());
    CHECK_FALSE(table[givm::player_id{ 1 }].state().active_character.has_value());
    REQUIRE(log.active.empty());
    REQUIRE(target.step(library, table, random) == givm::execution_state::initial_active_characters_selected);
    CHECK(table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 1 });
    CHECK(table[givm::player_id{ 1 }].state().active_character == givm::character_id{ givm::player_id{ 1 }, 1 });
    REQUIRE(log.active.empty());

    auto copied = target;
    auto copied_table = table;
    const auto advance = [&](givm::executor& execution, givm::table& current_table)
    {
        log.active.clear();
        if(behavior == 2)
        {
            REQUIRE(execution.step(library, current_table, random) == givm::execution_state::active_character_changed);
            const auto changed = execution.view_in<givm::execution_state::active_character_changed>();
            CHECK(changed.character() == givm::character_id{ givm::player_id{ 1 }, 0 });
            CHECK(current_table[givm::player_id{ 1 }].state().active_character == givm::character_id{ givm::player_id{ 1 }, 1 });
            CHECK(log.active == std::vector{ givm::character_id{ givm::player_id{ 0 }, 1 } });
        }
        const auto state = execution.step(library, current_table, random);
        if(behavior == 0)
        {
            REQUIRE(state == givm::execution_state::finished);
            CHECK(log.active == std::vector{ givm::character_id{ givm::player_id{ 0 }, 1 }, givm::character_id{ givm::player_id{ 1 }, 1 } });
        }
        else
        {
            REQUIRE(state == givm::execution_state::finished);
            CHECK(execution.view_in<givm::execution_state::finished>().result() == givm::game_result::player_0_win);
            CHECK(log.active.front() == givm::character_id{ givm::player_id{ 0 }, 1 });
            CHECK(log.active.size() == static_cast<std::size_t>(behavior));
            if(behavior == 2)
                CHECK(current_table[givm::player_id{ 1 }].state().active_character == givm::character_id{ givm::player_id{ 1 }, 0 });
        }
    };
    advance(target, table);
    advance(copied, copied_table);
    CHECK(random.calls == 0);
}

TEST_CASE("resuming a switch applies it once before a nested switch response", "[entity-observation][set_active_character]")
{
    entity_event_log log;
    const switch_back_source response{ &log };
    const givm::test::named_definition_source<givm::character_view> character_source{ "Character" };
    constexpr givm::character_id previous{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id next{ givm::player_id{ 0 }, 1 };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        givm::compile_mode::observed,
        std::tuple{ givm::set_active_character{ previous }, givm::set_active_character{ next } }, std::tuple{ givm::end_game{ .result = givm::game_result::both_loss } },
        response, character_source
    );
    const auto normal_compilation = givm::test::compile_definitions_with_program(
        givm::compile_mode::normal,
        std::tuple{ givm::set_active_character{ previous }, givm::set_active_character{ next } }, std::tuple{ givm::end_game{ .result = givm::game_result::both_loss } },
        response, character_source
    );
    givm::table table{};
    const auto character = ids.get_id<givm::character_view>(character_source.name());
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(response.name()), character } }, {});
    auto normal_table = table;
    givm::executor normal;
    normal.enter_entry(normal_compilation.library);
    counting_random random;
    REQUIRE(normal.step(normal_compilation.library, normal_table, random) == givm::execution_state::finished);
    CHECK(log.active == std::vector{ previous, next, previous });
    log.active.clear();

    givm::executor target;
    target.enter_entry(library);
    REQUIRE(target.step(library, table, random) == givm::execution_state::active_character_changed);
    CHECK(target.view_in<givm::execution_state::active_character_changed>().character() == previous);
    REQUIRE(target.step(library, table, random) == givm::execution_state::active_character_changed);
    log.active.clear();
    CHECK(target.view_in<givm::execution_state::active_character_changed>().character() == next);
    CHECK(table[givm::player_id{ 0 }].state().active_character == previous);
    REQUIRE(log.active.empty());
    auto copied = target;
    auto copied_table = table;
    const auto resume = [&](givm::executor& execution, givm::table& current_table)
    {
        log.active.clear();
        REQUIRE(execution.step(library, current_table, random) == givm::execution_state::active_character_changed);
        CHECK(execution.view_in<givm::execution_state::active_character_changed>().character() == previous);
        CHECK(current_table[givm::player_id{ 0 }].state().active_character == next);
        CHECK(log.active == std::vector{ next });
        REQUIRE(execution.step(library, current_table, random) == givm::execution_state::finished);
        CHECK(current_table[givm::player_id{ 0 }].state().active_character == previous);
        CHECK(log.active == std::vector{ next, previous });
        CHECK(current_table[givm::player_id{ 0 }].state().active_character
            == normal_table[givm::player_id{ 0 }].state().active_character);
    };
    resume(target, table);
    resume(copied, copied_table);
}

TEST_CASE("replacing selected cards broadcasts the replacements before the next command", "[entity-observation][replace_cards][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool respond_to_draws = GENERATE(false, true);
    entity_event_log log;
    const entity_observer_source observer{ &log, respond_to_draws };
    const givm::test::named_definition_source<givm::card_definition> first{ "FirstCard" };
    const givm::test::named_definition_source<givm::card_definition> second{ "SecondCard" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        mode,
        std::tuple{
            givm::draw_cards{ .count = 2 },
            givm::replace_cards{ .player = givm::player_id{ 0 } },
            givm::draw_cards{ .count = 1 },
            givm::end_game{ givm::game_result::both_loss }
        }, std::tuple{}, observer, first, second
    );
    const auto first_id = ids.get_id<givm::card_definition>(first.name());
    const auto second_id = ids.get_id<givm::card_definition>(second.name());
    givm::table table;
    load_deck(table, library,
        { .cards = { second_id, second_id, second_id, second_id, first_id, first_id } },
        { .characters = { ids.get_id<givm::character_view>(observer.name()) } });
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    const auto step_until_input = [&]
    {
        auto state = target.step(library, table, random);
        while(state == givm::execution_state::round_started)
            state = target.step(library, table, random);
        return state;
    };
    REQUIRE(step_until_input() == givm::execution_state::card_selection);
    REQUIRE(log.drawn.size() == 2);
    CHECK(log.card_counts_at_drawn == std::vector<std::array<std::size_t, 2>>{ { 2, 4 }, { 2, 4 } });
    for(const auto card : table[givm::player_id{ 0 }].hand_cards()) CHECK(card.definition_id() == first_id);
    log.drawn.clear();
    log.card_counts_at_drawn.clear();

    target.view_in<givm::execution_state::card_selection>().select(std::bitset<givm::selection_capacity>{ 0b11 });
    REQUIRE(step_until_input() == givm::execution_state::finished);
    REQUIRE(log.drawn.size() == 3);
    std::vector<givm::hand_card_id> hand;
    for(const auto card : table[givm::player_id{ 0 }].hand_cards())
    {
        CHECK(card.definition_id() == second_id);
        hand.push_back(card.id());
    }
    CHECK(log.drawn == hand);
    CHECK(log.card_counts_at_drawn == std::vector<std::array<std::size_t, 2>>{ { 2, 4 }, { 2, 4 }, { 3, 3 } });
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 3);
    CHECK(table[givm::player_id{ 0 }].deck_card_count() == 3);
    CHECK(table.state().round_number == (respond_to_draws ? 5 : 0));
    CHECK(random.calls == 2);
}
