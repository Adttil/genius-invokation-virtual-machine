#include <array>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/basic_definitions.hpp>
#include <givm/executor.hpp>

#include "../test_character_source.hpp"

namespace givm_test::executor_instructions::reaction_entities
{
namespace
{
    struct reaction_log
    {
        givm::player_id source_player{ 0 };
        bool take_over_effects = false;
        bool nested_after_first = false;
        bool nested_invoked = false;
        std::vector<std::size_t> statuses_at_calculation;
        std::vector<std::size_t> statuses_at_completion;
        std::vector<std::uint32_t> values;
        std::vector<std::uint32_t> summon_usages_at_completion;
        std::vector<givm::summon_id> removed_summons;
        std::uint32_t health_at_removal = 0;
        std::uint32_t repeated = 0;
    };

    struct reaction_observer
    {
        using definition_category = givm::skill_view;
        struct definition_type { reaction_log* log; givm::program_entry nested; givm::tag_id replacement; };
        reaction_log* log;
        std::string_view name() const { return "ReactionObserver"; }
        auto tags() const { return std::array{ std::string_view{ "EntityReactionReplacement" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_program(std::tuple{ givm::deal_damage{} }), *context.find_tag("EntityReactionReplacement") };
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::damage_calculation& event, givm::handle_context& context)
        {
            data.log->statuses_at_calculation.push_back(std::ranges::distance(
                context.table()[data.log->source_player].combat_statuses()));
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::elemental_reaction_will_occur& event, givm::handle_context&)
        {
            if(data.log->take_over_effects) event.replacement_reaction = data.replacement;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::after_damage& event, givm::handle_context& context)
        {
            data.log->statuses_at_completion.push_back(std::ranges::distance(
                context.table()[data.log->source_player].combat_statuses()));
            data.log->values.push_back(event.value);
            if(const auto* summon = std::get_if<givm::summon_id>(&event.source))
            {
                CHECK(context.table()[*summon].is_valid());
                data.log->summon_usages_at_completion.push_back(context.table()[*summon].state().usages);
            }
            if(data.log->nested_after_first && not data.log->nested_invoked)
            {
                data.log->nested_invoked = true;
                return context.invoke(data.nested, givm::deal_damage_input{ std::array{ givm::damage{
                    .source = givm::character_id{ data.log->source_player, 0 },
                    .target = givm::character_id{ other_player(data.log->source_player), 0 },
                    .value = 1, .type = givm::damage_type::dendro
                } } });
            }
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::summon_removed& event, givm::handle_context& context)
        {
            data.log->removed_summons.push_back(event.summon);
            CHECK_FALSE(context.table()[event.summon].is_valid());
            CHECK(context.table()[event.summon].state().usages == 0);
            data.log->health_at_removal = context.table()[givm::character_id{
                other_player(data.log->source_player), 0 }].state().health;
            return {};
        }
    };

    struct source_character
    {
        using definition_category = givm::character_view;
        struct definition_type { givm::definition_id<givm::skill_view> observer; };
        std::string_view name() const { return "ReactionSource"; }
        auto skill_dependencies() const { return std::array{ std::string_view{ "ReactionObserver" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_id<givm::skill_view>("ReactionObserver") };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 30, .health = 30 };
        }
        static givm::definition_id<givm::skill_view> query(const definition_type& data, const givm::character_initial_skill& query)
        {
            return query.skill_index == 0 ? data.observer : givm::definition_id<givm::skill_view>{};
        }
    };

    struct pausing_field
    {
        using definition_category = givm::combat_status_view;
        struct definition_type { reaction_log* log; givm::program_entry repeat; };
        reaction_log* log;
        std::string_view name() const { return "PausingField"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_program(std::tuple{ givm::set_combat_status_state{},
                givm::replace_cards{ givm::player_id{ 0 } } }) };
        }
        static givm::combat_status_state query(const definition_type&, const givm::combat_status_state_limit&)
        {
            return { .count = 2 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::combat_status_view& self,
            givm::combat_status_regeneration& event, givm::handle_context& context)
        {
            ++data.log->repeated;
            return context.invoke(data.repeat, givm::set_combat_status_state_input{ .status = self.id(), .state = event.state });
        }
    };

    struct zero_random { std::uint32_t operator()() const { return 0; } };

