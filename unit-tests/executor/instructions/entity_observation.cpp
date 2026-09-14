#include "../../executor_access.hpp"
#include <array>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"

using namespace givm;

namespace
{
    struct stop_entity_program
    {
        using context_type = void;

        execution_state execute(const definition_library&, detail::unrestricted_table&, detail::execution_context&, random_fn&) const noexcept
        {
            return execution_state::action;
        }
    };

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
        using definition_category = character_view;
        struct definition_type {};

        constexpr std::string_view name() const noexcept { return "ObservedCharacter"; }
        constexpr definition_type compile(definition_compile_context&) const noexcept { return {}; }

        static handler_program_entry_t<character_initialization> handle(
            const definition_type&, const character_view&, character_initialization& event,
            const card_table&, random_fn& random
        )
        {
            const auto health = 10 + random();
            event.state = { .max_health = health, .max_energy = 3, .health = health, .energy = 1 };
            return handler_program_entry_t<character_initialization>::null();
        }
    };

    struct creation_program_source
    {
        using definition_category = support_view;
        struct definition_type { program_entry<test_event> entry; };

        constexpr std::string_view name() const noexcept { return "CreationProgram"; }
        constexpr auto character_dependencies() const noexcept
        {
            return std::array{ std::string_view{ "ObservedCharacter" } };
        }
        constexpr auto card_dependencies() const noexcept
        {
            return std::array{ std::string_view{ "ObservedCard" } };
        }

        definition_type compile(definition_compile_context& context) const
        {
            const auto character = context.resolve_id<character_view>("ObservedCharacter");
            const auto card = context.resolve_id<card_definition>("ObservedCard");
            return { context.add_program<test_event>(std::tuple{
                enter_character{ .player = player_id{ 1 }, .definition = character },
                insert_deck_card{ .player = player_id{ 1 }, .definition = card },
                insert_deck_card{ .player = player_id{ 1 }, .definition = card, .position = 0 }
            }) };
        }

        static program_entry<test_event> handle(
            const definition_type& data, const support_view&, test_event&, const card_table&, random_fn&
        )
        {
            return data.entry;
        }
    };

    struct entity_event_log
    {
        std::vector<hand_card_id> drawn;
        std::vector<character_id> active;
    };

    struct empty_program_source
    {
        using definition_category = support_view;
        struct definition_type
        {
            program_entry<test_event> entry;
            std::uint32_t* calls;
        };
        std::uint32_t* calls;

        constexpr std::string_view name() const noexcept { return "EmptyProgram"; }

        definition_type compile(definition_compile_context& context) const
        {
            return { context.add_program<test_event>(std::tuple{}), calls };
        }

        static program_entry<test_event> handle(
            const definition_type& data, const support_view&, test_event&, const card_table&, random_fn&
        )
        {
            CHECK_FALSE(data.entry.is_null());
            ++*data.calls;
            return data.entry;
        }
    };

    struct entity_observer_source
    {
        using definition_category = support_view;
        struct definition_type { entity_event_log* log; };
        entity_event_log* log;

        constexpr std::string_view name() const noexcept { return "EntityObserver"; }
        constexpr definition_type compile(definition_compile_context&) const noexcept { return { log }; }

        static program_entry<card_drawn> handle(
            const definition_type& data, const support_view&, card_drawn& event,
            const card_table&, random_fn&
        )
        {
            data.log->drawn.push_back(event.card);
            return program_entry<card_drawn>::null();
        }

        static program_entry<active_character_changed> handle(
            const definition_type& data, const support_view&, active_character_changed& event,
            const card_table& table, random_fn&
        )
        {
            CHECK(table[event.current.player_id].state().active_character == event.current);
            data.log->active.push_back(event.current);
            return program_entry<active_character_changed>::null();
        }
    };

    struct initial_switch_response_source
    {
        using definition_category = support_view;
        struct definition_type
        {
            entity_event_log* log;
            program_entry<active_character_changed> entry;
        };
        entity_event_log* log;
        int behavior;

        constexpr std::string_view name() const noexcept { return "InitialSwitchResponse"; }
        definition_type compile(definition_compile_context& context) const
        {
            if(behavior == 1)
                return { log, context.add_program<active_character_changed>(std::tuple{
                    end_game{ .result = game_result::player_0_win }
                }) };
            if(behavior == 2)
                return { log, context.add_program<active_character_changed>(std::tuple{
                    set_active_character{ .target = character_id{ player_id{ 1 }, 0 } },
                    end_game{ .result = game_result::player_0_win }
                }) };
            return { log, program_entry<active_character_changed>::null() };
        }

        static program_entry<active_character_changed> handle(
            const definition_type& data, const support_view&, active_character_changed& event,
            const card_table& table, random_fn&
        )
        {
            CHECK(table[event.current.player_id].state().active_character == event.current);
            data.log->active.push_back(event.current);
            return event.current.player_id == player_id{ 0 }
                ? data.entry : program_entry<active_character_changed>::null();
        }
    };

    struct switch_back_source
    {
        using definition_category = support_view;
        struct definition_type
        {
            entity_event_log* log;
            program_entry<active_character_changed> entry;
        };
        entity_event_log* log;

        constexpr std::string_view name() const noexcept { return "SwitchBack"; }
        definition_type compile(definition_compile_context& context) const
        {
            return { log, context.add_program<active_character_changed>(std::tuple{
                set_active_character{ .target = character_id{ player_id{ 0 }, 0 } }
            }) };
        }
        static program_entry<active_character_changed> handle(
            const definition_type& data, const support_view&, active_character_changed& event,
            const card_table& table, random_fn&
        )
        {
            CHECK(table[event.current.player_id].state().active_character == event.current);
            data.log->active.push_back(event.current);
            return event.current.index == 1 ? data.entry : program_entry<active_character_changed>::null();
        }
    };

}

