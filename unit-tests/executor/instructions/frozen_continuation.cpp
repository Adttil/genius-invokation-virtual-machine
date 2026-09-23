#include <array>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <givm/basic_definitions.hpp>
#include <givm/executor.hpp>

#include "../test_character_source.hpp"

namespace
{
    struct pausing_frozen
    {
        using definition_category = givm::attachment_view;
        struct definition_type { std::size_t* applications; givm::program_entry pause; };
        std::size_t* applications;
        std::string_view name() const { return "PausingFrozen"; }
        auto tags() const { return std::array{ std::string_view{ "control" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { applications, context.add_program(std::tuple{ givm::replace_cards{ givm::player_id{ 0 } } }) };
        }
        static givm::attachment_state query(const definition_type&, const givm::attachment_state_limit&)
        {
            return { .count = 1 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::attachment_view&,
            givm::attachment_reapplication&, givm::handle_context& context)
        {
            ++*data.applications;
            return context.invoke(data.pause);
        }
    };

    struct zero_random { std::uint32_t operator()() const { return 0; } };
}

TEST_CASE("a copied damage group resumes after the selected frozen definition's reapplication program", "[control][frozen][resume]")
{
    constexpr givm::character_id source{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id target{ givm::player_id{ 1 }, 0 };
    std::size_t applications = 0;
    const pausing_frozen frozen{ &applications };
    const givm::test::initialized_character_source character{ "Character", { .max_health = 20, .health = 20 } };
    const givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0, givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0, frozen, character };
    const auto prepared_ids = sources.make_issued_id_map();
    const std::array damages{
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_damage_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::cryo },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_damage_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::physical }
    };
    const auto [library, ids] = compile(sources, std::tuple{
        givm::set_active_character{ givm::relative_character_target{ givm::relative_player::self, 0 } }, givm::set_active_character{ givm::relative_character_target{ givm::relative_player::opponent, 0 } },
        givm::attach{ .player = givm::relative_player::opponent,
            .definition = prepared_ids.get_id<givm::attachment_view>(frozen.name()) },
        givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::hydro },
        givm::deal_damage{ .damages = damages }, givm::end_game{ givm::game_result::both_loss }
    }, std::tuple{}, givm::compile_mode::observed);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    const auto character_id = ids.get_id<givm::character_view>(character.name());
    load_deck(table, library, { .characters = { character_id } }, { .characters = { character_id } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    REQUIRE(executor.step(library, table, random) == givm::execution_state::health_reduced);
    CHECK(executor.view_in<givm::execution_state::health_reduced>().value() == 2);
    CHECK(applications == 0);
    REQUIRE(executor.step(library, table, random) == givm::execution_state::card_selection);
    CHECK(applications == 1);
    CHECK(table[target].state().health == 18);
    CHECK(table.state().round_number == 0);
    CHECK(library.is_controlled(table[target]));
    auto copied_executor = executor;
    auto copied_table = table;
    const auto finish = [&](givm::executor& running, givm::table& current)
    {
        running.view_in<givm::execution_state::card_selection>().select({});
        REQUIRE(running.step(library, current, random) == givm::execution_state::health_reduced);
        const auto next = running.view_in<givm::execution_state::health_reduced>();
        CHECK(next.target() == target);
        CHECK(next.value() == 1);
        CHECK(next.reaction() == givm::elemental_reaction::none);
        REQUIRE(running.step(library, current, random) == givm::execution_state::finished);
        CHECK(current[target].state().health == 17);
        CHECK(current.state().round_number == 0);
        CHECK(library.is_controlled(current[target]));
        REQUIRE(std::ranges::distance(current[target].attachments()) == 1);
        CHECK((*current[target].attachments().begin()).definition_id() == library.frozen_id());
        CHECK(applications == 1);
    };
    finish(executor, table);
    finish(copied_executor, copied_table);
}