    givm::element_aura reaction_aura(givm::elemental_reaction reaction)
    {
        switch(reaction)
        {
        case givm::elemental_reaction::quicken: return givm::element_aura::electro;
        case givm::elemental_reaction::bloom: return givm::element_aura::hydro;
        default: return givm::element_aura::pyro;
        }
    }

    std::tuple<givm::set_active_character, givm::set_active_character> select_fronts()
    {
        return { givm::set_active_character{ givm::relative_character_target{ givm::relative_player::self, 0 } },
            givm::set_active_character{ givm::relative_character_target{ givm::relative_player::opponent, 0 } } };
    }

    givm::execution_state advance(givm_test::executor_driver& executor, const givm::definition_library& library,
        givm::table& table, zero_random& random)
    {
        auto state = executor.advance(library, table, random);
        while(state == givm::execution_state::active_character_changed)
            state = executor.advance(library, table, random);
        return state;
    }
}

TEST_CASE("quicken creates and refreshes its field between hits and empowers later damage", "[reaction-entities][quicken]")
{
    const bool observed = GENERATE(false, true);
    const bool preexisting = GENERATE(false, true);
    const auto player = GENERATE(givm::player_id{ 0 }, givm::player_id{ 1 });
    reaction_log log{ .source_player = player };
    const source_character source_definition;
    const reaction_observer observer{ &log };
    const givm::test::initialized_character_source victim{ "ReactionTarget",
        { .max_health = 30, .health = 30, .aura = givm::element_aura::electro } };
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0, givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0, givm::genshin_impact::frozen_3_3_0, givm::genshin_impact::shield_3_3_0 };
    givm::definition_source_library sources;
    REQUIRE(sources.add(source_definition, observer, victim));
    const givm::character_id source{ player, 0 };
    const givm::character_id first{ other_player(player), 0 };
    const givm::character_id second{ other_player(player), 1 };
    const std::array group{
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::dendro },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 1 }, .value = 1, .type = givm::damage_type::dendro },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::dendro }
    };
    const std::array next{ givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::dendro } };
    std::vector<givm::any_command> commands{
        givm::set_active_character{ givm::relative_character_target{ givm::relative_player::self, 0 } },
        givm::set_active_character{ givm::relative_character_target{ givm::relative_player::opponent, 0 } }
    };
    if(preexisting)
    {
        commands.emplace_back(givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::dendro });
        commands.emplace_back(givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::electro });
    }
    commands.emplace_back(givm::deal_damage{ .damages = group });
    commands.emplace_back(givm::deal_damage{ .damages = next });
    commands.emplace_back(givm::end_game{ givm::game_result::both_loss });
    const auto [library, ids] = givm_test::require_success(compile(sources, basics, commands, std::tuple{},
        observed ? givm::compile_mode::observed : givm::compile_mode::normal));
    const givm::linked_deck source_deck{ .characters = { ids.get_id<givm::character_view>(source_definition.name()) } };
    const auto target_id = ids.get_id<givm::character_view>(victim.name());
    const givm::linked_deck target_deck{ .characters = { target_id, target_id } };
    givm::table table{ { .self_player = player },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, player == givm::player_id{ 0 } ? source_deck : target_deck,
        player == givm::player_id{ 1 } ? source_deck : target_deck);
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    std::vector<std::uint32_t> values;
    std::vector<std::uint32_t> counts_at_health_observation;
    for(;;)
    {
        const auto state = advance(executor, library, table, random);
        if(state == givm::execution_state::finished) break;
        REQUIRE(observed);
        REQUIRE(state == givm::execution_state::health_reduced);
        values.push_back(executor.view_in<givm::execution_state::health_reduced>().value());
        auto statuses = table[player].combat_statuses();
        if(std::ranges::empty(statuses)) counts_at_health_observation.push_back(0);
        else
        {
            REQUIRE(std::ranges::distance(statuses) == 1);
            const auto field = *statuses.begin();
            CHECK(field.definition_id() == library.catalyzing_field_id());
            counts_at_health_observation.push_back(field.state().count);
        }
    }
    CHECK(log.statuses_at_calculation == std::vector<std::size_t>{ preexisting ? 1uz : 0uz, 1, 1, 1 });
    CHECK(log.statuses_at_completion == std::vector<std::size_t>{ 1, 1, 1, 0 });
    CHECK(log.values == std::vector<std::uint32_t>{ preexisting ? 3u : 2u, 2, 2, 2 });
    CHECK(values == (observed ? log.values : std::vector<std::uint32_t>{}));
    CHECK(counts_at_health_observation == (observed
        ? std::vector<std::uint32_t>{ preexisting ? 1u : 0u, 2, 1, 0 } : std::vector<std::uint32_t>{}));
    CHECK(table[first].state().health == (preexisting ? 23 : 24));
    CHECK(table[second].state().health == 28);
    CHECK(std::ranges::empty(table[player].combat_statuses()));
    CHECK(std::ranges::empty(table[other_player(player)].combat_statuses()));
    CHECK(table.state().active_player == givm::player_id{ 0 });
}