TEST_CASE("step passes through creation responses and preserves initialization", "[entity-observation]")
{
    const initialized_character_source character_source;
    const test::named_definition_source<card_definition> card_source{ "ObservedCard" };
    const creation_program_source program_source;
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{ test_command{} }, std::tuple{ stop_entity_program{} },
        program_source, card_source, character_source
    );
    card_table table{};
    auto& mutable_table = detail::executor_access::unrestricted(table);
    mutable_table[player_id{ 0 }].add(ids.get_id<support_view>(program_source.name()), { .count = 1 });
    auto normal_table = table;
    counting_random normal_random;
    executor normal;
    normal.enter_entry(library);
    REQUIRE(normal.run(library, normal_table, normal_random) == execution_state::action);

    counting_random random;
    executor observed;
    observed.enter_entry(library);
    REQUIRE(observed.step(library, table, random) == execution_state::action);
    const character_id character{ .player_id = player_id{ 1 }, .index = 0 };
    CHECK(table[character].definition_id().value() == ids.get_id<character_view>(character_source.name()).value());
    CHECK(table[character].state().max_health == 12);
    CHECK(table[character].state().health == 12);
    CHECK(table[character].state().energy == 1);
    CHECK(table[player_id{ 1 }].deck_card_count() == 2);
    const auto cards = mutable_table[player_id{ 1 }].deck_cards<false>();
    CHECK(cards.front().id().index == 1);
    CHECK(cards.back().id().index == 0);
    CHECK(random.calls == normal_random.calls);
    CHECK(random.calls == 1);
    CHECK(table[character].state().health == normal_table[character].state().health);
    CHECK(table[player_id{ 1 }].deck_card_count() == normal_table[player_id{ 1 }].deck_card_count());
    CHECK(detail::executor_access::stack(observed).size() == detail::executor_access::stack(normal).size());
}

