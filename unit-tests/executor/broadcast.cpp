#include "../test_source_library.hpp"

#include <cstdint>
#include <span>
#include <string_view>
#include <tuple>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../table/test_definition_library.hpp"

namespace
{
    struct response_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            std::vector<givm::character_id>* handlers;
            givm::program_entry entry;
        };
        std::string_view source_name;
        std::vector<givm::character_id>* handlers;
        bool terminal;
        std::string_view name() const noexcept { return source_name; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            if(terminal)
                return { handlers, context.add_program(std::tuple{
                    givm::end_game{ .result = givm::game_result::player_1_win }
                }) };
            return { handlers, context.add_program(std::tuple{
                givm::set_element_aura{ .target = givm::character_id{ givm::player_id{ 0 }, 0 }, .aura = givm::element_aura::hydro }
            }) };
        }
        static givm::program_entry handle(
            const definition_type& data, const givm::character_view& self, givm::test_event&,
            givm::handle_context& context)
        {
            if(not data.handlers->empty())
                CHECK(context.table()[givm::character_id{ givm::player_id{ 0 }, 0 }].state().aura == givm::element_aura::hydro);
            data.handlers->push_back(self.id());
            return context.invoke(data.entry);
        }
    };
    struct nested_selection_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            std::vector<givm::hand_card_id>* drawn;
            givm::program_entry draw_entry;
            givm::program_entry selection_entry;
        };
        std::vector<givm::hand_card_id>* drawn;

        std::string_view name() const noexcept { return "NestedSelection"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return {
                drawn,
                context.add_program(std::tuple{ givm::draw_cards{ .count = 2 } }),
                context.add_program(std::tuple{
                    givm::replace_cards{ .player = givm::player_id{ 0 } },
                    givm::start_round{ .max_rounds = 10 }
                })
            };
        }

        static givm::program_entry handle(
            const definition_type& data, const givm::character_view&, givm::test_event&,
            givm::handle_context& context)
        {
            return context.invoke(data.draw_entry);
        }

        static givm::program_entry handle(
            const definition_type& data, const givm::character_view&, givm::card_drawn& event,
            givm::handle_context& context)
        {
            data.drawn->push_back(event.card);
            return context.invoke(data.selection_entry, std::span<const unsigned char>{});
        }
    };
    struct context_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            std::vector<std::uint32_t>* samples;
            givm::program_entry entry;
        };
        std::vector<std::uint32_t>* samples;

        std::string_view name() const noexcept { return "HandleContext"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { samples, context.add_program(std::tuple{ givm::draw_cards{ .count = 1 } }) };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
                           givm::test_event&, givm::handle_context& context)
        {
            CHECK(context.table()[self.player().id()].deck_card_count() == 1);
            data.samples->push_back(context.random());
            data.samples->push_back(context.random());
            return context.invoke(data.entry);
        }
    };

    struct mixed_response_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            std::vector<givm::character_id>* handlers;
            std::uint32_t* nested_responses;
            givm::program_entry empty;
            givm::program_entry change_active;
            givm::program_entry draw;
        };
        std::vector<givm::character_id>* handlers;
        std::uint32_t* nested_responses;

        std::string_view name() const noexcept { return "MixedResponses"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { handlers, nested_responses,
                context.add_program(std::tuple{}),
                context.add_program(std::tuple{ givm::set_active_character{} }),
                context.add_program(std::tuple{ givm::draw_cards{ .count = 1 } }) };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
                                          givm::test_event&, givm::handle_context& context)
        {
            data.handlers->push_back(self.id());
            switch(self.id().index)
            {
            case 0: return {};
            case 1: return context.invoke(data.empty);
            case 2: return context.invoke(data.change_active, givm::active_character_changed{ .current = self.id() });
            default: return context.invoke(data.draw);
            }
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
                                          givm::card_drawn&, givm::handle_context& context)
        {
            ++*data.nested_responses;
            return context.invoke(data.empty, std::span<const unsigned char>{});
        }
    };

    struct zero_random { std::uint32_t operator()() const noexcept { return 0; } };
}

TEST_CASE("handle context exposes the current table random source and invocation", "[broadcast][handle-context]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    std::vector<std::uint32_t> samples;
    const context_source source{ &samples };
    const givm::test::named_definition_source<givm::card_definition> card{ "ContextCard" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::test_command{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, source, card);
    givm::table table;
    load_deck(table, library, {
        .cards = { ids.get_id<givm::card_definition>(card.name()) },
        .characters = { ids.get_id<givm::character_view>(source.name()) }
    }, {});
    struct sequential_random
    {
        std::uint32_t next = 17;
        std::uint32_t operator()() noexcept { return next++; }
    } random;
    givm::executor execution;
    execution.enter_entry(library);
    REQUIRE(execution.step(library, table, random) == givm::execution_state::finished);
    CHECK(samples == std::vector<std::uint32_t>{ 17, 18 });
    CHECK(random.next == 19);
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 1);
    CHECK(table[givm::player_id{ 0 }].deck_card_count() == 0);
}