TEST_CASE("bloom and burning repeat their official entities within their limits", "[reaction-entities][repeat]")
{
    const auto reaction = GENERATE(givm::elemental_reaction::bloom, givm::elemental_reaction::burning);
    const bool application_only = GENERATE(false, true);
    const source_character source_definition;
    reaction_log log;
    const reaction_observer observer{ &log };
    const givm::test::initialized_character_source victim{ "ReactionTarget", { .max_health = 30, .health = 30 } };
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0, givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0, givm::genshin_impact::frozen_3_3_0, givm::genshin_impact::shield_3_3_0 };
    givm::definition_source_library sources;
    REQUIRE(sources.add(source_definition, observer, victim));
    constexpr givm::character_id source{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id target{ givm::player_id{ 1 }, 0 };
    const std::array damages{ givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::dendro } };
    std::vector<givm::any_command> commands;
    for(int index = 0; index != 3; ++index)
    {
        commands.emplace_back(givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = reaction == givm::elemental_reaction::bloom ? givm::element::hydro : givm::element::pyro });
        if(application_only)
            commands.emplace_back(givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::dendro });
        else commands.emplace_back(givm::deal_damage{ .damages = damages });
    }
    commands.emplace_back(givm::end_game{ givm::game_result::both_loss });
    const auto [library, ids] = givm_test::require_success(compile(sources, basics, commands, std::tuple{}, givm::compile_mode::normal));
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(source_definition.name()) } },
        { .characters = { ids.get_id<givm::character_view>(victim.name()) } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::finished);
    CHECK(table[target].state().health == (application_only ? 30 : 24));
    CHECK(table[target].state().aura == givm::element_aura::none);
    if(reaction == givm::elemental_reaction::bloom)
    {
        REQUIRE(std::ranges::distance(table[givm::player_id{ 0 }].combat_statuses()) == 1);
        const auto core = *table[givm::player_id{ 0 }].combat_statuses().begin();
        CHECK(core.definition_id() == library.dendro_core_id());
        CHECK(core.state().count == 1);
    }
    else
    {
        REQUIRE(std::ranges::distance(table[givm::player_id{ 0 }].summons()) == 1);
        const auto flame = *table[givm::player_id{ 0 }].summons().begin();
        CHECK(flame.definition_id() == library.burning_flame_id());
        CHECK(flame.state().value == 1);
        CHECK(flame.state().usages == 2);
    }
    CHECK(std::ranges::empty(table[givm::player_id{ 1 }].combat_statuses()));
    CHECK(std::ranges::empty(table[givm::player_id{ 1 }].summons()));
    CHECK(log.values.size() == (application_only ? 0 : 3));
}