TEST_CASE("step passes through an empty response without an observation", "[entity-observation]")
{
    std::uint32_t handler_calls = 0;
    const empty_program_source source{ &handler_calls };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{ test_command{} }, std::tuple{ stop_entity_program{} }, source
    );
    card_table table{};
    auto& mutable_table = detail::executor_access::unrestricted(table);
    const auto entity = mutable_table[player_id{ 0 }].add(ids.get_id<support_view>(source.name()), { .count = 1 }).id();
    auto normal_table = table;
    counting_random random;
    executor normal;
    normal.enter_entry(library);
    REQUIRE(normal.run(library, normal_table, random) == execution_state::action);
    REQUIRE(handler_calls == 1);
    handler_calls = 0;

    executor observed;
    observed.enter_entry(library);
    REQUIRE(observed.step(library, table, random) == execution_state::action);
    CHECK(handler_calls == 1);
    CHECK(detail::executor_access::stack(observed).size() == detail::executor_access::stack(normal).size());
    CHECK(random.calls == 0);
}

TEST_CASE("step passes through draws and full-hand discards while preserving broadcasts", "[entity-observation][draw_cards]")
{
    std::size_t initial_hand_count = 1;
    SECTION("one card fits before the hand becomes full") {}
    SECTION("all cards are discarded") { initial_hand_count = 2; }

    entity_event_log log;
    const entity_observer_source observer_source{ &log };
    const test::named_definition_source<card_definition> card_source{ "ObservedCard" };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{
            draw_cards{ .count = 5, .player = relative_player::other },
            draw_cards{ .count = 1, .player = relative_player::other }
        },
        std::tuple{ stop_entity_program{} }, observer_source, card_source
    );
    card_table table{ { .hand_limit = 2 } };
    auto& mutable_table = detail::executor_access::unrestricted(table);
    mutable_table.state().active_player = player_id{ 1 };
    mutable_table[player_id{ 1 }].add(ids.get_id<support_view>(observer_source.name()), { .count = 1 });
    const auto card_definition = ids.get_id<givm::card_definition>(card_source.name());
    auto player = mutable_table[player_id{ 0 }];
    for(std::size_t index = 0; index < initial_hand_count; ++index)
        player.add_hand_card(card_definition, {});
    std::array<deck_card_id, 3> deck;
    for(auto& card : deck)
        card = player.add_deck_card(card_definition, {}).id();
    player.swap_deck_cards(0, 2);

    auto normal_table = table;
    counting_random random;
    executor normal;
    normal.enter_entry(library);
    REQUIRE(normal.run(library, normal_table, random) == execution_state::action);
    const auto normal_drawn = log.drawn;
    log.drawn.clear();

    executor observed;
    observed.enter_entry(library);
    REQUIRE(observed.step(library, table, random) == execution_state::action);
    CHECK(player.deck_card_count() == 0);
    CHECK(player.hand_card_count() == 2);
    CHECK(log.drawn.size() == 2 - initial_hand_count);
    if(initial_hand_count == 1)
        CHECK(log.drawn.front() == hand_card_id{ .player_id = player_id{ 0 }, .index = 1 });
    for(const auto card : deck)
        CHECK_FALSE(static_cast<bool>(table[card]));
    CHECK(log.drawn == normal_drawn);
    CHECK(player.hand_card_count() == normal_table[player_id{ 0 }].hand_card_count());
    CHECK(player.deck_card_count() == normal_table[player_id{ 0 }].deck_card_count());
    CHECK(detail::executor_access::stack(observed).size() == detail::executor_access::stack(normal).size());
    CHECK(random.calls == 0);
}

