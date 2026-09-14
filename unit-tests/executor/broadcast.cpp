#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

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
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::test_command{}, givm::end_game{ .result = givm::game_result::both_loss } }, std::tuple{}
    );
    givm::table table;
    table.load_deck(givm::player_id{ 0 }, { .characters = {
        ids.get_id<givm::character_view>(first.name()), ids.get_id<givm::character_view>(second.name())
    } });
    givm::executor target;
    target.enter_entry(library);
    zero_random random;
    REQUIRE((observed ? target.step(library, table, random) : target.run(library, table, random))
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