TEST_CASE("reaction replacement suppresses default numbers and entities while consuming aura", "[reaction-entities][takeover]")
{
    const auto reaction = GENERATE(givm::elemental_reaction::quicken,
        givm::elemental_reaction::bloom, givm::elemental_reaction::burning, givm::elemental_reaction::crystallize);
    const bool take_over_effects = GENERATE(false, true);
    reaction_log log{ .take_over_effects = take_over_effects };
    const source_character source_definition;
    const reaction_observer observer{ &log };
    const givm::test::initialized_character_source victim{ "ReactionTarget",
        { .max_health = 30, .health = 30, .aura = reaction_aura(reaction) } };
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0, givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0, givm::genshin_impact::frozen_3_3_0, givm::genshin_impact::shield_3_3_0 };
    givm::definition_source_library sources;
    REQUIRE(sources.add(source_definition, observer, victim));
    constexpr givm::character_id source{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id target{ givm::player_id{ 1 }, 0 };
    const std::array damages{ givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = reaction == givm::elemental_reaction::crystallize ? givm::damage_type::geo : givm::damage_type::dendro } };
    const auto [library, ids] = givm_test::require_success(compile(sources, basics, std::tuple{ givm::deal_damage{ .damages = damages },
        givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, givm::compile_mode::normal));
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(source_definition.name()) } },
        { .characters = { ids.get_id<givm::character_view>(victim.name()) } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::finished);
    CHECK(table[target].state().health == (take_over_effects ? 29 : 28));
    CHECK(table[target].state().aura == givm::element_aura::none);
    const auto entity_count = std::ranges::distance(table[givm::player_id{ 0 }].combat_statuses())
        + std::ranges::distance(table[givm::player_id{ 0 }].summons());
    CHECK(entity_count == (take_over_effects ? 0 : 1));
}

TEST_CASE("a self-applied reaction creates its entity for the affected player's opponent", "[reaction-entities][apply_element]")
{
    const auto reaction = GENERATE(givm::elemental_reaction::bloom, givm::elemental_reaction::crystallize);
    const givm::test::initialized_character_source target_definition{ "SelfApplicationTarget",
        { .max_health = 30, .health = 30, .aura = givm::element_aura::hydro } };
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0, givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0, givm::genshin_impact::frozen_3_3_0, givm::genshin_impact::shield_3_3_0 };
    givm::definition_source_library sources;
    REQUIRE(sources.add(target_definition));
    constexpr givm::character_id self{ givm::player_id{ 0 }, 0 };
    const auto [library, ids] = givm_test::require_success(compile(sources, basics, std::tuple{
        givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::self, 0 }, .element = reaction == givm::elemental_reaction::crystallize ? givm::element::geo : givm::element::dendro },
        givm::end_game{ givm::game_result::both_loss }
    }, std::tuple{}, givm::compile_mode::normal));
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    const auto target_id = ids.get_id<givm::character_view>(target_definition.name());
    load_deck(table, library, { .characters = { target_id } }, { .characters = { target_id } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::finished);
    CHECK(table[self].state().health == 30);
    CHECK(table[self].state().aura == givm::element_aura::none);
    CHECK(std::ranges::empty(table[givm::player_id{ 0 }].combat_statuses()));
    REQUIRE(std::ranges::distance(table[givm::player_id{ 1 }].combat_statuses()) == 1);
    CHECK((*table[givm::player_id{ 1 }].combat_statuses().begin()).definition_id() == (reaction == givm::elemental_reaction::crystallize ? library.shield_id() : library.dendro_core_id()));
}

TEST_CASE("the first damage completion can use a field produced by a later hit", "[reaction-entities][nested]")
{
    reaction_log log{ .nested_after_first = true };
    const source_character source_definition;
    const reaction_observer observer{ &log };
    const givm::test::initialized_character_source victim{ "ReactionTarget",
        { .max_health = 30, .health = 30, .aura = givm::element_aura::electro } };
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0, givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0, givm::genshin_impact::frozen_3_3_0, givm::genshin_impact::shield_3_3_0 };
    givm::definition_source_library sources;
    REQUIRE(sources.add(source_definition, observer, victim));
    constexpr givm::character_id source{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id target{ givm::player_id{ 1 }, 0 };
    const std::array damages{
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::physical },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::dendro }
    };
    const auto [library, ids] = givm_test::require_success(compile(sources, basics, std::tuple_cat(select_fronts(), std::tuple{
        givm::deal_damage{ .damages = damages }, givm::end_game{ givm::game_result::both_loss }
    }), std::tuple{}, givm::compile_mode::normal));
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(source_definition.name()) } },
        { .characters = { ids.get_id<givm::character_view>(victim.name()) } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::finished);
    CHECK(log.nested_invoked);
    CHECK(log.statuses_at_calculation == std::vector<std::size_t>{ 0, 0, 1 });
    CHECK(log.statuses_at_completion == std::vector<std::size_t>{ 1, 1, 1 });
    CHECK(log.values == std::vector<std::uint32_t>{ 1, 2, 2 });
    CHECK(table[target].state().health == 25);
    REQUIRE(std::ranges::distance(table[givm::player_id{ 0 }].combat_statuses()) == 1);
    CHECK((*table[givm::player_id{ 0 }].combat_statuses().begin()).state().count == 1);
}