TEST_CASE("single-player active-character observation precedes the table update and skips no-op logs", "[entity-observation][set_active_character]")
{
    entity_event_log log;
    const entity_observer_source observer_source{ &log };
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr character_id previous{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id current{ .player_id = player_id{ 0 }, .index = 1 };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{ set_active_character{ current }, set_active_character{ current } },
        std::tuple{ stop_entity_program{} }, observer_source, character_source
    );
    card_table table{};
    auto& mutable_table = detail::executor_access::unrestricted(table);
    const auto definition = ids.get_id<character_view>(character_source.name());
    mutable_table[player_id{ 0 }].add(ids.get_id<support_view>(observer_source.name()), { .count = 1 });
    mutable_table[player_id{ 0 }].add(definition, { .max_health = 10, .health = 10 });
    mutable_table[player_id{ 0 }].add(definition, { .max_health = 10, .health = 10 });
    mutable_table[player_id{ 0 }].state().active_character = previous;

    auto normal_table = table;
    counting_random random;
    executor normal;
    normal.enter_entry(library);
    REQUIRE(normal.run(library, normal_table, random) == execution_state::action);
    const auto normal_events = log.active;
    log.active.clear();

    executor observed;
    observed.enter_entry(library);
    REQUIRE(observed.step(library, table, random) == execution_state::active_character_changed);
    const auto view = observed.view_in<execution_state::active_character_changed>();
    CHECK(view.character() == current);
    auto original_frame = detail::executor_access::stack(observed);
    original_frame.pop<detail::handler_id<active_character_changed>[], stack_count_t,
        active_character_changed, detail::handler_id<active_character_changed>, detail::stage_t>();
    frame_stack initial_frame;
    initial_frame.push(detail::stage_t{});
    CHECK(original_frame.size() == initial_frame.size());
    CHECK(table[player_id{ 0 }].state().active_character == previous);
    CHECK(log.active.empty());

    REQUIRE(observed.step(library, table, random) == execution_state::action);
    CHECK(log.active == std::vector{ current, current });
    CHECK(log.active == normal_events);
    CHECK(table[player_id{ 0 }].state().active_character == normal_table[player_id{ 0 }].state().active_character);
    CHECK(detail::executor_access::stack(observed).size() == detail::executor_access::stack(normal).size());
}

