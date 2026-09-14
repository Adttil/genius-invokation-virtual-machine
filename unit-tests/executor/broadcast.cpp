#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

using namespace givm;

namespace
{
    struct response_source
    {
        using definition_category = character_view;
        struct definition_type
        {
            std::vector<character_id>* handlers;
            program_entry<test_event> entry;
        };
        std::string_view source_name;
        std::vector<character_id>* handlers;
        bool terminal;
        std::string_view name() const noexcept { return source_name; }
        definition_type compile(definition_compile_context& context) const
        {
            if(terminal)
                return { handlers, context.add_program<test_event>(std::tuple{
                    end_game{ .result = game_result::player_1_win }
                }) };
            return { handlers, context.add_program<test_event>(std::tuple{
                set_element_aura{ .target = character_id{ player_id{ 0 }, 0 }, .aura = element_aura::hydro }
            }) };
        }
        static program_entry<test_event> handle(
            const definition_type& data, const character_view& self, test_event&,
            const card_table& table, random_fn&)
        {
            if(not data.handlers->empty())
                CHECK(table[character_id{ player_id{ 0 }, 0 }].state().aura == element_aura::hydro);
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
    std::vector<character_id> handlers;
    const response_source first{ "First", &handlers, terminal };
    const response_source second{ "Second", &handlers, false };
    definition_source_library sources;
    REQUIRE(sources.add(first, second));
    const auto [library, ids] = sources.compile(
        std::tuple{ test_command{}, end_game{ .result = game_result::both_loss } }, std::tuple{}
    );
    card_table table;
    table.load_deck(player_id{ 0 }, { .characters = {
        ids.get_id<character_view>(first.name()), ids.get_id<character_view>(second.name())
    } });
    executor target;
    target.enter_entry(library);
    zero_random random;
    REQUIRE((observed ? target.step(library, table, random) : target.run(library, table, random))
        == execution_state::finished);
    if(terminal)
    {
        CHECK(handlers == std::vector{ character_id{ player_id{ 0 }, 0 } });
        CHECK(target.view_in<execution_state::finished>().result() == game_result::player_1_win);
    }
    else
    {
        CHECK(handlers == std::vector{ character_id{ player_id{ 0 }, 0 }, character_id{ player_id{ 0 }, 1 } });
        CHECK(target.view_in<execution_state::finished>().result() == game_result::both_loss);
    }
}