TEST_CASE("burning flame finishes its damage before exhausting and broadcasting removal", "[reaction-entities][burning]")
{
    const bool observed = GENERATE(false, true);
    reaction_log log;
    const source_character source_definition;
    const reaction_observer observer{ &log };
    const givm::test::initialized_character_source victim{ "ReactionTarget",
        { .max_health = 30, .health = 30, .aura = givm::element_aura::pyro } };
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0, givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0, givm::genshin_impact::frozen_3_3_0, givm::genshin_impact::shield_3_3_0 };
    givm::definition_source_library sources;
    REQUIRE(sources.add(source_definition, observer, victim));
    constexpr givm::character_id source{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id target{ givm::player_id{ 1 }, 0 };
    const auto [library, ids] = givm_test::require_success(compile(sources, basics, std::tuple_cat(select_fronts(), std::tuple{
        givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::dendro },
        givm::end_round{}, givm::end_game{ givm::game_result::both_loss }
    }), std::tuple{}, observed ? givm::compile_mode::observed : givm::compile_mode::normal));
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(source_definition.name()) } },
        { .characters = { ids.get_id<givm::character_view>(victim.name()) } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    std::size_t damage_observations = 0;
    std::size_t round_observations = 0;
    for(;;)
    {
        const auto state = advance(executor, library, table, random);
        if(state == givm::execution_state::finished) break;
        REQUIRE(observed);
        if(state == givm::execution_state::round_ending)
        {
            ++round_observations;
            CHECK(table[target].state().health == 30);
        }
        else
        {
            REQUIRE(state == givm::execution_state::health_reduced);
            ++damage_observations;
            CHECK(executor.view_in<givm::execution_state::health_reduced>().value() == 1);
            CHECK(log.removed_summons.empty());
        }
        REQUIRE(std::ranges::distance(table[givm::player_id{ 0 }].summons()) == 1);
        CHECK((*table[givm::player_id{ 0 }].summons().begin()).state().usages == 1);
    }
    CHECK(damage_observations == (observed ? 1 : 0));
    CHECK(round_observations == (observed ? 1 : 0));
    CHECK(log.values == std::vector<std::uint32_t>{ 1 });
    CHECK(log.summon_usages_at_completion == std::vector<std::uint32_t>{ 1 });
    REQUIRE(log.removed_summons.size() == 1);
    CHECK(log.health_at_removal == 29);
    CHECK(table[target].state().health == 29);
    CHECK(std::ranges::empty(table[givm::player_id{ 0 }].summons()));
    CHECK(table[log.removed_summons.front()].definition_id() == library.burning_flame_id());
}