TEST_CASE("broadcast responses finish before the next handler and may end the game", "[broadcast]")
{
    const bool observed = GENERATE(false, true);
    const bool terminal = GENERATE(false, true);
    std::vector<givm::character_id> handlers;
    const response_source first{ "First", &handlers, terminal };
    const response_source second{ "Second", &handlers, false };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(first, second));
    const auto [library, ids] = compile(sources,
        std::tuple{ givm::test_command{}, givm::end_game{ .result = givm::game_result::both_loss } }, std::tuple{}, observed ? givm::compile_mode::observed : givm::compile_mode::normal
    );
    givm::table table;
    load_deck(table, library, { .characters = {
        ids.get_id<givm::character_view>(first.name()), ids.get_id<givm::character_view>(second.name())
    } }, {});
    givm::executor target;
    target.enter_entry(library);
    zero_random random;
    REQUIRE(target.step(library, table, random)
        == givm::execution_state::finished);
    if(terminal)
    {
        CHECK(handlers == std::vector{ givm::character_id{ givm::player_id{ 0 }, 0 } });
        CHECK(target.view_in<givm::execution_state::finished>().result() == givm::game_result::player_1_win);
    }
    else
    {
        CHECK(handlers == std::vector{ givm::character_id{ givm::player_id{ 0 }, 0 }, givm::character_id{ givm::player_id{ 0 }, 1 } });
        CHECK(target.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
    }
}

TEST_CASE("nested input resumes after library copies and moves in both compile modes", "[broadcast][bytecode][compile-mode][raw-input]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    std::vector<givm::hand_card_id> drawn;
    const nested_selection_source source{ &drawn };
    const givm::test::named_definition_source<givm::card_definition> card{ "NestedCard" };
    auto [library, ids] = givm::test::compile_definitions_with_program(
        mode,
        std::tuple{
            givm::test_command{},
            givm::draw_cards{ .count = 1 },
            givm::end_game{ givm::game_result::both_loss }
        }, std::tuple{}, source, card
    );
    givm::table table;
    const auto card_id = ids.get_id<givm::card_definition>(card.name());
    load_deck(table, library, {
        .cards = { card_id, card_id, card_id },
        .characters = { ids.get_id<givm::character_view>(source.name()) }
    }, {});
    givm::executor target;
    target.enter_entry(library);
    zero_random random;
    REQUIRE(target.step(library, table, random) == givm::execution_state::card_selection);
    REQUIRE(drawn.size() == 1);
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 2);

    auto copied_library = library;
    auto assigned_library = copied_library;
    assigned_library = library;
    auto moved_library = std::move(library);
    auto move_assigned_library = copied_library;
    auto assignment_source = copied_library;
    move_assigned_library = std::move(assignment_source);
    auto copied_target = target;
    auto copied_table = table;
    const auto prefix = drawn;
    const auto resume = [&](givm::executor& execution, givm::table& current_table,
                            const givm::definition_library& current_library)
    {
        drawn = prefix;
        for(std::uint32_t index = 0; index < 3; ++index)
        {
            REQUIRE(drawn.size() == index + 1);
            CHECK(execution.view_in<givm::execution_state::card_selection>().player() == givm::player_id{ 0 });
            execution.view_in<givm::execution_state::card_selection>().select({});
            auto state = execution.step(current_library, current_table, random);
            if(mode == givm::compile_mode::observed)
            {
                REQUIRE(state == givm::execution_state::round_started);
                CHECK(current_table.state().round_number == index + 1);
                state = execution.step(current_library, current_table, random);
            }
            REQUIRE(state == (index == 2 ? givm::execution_state::finished : givm::execution_state::card_selection));
        }
        CHECK(current_table.state().round_number == 3);
        CHECK(current_table[givm::player_id{ 0 }].hand_card_count() == 3);
        CHECK(current_table[givm::player_id{ 0 }].deck_card_count() == 0);
        CHECK(execution.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
        std::vector<givm::hand_card_id> hand;
        for(const auto entity : current_table[givm::player_id{ 0 }].hand_cards()) hand.push_back(entity.id());
        CHECK(drawn == hand);
    };
    resume(target, table, moved_library);
    const auto first_events = drawn;
    for(const auto* current_library : { &copied_library, &assigned_library, &move_assigned_library })
    {
        auto execution_copy = copied_target;
        auto table_copy = copied_table;
        resume(execution_copy, table_copy, *current_library);
        CHECK(drawn == first_events);
    }
}

TEST_CASE("consecutive broadcasts mix missing empty and parameterized response programs", "[broadcast][program-input][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    std::vector<givm::character_id> handlers;
    std::uint32_t nested_responses = 0;
    const mixed_response_source source{ &handlers, &nested_responses };
    const givm::test::named_definition_source<givm::card_definition> card{ "MixedResponseCard" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::test_command{}, givm::test_command{}, givm::draw_cards{ .count = 1 },
                    givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, source, card);
    const auto character_id = ids.get_id<givm::character_view>(source.name());
    const auto card_id = ids.get_id<givm::card_definition>(card.name());
    givm::table table;
    load_deck(table, library, {
        .cards = { card_id, card_id, card_id },
        .characters = { character_id, character_id, character_id, character_id }
    }, {});
    givm::executor execution;
    execution.enter_entry(library);
    zero_random random;
    auto state = execution.step(library, table, random);
    while(state == givm::execution_state::active_character_changed)
        state = execution.step(library, table, random);
    REQUIRE(state == givm::execution_state::finished);
    CHECK(handlers == std::vector<givm::character_id>{
        { givm::player_id{ 0 }, 0 }, { givm::player_id{ 0 }, 1 },
        { givm::player_id{ 0 }, 2 }, { givm::player_id{ 0 }, 3 },
        { givm::player_id{ 0 }, 0 }, { givm::player_id{ 0 }, 1 },
        { givm::player_id{ 0 }, 2 }, { givm::player_id{ 0 }, 3 }
    });
    CHECK(nested_responses == 12);
    CHECK(table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 2 });
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 3);
    CHECK(table[givm::player_id{ 0 }].deck_card_count() == 0);
    CHECK(execution.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
}
