#include <cstdint>
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
            givm::program_entry<givm::test_event> entry;
        };
        std::string_view source_name;
        std::vector<givm::character_id>* handlers;
        bool terminal;
        std::string_view name() const noexcept { return source_name; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            if(terminal)
                return { handlers, context.add_program<givm::test_event>(std::tuple{
                    givm::end_game{ .result = givm::game_result::player_1_win }
                }) };
            return { handlers, context.add_program<givm::test_event>(std::tuple{
                givm::set_element_aura{ .target = givm::character_id{ givm::player_id{ 0 }, 0 }, .aura = givm::element_aura::hydro }
            }) };
        }
        static givm::program_entry<givm::test_event> handle(
            const definition_type& data, const givm::character_view& self, givm::test_event&,
            const givm::table& table, givm::random_fn&)
        {
            if(not data.handlers->empty())
                CHECK(table[givm::character_id{ givm::player_id{ 0 }, 0 }].state().aura == givm::element_aura::hydro);
            data.handlers->push_back(self.id());
            return data.entry;
        }
    };
    struct nested_selection_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            std::vector<givm::hand_card_id>* drawn;
            givm::program_entry<givm::test_event> draw_entry;
            givm::program_entry<givm::card_drawn> selection_entry;
        };
        std::vector<givm::hand_card_id>* drawn;

        std::string_view name() const noexcept { return "NestedSelection"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return {
                drawn,
                context.add_program<givm::test_event>(std::tuple{ givm::draw_cards{ .count = 2 } }),
                context.add_program<givm::card_drawn>(std::tuple{
                    givm::replace_cards{ .player = givm::player_id{ 0 } },
                    givm::start_round{ .max_rounds = 10 }
                })
            };
        }

        static givm::program_entry<givm::test_event> handle(
            const definition_type& data, const givm::character_view&, givm::test_event&,
            const givm::table&, givm::random_fn&)
        {
            return data.draw_entry;
        }

        static givm::program_entry<givm::card_drawn> handle(
            const definition_type& data, const givm::character_view&, givm::card_drawn& event,
            const givm::table&, givm::random_fn&)
        {
            data.drawn->push_back(event.card);
            return data.selection_entry;
        }
    };
    struct zero_random { std::uint32_t operator()() const noexcept { return 0; } };
}

TEST_CASE("broadcast responses finish before the next handler and may end the game", "[broadcast]")
{
    const bool observed = GENERATE(false, true);
    const bool terminal = GENERATE(false, true);
    std::vector<givm::character_id> handlers;
    const response_source first{ "First", &handlers, terminal };
    const response_source second{ "Second", &handlers, false };
    givm::definition_source_library sources;
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

TEST_CASE("nested input resumes after library copies and moves in both compile modes", "[broadcast][bytecode][compile-mode]")
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