TEST_CASE("reaction regeneration resumes once before the next hit and copied groups finish independently", "[reaction-entities][observation]")
{
    reaction_log log;
    const source_character source_definition;
    const reaction_observer observer{ &log };
    const pausing_field field{ &log };
    const givm::test::initialized_character_source victim{ "ReactionTarget",
        { .max_health = 30, .health = 30, .aura = givm::element_aura::electro } };
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0, field,
        givm::genshin_impact::burning_flame_3_3_0, givm::genshin_impact::frozen_3_3_0, givm::genshin_impact::shield_3_3_0 };
    givm::definition_source_library sources;
    REQUIRE(sources.add(source_definition, observer, victim));
    constexpr givm::character_id source{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id first{ givm::player_id{ 1 }, 0 };
    constexpr givm::character_id second{ givm::player_id{ 1 }, 1 };
    const std::array damages{
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::dendro },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 1 }, .value = 1, .type = givm::damage_type::dendro },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::physical }
    };
    const auto [library, ids] = givm_test::require_success(compile(sources, basics, std::tuple{ givm::deal_damage{ .damages = damages },
        givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, givm::compile_mode::observed));
    const auto target_id = ids.get_id<givm::character_view>(victim.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(source_definition.name()) } },
        { .characters = { target_id, target_id } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    for(const auto target : { first, second })
    {
        REQUIRE(executor.advance(library, table, random) == givm::execution_state::health_reduced);
        CHECK(executor.view_in<givm::execution_state::health_reduced>().target() == target);
        CHECK(executor.view_in<givm::execution_state::health_reduced>().value() == 2);
        CHECK(std::ranges::distance(table[givm::player_id{ 0 }].combat_statuses()) == (target == first ? 0 : 1));
        CHECK(log.values.empty());
    }
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::card_selection);
    CHECK(log.repeated == 1);
    CHECK(log.values.empty());
    REQUIRE(std::ranges::distance(table[givm::player_id{ 0 }].combat_statuses()) == 1);
    CHECK(table[first].state().health == 28);
    CHECK(table[second].state().health == 28);
    auto copied_executor = executor;
    auto copied_table = table;
    executor.submitted(executor.view_in<givm::execution_state::card_selection>().select(library, table, random, {}));
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::health_reduced);
    CHECK(executor.view_in<givm::execution_state::health_reduced>().target() == first);
    CHECK(executor.view_in<givm::execution_state::health_reduced>().value() == 1);
    CHECK(table[first].state().health == 27);
    CHECK(log.values.empty());
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::finished);
    CHECK(log.values == std::vector<std::uint32_t>{ 2, 2, 1 });
    CHECK(log.statuses_at_completion == std::vector<std::size_t>{ 1, 1, 1 });
    CHECK(log.repeated == 1);
    log.values.clear();
    log.statuses_at_completion.clear();
    copied_executor.submitted(copied_executor.view_in<givm::execution_state::card_selection>().select(library, copied_table, random, {}));
    REQUIRE(copied_executor.advance(library, copied_table, random) == givm::execution_state::health_reduced);
    CHECK(copied_executor.view_in<givm::execution_state::health_reduced>().target() == first);
    CHECK(copied_executor.view_in<givm::execution_state::health_reduced>().value() == 1);
    CHECK(copied_table[first].state().health == 27);
    CHECK(log.values.empty());
    REQUIRE(copied_executor.advance(library, copied_table, random) == givm::execution_state::finished);
    CHECK(log.values == std::vector<std::uint32_t>{ 2, 2, 1 });
    CHECK(log.statuses_at_completion == std::vector<std::size_t>{ 1, 1, 1 });
    CHECK(log.repeated == 1);
    CHECK(table.state().round_number == 0);
    CHECK(copied_table.state().round_number == 0);
    CHECK((*copied_table[givm::player_id{ 0 }].combat_statuses().begin()).state().count == 2);
}