TEST_CASE("initial active choices share their original frame before either response", "[entity-observation][select_active_character_both]")
{
    const auto first_player = GENERATE(player_id{ 0 }, player_id{ 1 });
    const auto behavior = GENERATE(0, 1, 2);
    entity_event_log log;
    const initial_switch_response_source observer{ &log, behavior };
    const test::named_definition_source<character_view> character_source{ "Character" };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{ select_active_character_both{} }, std::tuple{ stop_entity_program{} },
        observer, character_source
    );
    card_table table{};
    auto& mutable_table = detail::executor_access::unrestricted(table);
    mutable_table[player_id{ 0 }].add(ids.get_id<support_view>(observer.name()), { .count = 1 });
    const auto character = ids.get_id<character_view>(character_source.name());
    for(auto player : mutable_table.players())
    {
        player.add(character, { .max_health = 10, .health = 10 });
        player.add(character, { .max_health = 10, .health = 10 });
    }

    executor target;
    target.enter_entry(library);
    counting_random random;
    REQUIRE(target.step(library, table, random) == execution_state::initial_active_character_selection);
    const auto original_size = detail::executor_access::stack(target).size();
    const character_id first_choice{ first_player, 1 };
    target.view_in<execution_state::initial_active_character_selection>().select(first_choice);
    CHECK(detail::executor_access::stack(target).size() == original_size);
    CHECK_FALSE(table[player_id{ 0 }].state().active_character.has_value());
    CHECK_FALSE(table[player_id{ 1 }].state().active_character.has_value());
    REQUIRE(log.active.empty());
    REQUIRE(target.step(library, table, random) == execution_state::remaining_active_character_selection);
    CHECK(detail::executor_access::stack(target).size() == original_size);
    const auto remaining = target.view_in<execution_state::remaining_active_character_selection>();
    CHECK(remaining.selected() == first_choice);
    CHECK(remaining.player() == other_player(first_player));
    remaining.select(1);
    CHECK(remaining.selected() == first_choice);
    CHECK(remaining.player() == other_player(first_player));
    CHECK_FALSE(table[player_id{ 0 }].state().active_character.has_value());
    CHECK_FALSE(table[player_id{ 1 }].state().active_character.has_value());
    REQUIRE(log.active.empty());
    REQUIRE(target.step(library, table, random) == execution_state::initial_active_characters_selected);
    CHECK(detail::executor_access::stack(target).size() == original_size);
    const auto view = target.view_in<execution_state::initial_active_characters_selected>();
    static_assert(std::is_empty_v<decltype(view)>);
    CHECK(table[player_id{ 0 }].state().active_character == character_id{ player_id{ 0 }, 1 });
    CHECK(table[player_id{ 1 }].state().active_character == character_id{ player_id{ 1 }, 1 });
    REQUIRE(log.active.empty());

    auto copied = target;
    auto copied_table = table;
    const auto copied_view = copied.view_in<execution_state::initial_active_characters_selected>();
    static_assert(std::is_empty_v<decltype(copied_view)>);
    const auto advance = [&](executor& execution, card_table& current_table)
    {
        log.active.clear();
        if(behavior == 2)
        {
            REQUIRE(execution.step(library, current_table, random) == execution_state::active_character_changed);
            const auto changed = execution.view_in<execution_state::active_character_changed>();
            CHECK(changed.character() == character_id{ player_id{ 1 }, 0 });
            CHECK(current_table[player_id{ 1 }].state().active_character == character_id{ player_id{ 1 }, 1 });
            CHECK(log.active == std::vector{ character_id{ player_id{ 0 }, 1 } });
        }
        const auto state = execution.step(library, current_table, random);
        if(behavior == 0)
        {
            REQUIRE(state == execution_state::action);
            CHECK(log.active == std::vector{ character_id{ player_id{ 0 }, 1 }, character_id{ player_id{ 1 }, 1 } });
        }
        else
        {
            REQUIRE(state == execution_state::finished);
            CHECK(execution.view_in<execution_state::finished>().result() == game_result::player_0_win);
            CHECK(log.active.front() == character_id{ player_id{ 0 }, 1 });
            CHECK(log.active.size() == static_cast<std::size_t>(behavior));
            if(behavior == 2)
                CHECK(current_table[player_id{ 1 }].state().active_character == character_id{ player_id{ 1 }, 0 });
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
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr character_id previous{ player_id{ 0 }, 0 };
    constexpr character_id next{ player_id{ 0 }, 1 };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{ set_active_character{ next } }, std::tuple{ stop_entity_program{} },
        response, character_source
    );
    card_table table{};
    auto& mutable_table = detail::executor_access::unrestricted(table);
    const auto character = ids.get_id<character_view>(character_source.name());
    mutable_table[player_id{ 0 }].add(character, { .max_health = 10, .health = 10 });
    mutable_table[player_id{ 0 }].add(character, { .max_health = 10, .health = 10 });
    mutable_table[player_id{ 0 }].add(ids.get_id<support_view>(response.name()), { .count = 1 });
    mutable_table[player_id{ 0 }].state().active_character = previous;
    auto normal_table = table;
    executor normal;
    normal.enter_entry(library);
    counting_random random;
    REQUIRE(normal.run(library, normal_table, random) == execution_state::action);
    CHECK(log.active == std::vector{ next, previous });
    log.active.clear();

    executor target;
    target.enter_entry(library);
    REQUIRE(target.step(library, table, random) == execution_state::active_character_changed);
    CHECK(target.view_in<execution_state::active_character_changed>().character() == next);
    CHECK(table[player_id{ 0 }].state().active_character == previous);
    REQUIRE(log.active.empty());
    auto copied = target;
    auto copied_table = table;
    const auto resume = [&](executor& execution, card_table& current_table)
    {
        log.active.clear();
        REQUIRE(execution.step(library, current_table, random) == execution_state::active_character_changed);
        CHECK(execution.view_in<execution_state::active_character_changed>().character() == previous);
        CHECK(current_table[player_id{ 0 }].state().active_character == next);
        CHECK(log.active == std::vector{ next });
        REQUIRE(execution.step(library, current_table, random) == execution_state::action);
        CHECK(current_table[player_id{ 0 }].state().active_character == previous);
        CHECK(log.active == std::vector{ next, previous });
        CHECK(current_table[player_id{ 0 }].state().active_character
            == normal_table[player_id{ 0 }].state().active_character);
        CHECK(detail::executor_access::stack(execution).size() == detail::executor_access::stack(normal).size());
    };
    resume(target, table);
    resume(copied, copied_table);
}
