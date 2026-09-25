#include <array>
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
    struct positioned_character_source
    {
        using definition_category = givm::character_view;
        struct definition_type { std::uint32_t health; };
        std::string_view source_name;
        std::uint32_t health = 10;

        std::string_view name() const { return source_name; }
        definition_type compile(givm::definition_compile_context&) const { return { health }; }
        static givm::character_state query(const definition_type& definition, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = definition.health };
        }
    };

    constexpr givm::player_id player{ 0 };
    constexpr givm::relative_character_target relative(std::int32_t offset)
    {
        return { givm::relative_player::self, offset };
    }
}

TEST_CASE("fixed character positions use signed offsets and circular living targets", "[fixed-target][set_active_character]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{
            givm::set_active_character{ relative(1) },
            givm::set_active_character{ { givm::relative_player::self, 1 } },
            givm::set_active_character{ { givm::relative_player::self, -1 } },
            givm::set_active_character{ { givm::relative_player::self, 2 } },
            givm::end_game{ givm::game_result::both_loss }
        }, std::tuple{},
        positioned_character_source{ "Alive", 10 }, positioned_character_source{ "Defeated", 0 });
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ player, 0 } } };
    const auto alive = ids.get_id<givm::character_view>("Alive");
    const auto defeated = ids.get_id<givm::character_view>("Defeated");
    load_deck(table, library, { .characters = { alive, defeated, alive } }, {});
    givm::executor executor;
    executor.enter_entry(library);
    auto random = [] { return std::uint32_t{ 0 }; };
    if(mode == givm::compile_mode::observed)
    {
        for(const auto index : std::array<std::size_t, 3>{ 2, 0, 2 })
        {
            REQUIRE(executor.step(library, table, random) == givm::execution_state::active_character_changed);
            CHECK(executor.view_in<givm::execution_state::active_character_changed>().character()
                == givm::character_id{ player, index });
        }
    }
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(table[player].state().active_character == givm::character_id{ player, 2 });
}

TEST_CASE("fixed healing and maximum health increases can locate a defeated character", "[fixed-target][heal]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{
            givm::heal{ .source = relative(0), .target = relative(1), .value = 2 },
            givm::increase_max_health{ .source = relative(0), .target = relative(1), .value = 3 },
            givm::apply_element{ .source = relative(0), .target = relative(1), .element = givm::element::hydro },
            givm::end_game{ givm::game_result::both_loss }
        }, std::tuple{},
        positioned_character_source{ "Alive", 10 }, positioned_character_source{ "Defeated", 0 });
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ player, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>("Alive"),
        ids.get_id<givm::character_view>("Defeated") } }, {});
    givm::executor executor;
    executor.enter_entry(library);
    auto random = [] { return std::uint32_t{ 0 }; };
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    const auto& state = table[givm::character_id{ player, 1 }].state();
    CHECK(state.health == 5);
    CHECK(state.max_health == 13);
    CHECK(state.aura == givm::element_aura::hydro);
    CHECK(table[givm::character_id{ player, 0 }].state().health == 10);
}

TEST_CASE("fixed character commands skip effects without an active character", "[fixed-target]")
{
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::observed,
        std::tuple{
            givm::set_active_character{ { givm::relative_player::self, 0 } },
            givm::set_active_character{ relative(1) },
            givm::heal{ .source = relative(0), .target = relative(1), .value = 2 },
            givm::increase_max_health{ .source = relative(1), .target = relative(0), .value = 3 },
            givm::apply_element{ .source = relative(0), .target = relative(1), .element = givm::element::hydro },
            givm::end_game{ givm::game_result::both_loss }
        }, std::tuple{}, positioned_character_source{ "Alive", 7 });
    givm::table table{ { .self_player = givm::player_id{ 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>("Alive") } }, {});
    givm::executor executor;
    executor.enter_entry(library);
    auto random = [] { return std::uint32_t{ 0 }; };
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    REQUIRE_FALSE(table[player].state().active_character.has_value());
    const auto& state = table[givm::character_id{ player, 0 }].state();
    CHECK(state.health == 7);
    CHECK(state.max_health == 10);
    CHECK(state.aura == givm::element_aura::none);
}

namespace
{
    struct switch_order_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            std::vector<givm::character_id>* responses;
            givm::program_entry entry;
            bool dynamic;
        };
        std::vector<givm::character_id>* responses;
        bool dynamic;

        std::string_view name() const { return "SwitchOrderSource"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { responses, context.add_program(std::tuple{
                dynamic ? givm::set_active_character{} : givm::set_active_character{ relative(0) },
                dynamic ? givm::set_active_character{} : givm::set_active_character{ relative(1) },
                dynamic ? givm::set_active_character{} : givm::set_active_character{ relative(0) }
            }), dynamic };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            givm::test_event&, givm::handle_context& context)
        {
            if(self.id().index != 0) return {};
            if(data.dynamic)
                return context.invoke(data.entry, givm::set_active_character_input{ { player, 0 } },
                    givm::set_active_character_input{ { player, 1 } }, givm::set_active_character_input{ { player, 1 } });
            return context.invoke(data.entry);
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            givm::active_character_changed& event, givm::handle_context& context)
        {
            CHECK(event.current.index == 1);
            CHECK(context.table()[player].state().active_character == event.current);
            data.responses->push_back(self.id());
            return {};
        }
    };
}

TEST_CASE("switch notifications prioritize the new active character in both compilation modes", "[set_active_character][broadcast-order]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool dynamic = GENERATE(false, true);
    std::vector<givm::character_id> responses;
    const auto source = givm::test::with_passive_skill(switch_order_source{ &responses, dynamic });
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::set_active_character{ relative(0) }, givm::test_command{},
            givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, source);
    const auto character = ids.get_id<givm::character_view>(source.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ player, 0 } } };
    load_deck(table, library, { .characters = { character, character } }, {});
    givm::executor executor;
    executor.enter_entry(library);
    auto random = [] { return std::uint32_t{ 0 }; };
    if(mode == givm::compile_mode::observed)
    {
        REQUIRE(executor.step(library, table, random) == givm::execution_state::active_character_changed);
        CHECK(executor.view_in<givm::execution_state::active_character_changed>().character()
            == givm::character_id{ player, 1 });
        CHECK(table[player].state().active_character == givm::character_id{ player, 0 });
        CHECK_FALSE(table[player].state().can_plunge);
        CHECK(responses.empty());
    }
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(responses == std::vector<givm::character_id>{ { player, 1 }, { player, 0 } });
    CHECK(table[player].state().active_character == givm::character_id{ player, 1 });
    CHECK(table[player].state().can_plunge);
}