TEST_CASE("crystallize adds damage and stacks a shield that protects only the active character", "[reaction-entities][crystallize]")
{
    const bool observed = GENERATE(false, true);
    const bool application_only = GENERATE(false, true);
    reaction_log log;
    const reaction_observer observer{ &log };
    const source_character source_definition;
    const givm::test::initialized_character_source other{ "ShieldTarget", { .max_health = 30, .health = 30 } };
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0, givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0, givm::genshin_impact::frozen_3_3_0, givm::genshin_impact::shield_3_3_0 };
    givm::definition_source_library sources;
    REQUIRE(sources.add(source_definition, observer, other));
    const givm::relative_character_target self{ givm::relative_player::self, 0 };
    const givm::relative_character_target opponent{ givm::relative_player::opponent, 0 };
    const std::array crystallize{ givm::fixed_damage{ .source = self, .target = opponent,
        .value = 1, .type = givm::damage_type::geo } };
    const std::array incoming{
        givm::fixed_damage{ .source = opponent, .target = self, .value = 2, .type = givm::damage_type::piercing },
        givm::fixed_damage{ .source = opponent, .target = { givm::relative_player::self, 1 }, .value = 2, .type = givm::damage_type::physical },
        givm::fixed_damage{ .source = opponent, .target = self, .value = 0, .type = givm::damage_type::physical },
        givm::fixed_damage{ .source = opponent, .target = self, .value = 1, .type = givm::damage_type::physical },
        givm::fixed_damage{ .source = opponent, .target = self, .value = 2, .type = givm::damage_type::physical }
    };
    std::vector<givm::any_command> commands;
    for(int index = 0; index != 3; ++index)
    {
        commands.emplace_back(givm::apply_element{ .source = self, .target = opponent, .element = givm::element::hydro });
        if(application_only)
            commands.emplace_back(givm::apply_element{ .source = self, .target = opponent, .element = givm::element::geo });
        else commands.emplace_back(givm::deal_damage{ .damages = crystallize });
    }
    commands.emplace_back(givm::deal_damage{ .damages = incoming });
    commands.emplace_back(givm::end_game{ givm::game_result::both_loss });
    const auto [library, ids] = givm_test::require_success(compile(sources, basics, commands, std::tuple{},
        observed ? givm::compile_mode::observed : givm::compile_mode::normal));
    const givm::player_id player{ 0 };
    const givm::character_id front{ player, 0 };
    const givm::character_id back{ player, 1 };
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    givm::table table{ { .self_player = player }, { .active_character = front }, { .active_character = target } };
    const auto other_id = ids.get_id<givm::character_view>(other.name());
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(source_definition.name()), other_id } },
        { .characters = { other_id } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    std::vector<std::uint32_t> observations;
    for(;;)
    {
        const auto state = executor.advance(library, table, random);
        if(state == givm::execution_state::finished) break;
        REQUIRE(observed);
        REQUIRE(state == givm::execution_state::health_reduced);
        const auto view = executor.view_in<givm::execution_state::health_reduced>();
        observations.push_back(view.value());
        if(observations.size() == (application_only ? 1 : 4))
        {
            REQUIRE(std::ranges::distance(table[player].combat_statuses()) == 1);
            const auto shield = *table[player].combat_statuses().begin();
            CHECK(shield.definition_id() == library.shield_id());
            CHECK(shield.state().count == 2);
            CHECK(shield.state().round_usages == 0);
        }
    }
    std::vector<std::uint32_t> expected;
    if(not application_only) expected = { 2, 2, 2 };
    expected.insert(expected.end(), { 2, 2, 0, 0, 1 });
    CHECK(log.values == expected);
    auto expected_observations = observed ? expected : std::vector<std::uint32_t>{};
    std::erase(expected_observations, 0);
    CHECK(observations == expected_observations);
    CHECK(table[front].state().health == 27);
    CHECK(table[back].state().health == 28);
    CHECK(table[target].state().health == (application_only ? 30 : 24));
    CHECK(table[target].state().aura == givm::element_aura::none);
    CHECK(std::ranges::empty(table[player].combat_statuses()));
    CHECK(std::ranges::empty(table[givm::player_id{ 1 }].combat_statuses()));
}

TEST_CASE("crystallize preserves shield points above its accumulation limit", "[reaction-entities][crystallize][state-limit]")
{
    const givm::test::initialization_skill_source initialize{ [](givm::definition_compile_context& context)
    {
        return std::tuple{
            givm::generate_combat_status{ .definition = context.shield_id(), .state = { 2, 0 } },
            givm::modify_combat_status_state{ .definition = context.shield_id(), .count = 1, .ignore_limit = true },
            givm::apply_element{ .source = { givm::relative_player::self, 0 },
                .target = { givm::relative_player::opponent, 0 }, .element = givm::element::geo }
        };
    } };
    const givm::test::initialization_character_source source;
    const givm::test::initialized_character_source target{ "OverLimitShieldTarget",
        { .max_health = 10, .health = 10, .aura = givm::element_aura::hydro } };
    givm::definition_source_library sources;
    REQUIRE(sources.add(source, initialize, target));
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0, givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0, givm::genshin_impact::frozen_3_3_0, givm::genshin_impact::shield_3_3_0 };
    const auto [library, ids] = givm_test::require_success(compile(sources, basics,
        std::tuple{ givm::start_battle{}, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, givm::compile_mode::normal));
    const givm::player_id player{ 0 };
    givm::table table{ { .round_number = 1 },
        { .active_character = givm::character_id{ player, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(source.name()) } },
        { .characters = { ids.get_id<givm::character_view>(target.name()) } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::finished);
    REQUIRE(std::ranges::distance(table[player].combat_statuses()) == 1);
    const auto shield = *table[player].combat_statuses().begin();
    CHECK(shield.definition_id() == library.shield_id());
    CHECK(shield.state().count == 3);
    CHECK(shield.state().round_usages == 0);
}
}
