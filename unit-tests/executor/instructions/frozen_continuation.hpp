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

namespace givm_test::executor_instructions::frozen_continuation
{
namespace
{
    struct pausing_frozen
    {
        using definition_category = givm::attachment_view;
        struct definition_type { std::size_t* applications; givm::normal_effect pause; };
        std::size_t* applications;
        std::string_view name() const { return "PausingFrozen"; }
        auto tags() const { return std::array{ std::string_view{ "control" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { applications, context.add_normal_effect(std::tuple{ givm::replace_cards{ givm::player_id{ 0 } } }) };
        }
        static givm::attachment_state query(const definition_type&, const givm::attachment_state_limit&)
        {
            return { .count = 1 };
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::this_attachment_reapply&, givm::handle_context<givm::attachment_view>& context, std::uint32_t = 0)
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
    const auto reactions = givm_test::default_reactions((givm::genshin_impact::dendro_core_3_3_0).name(), (givm::genshin_impact::catalyzing_field_3_4_0).name(), (givm::genshin_impact::burning_flame_3_3_0).name(), (frozen).name(), (givm_test::shield).name());
    const auto basics = givm_test::basic_sources;
    givm::definition_source_library sources;
    REQUIRE(sources.add(givm::genshin_impact::dendro_core_3_3_0, givm::genshin_impact::catalyzing_field_3_4_0, givm::genshin_impact::burning_flame_3_3_0, frozen, givm_test::shield));
    std::apply([&](const auto&... reaction) { REQUIRE(sources.add(reaction...)); }, reactions);
    REQUIRE(sources.add(character));
    const std::array damages{
        givm::deal_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::cryo },
        givm::deal_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::physical }
    };
    const std::array attachment_names{ frozen.name() };
    const givm::test::initialization_skill_source initialization{
        [&](givm::definition_compile_context& context)
        {
            return std::tuple{
                givm::attach{ .player = givm::relative_player::opponent, .definition = context.resolve_id<givm::attachment_view>(frozen.name()) },
                givm::apply_element{ .source = { givm::relative_player::self },
                    .target = { givm::relative_player::opponent }, .element = givm::element::hydro },
                damages[0], damages[1] };
        }, {}, {}, attachment_names };
    const givm::test::initialization_character_source driver{ "FrozenDriver", { .max_health = 20, .health = 20 } };
    REQUIRE(sources.add(initialization, driver));
    const auto [library, ids] = givm_test::require_success(compile(sources, basics, std::tuple{
        givm::set_active_character{ givm::relative_character_target{ givm::relative_player::self } },
        givm::set_active_character{ givm::relative_character_target{ givm::relative_player::opponent } },
        givm::start_battle{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss }
    }, std::tuple{}, givm::compile_mode::observed));
    givm::table table{ { .round_number = 1, .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    const auto character_id = ids.get_id<givm::character_view>(character.name());
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(driver.name()) } },
        { .characters = { character_id } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::health_reduced);
    CHECK(executor.view_in<givm::execution_state::health_reduced>().value() == 2);
    CHECK(applications == 0);
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::card_selection);
    CHECK(applications == 1);
    CHECK(table[target].state().health == 18);
    CHECK(table.state().round_number == 1);
    CHECK(library.is_controlled(table[target]));
    auto copied_executor = executor;
    auto copied_table = table;
    const auto finish = [&](givm_test::executor_driver& running, givm::table& current)
    {
        running.submitted(running.view_in<givm::execution_state::card_selection>().select(library, current, random, {}));
        REQUIRE(running.advance(library, current, random) == givm::execution_state::health_reduced);
        const auto next = running.view_in<givm::execution_state::health_reduced>();
        CHECK(next.target() == target);
        CHECK(next.value() == 1);
        CHECK(next.reaction().slot == givm::elemental_reaction::none);
        REQUIRE(running.advance(library, current, random) == givm::execution_state::finished);
        CHECK(current[target].state().health == 17);
        CHECK(current.state().round_number == 1);
        CHECK(library.is_controlled(current[target]));
        REQUIRE(std::ranges::distance(current[target].attachments()) == 1);
        CHECK((*current[target].attachments().begin()).definition_id() == ids.get_id<givm::attachment_view>(frozen.name()));
        CHECK(applications == 1);
    };
    finish(executor, table);
    finish(copied_executor, copied_table);
}
}
