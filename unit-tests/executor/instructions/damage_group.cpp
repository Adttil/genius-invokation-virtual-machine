#include <array>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"
#include "../test_character_source.hpp"

namespace
{
    constexpr givm::player_id attacking_player{ 0 };
    constexpr givm::player_id defending_player{ 1 };
    constexpr givm::character_id attacker{ attacking_player, 0 };
    constexpr givm::character_id victim(std::size_t index) { return { defending_player, index }; }

    enum class phase { calculation, effect, reaction, after_reaction, after_damage };

    struct group_log
    {
        std::vector<std::pair<phase, std::size_t>> order;
        std::vector<std::vector<std::uint32_t>> after_health;
        std::vector<givm::element_aura> after_second_aura;
        std::vector<givm::damage_type> final_types;
        std::vector<givm::elemental_reaction> reactions;
        bool enchant = false;
        bool take_over = false;
        bool change_aura_after_first = false;
        bool nested = false;
        bool nested_invoked = false;
        bool nested_after_first = false;
        bool change_aura_during_effect = false;
        bool invoke_each_phase = false;
    };

    struct group_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            group_log* log;
            givm::program_entry pair;
            givm::program_entry single;
            givm::program_entry change_aura;
            givm::program_entry change_target_aura;
            givm::program_entry count_response;
            givm::tag_id replacement;
        };
        group_log* log;

        std::string_view name() const noexcept { return "GroupObserver"; }
        auto tags() const { return std::array{ std::string_view{ "GroupReactionReplacement" } }; }
        auto tag_dependencies() const { return tags(); }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log,
                context.add_program(std::tuple{ givm::deal_damage{} }),
                context.add_program(std::tuple{ givm::deal_damage{} }),
                context.add_program(std::tuple{
                    givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 1 }, .element = givm::element::none },
                    givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 1 }, .element = givm::element::pyro }
                }),
                context.add_program(std::tuple{
                    givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::none },
                    givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::pyro }
                }),
                context.add_program(std::tuple{ givm::replace_cards{ .player = attacking_player } }),
                context.resolve_tag("GroupReactionReplacement") };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::test_event&, givm::handle_context& context)
        {
            return context.invoke(data.pair,
                givm::deal_damage_input{ std::array{ givm::damage{ .source = attacker, .target = victim(0), .value = 2, .type = givm::damage_type::physical },
                givm::damage{ .source = attacker, .target = victim(1), .value = 3, .type = givm::damage_type::physical } } });
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::damage_preparation& event, givm::handle_context&)
        {
            if(data.log->enchant && event.target == victim(0) && event.type == givm::damage_type::physical)
                event.type = givm::damage_type::electro;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::damage_calculation& event, givm::handle_context& context)
        {
            data.log->order.emplace_back(phase::calculation, event.target.index);
            if(data.log->nested && !data.log->nested_invoked && event.target == victim(0))
            {
                data.log->nested_invoked = true;
                return context.invoke(data.single, givm::deal_damage_input{ std::array{ givm::damage{
                    .source = attacker, .target = victim(2), .value = 4, .type = givm::damage_type::physical } } });
            }
            return data.log->invoke_each_phase ? context.invoke(data.count_response) : givm::program_entry{};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::damage_effect& event, givm::handle_context& context)
        {
            data.log->order.emplace_back(phase::effect, event.target.index);
            data.log->final_types.push_back(event.type);
            if(data.log->change_aura_during_effect && event.target == victim(0))
                return context.invoke(data.change_target_aura);
            return data.log->invoke_each_phase ? context.invoke(data.count_response) : givm::program_entry{};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::elemental_reaction_will_occur& event, givm::handle_context& context)
        {
            data.log->order.emplace_back(phase::reaction, event.target.index);
            data.log->reactions.push_back(event.reaction);
            if(data.log->take_over) event.replacement_reaction = data.replacement;
            return data.log->invoke_each_phase ? context.invoke(data.count_response) : givm::program_entry{};
        }
        static void record_health(group_log& log, const givm::table& table)
        {
            std::vector<std::uint32_t> values;
            for(const auto character : table[defending_player].characters())
                values.push_back(character.state().health);
            log.after_health.push_back(std::move(values));
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::after_elemental_reaction& event, givm::handle_context& context)
        {
            data.log->order.emplace_back(phase::after_reaction, event.target.index);
            record_health(*data.log, context.table());
            return data.log->invoke_each_phase ? context.invoke(data.count_response) : givm::program_entry{};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::after_damage& event, givm::handle_context& context)
        {
            data.log->order.emplace_back(phase::after_damage, event.target.index);
            record_health(*data.log, context.table());
            data.log->after_second_aura.push_back(context.table()[victim(1)].state().aura);
            if(data.log->nested_after_first && event.target == victim(0))
                return context.invoke(data.single, givm::deal_damage_input{ std::array{ givm::damage{
                    .source = attacker, .target = victim(2), .value = 4, .type = givm::damage_type::physical } } });
            if(data.log->change_aura_after_first && event.target == victim(0))
                return context.invoke(data.change_aura);
            return data.log->invoke_each_phase ? context.invoke(data.count_response) : givm::program_entry{};
        }
    };

    struct zero_random { std::uint32_t operator()() const noexcept { return 0; } };

    struct death_log
    {
        std::vector<givm::character_id> completed;
        std::size_t attachment_responses = 0;
    };

    struct death_attachment_source
    {
        using definition_category = givm::attachment_view;
        struct definition_type { death_log* log; };
        death_log* log;
        std::string_view source_name;
        std::vector<std::string_view> source_tags;

        std::string_view name() const { return source_name; }
        const auto& tags() const { return source_tags; }
        definition_type compile(givm::definition_compile_context&) const { return { log }; }
        static givm::program_entry handle(const definition_type& data, const givm::attachment_view&,
            givm::after_damage&, givm::handle_context&)
        {
            ++data.log->attachment_responses;
            return {};
        }
    };

    struct dying_character_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            death_log* log;
            givm::definition_id<givm::skill_view> skill;
            givm::definition_id<givm::attachment_view> ordinary;
            givm::definition_id<givm::attachment_view> artifact;
            givm::program_entry setup;
        };
        death_log* log;

        std::string_view name() const { return "DyingCharacter"; }
        auto skill_dependencies() const { return std::array{ std::string_view{ "RetainedSkill" } }; }
        auto attachment_dependencies() const
        {
            return std::array{ std::string_view{ "RemovedAttachment" }, std::string_view{ "RemovedArtifact" } };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.resolve_id<givm::skill_view>("RetainedSkill"),
                context.resolve_id<givm::attachment_view>("RemovedAttachment"),
                context.resolve_id<givm::attachment_view>("RemovedArtifact"),
                context.add_program(std::tuple{ givm::add_attachment{}, givm::add_attachment{} }) };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .max_energy = 3, .health = 1, .energy = 3 };
        }
        static givm::definition_id<givm::skill_view> query(const definition_type& data, const givm::character_initial_skill& query)
        {
            return query.skill_index == 0 ? data.skill : givm::definition_id<givm::skill_view>{};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            givm::test_event&, givm::handle_context& context)
        {
            return context.invoke(data.setup,
                givm::add_attachment_input{ .target = self.id(), .definition = data.ordinary },
                givm::add_attachment_input{ .target = self.id(), .definition = data.artifact });
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            givm::after_damage& event, givm::handle_context&)
        {
            CHECK(self.is_valid());
            CHECK(self.state().health == 0);
            CHECK(self.state().energy == 0);
            CHECK(std::ranges::empty(self.attachments()));
            CHECK_FALSE(self.has(givm::equipment_type::artifact));
            CHECK(std::ranges::distance(self.skills()) == 2);
            data.log->completed.push_back(event.target);
            return {};
        }
    };

    std::vector<std::size_t> targets_at(const group_log& log, phase wanted)
    {
        std::vector<std::size_t> targets;
        for(const auto [kind, target] : log.order)
            if(kind == wanted) targets.push_back(target);
        return targets;
    }
}

TEST_CASE("damage groups finish all health changes before invoking completion responses", "[deal_damage][group]")
{
    const bool dynamic = GENERATE(false, true);
    group_log log{ .change_aura_after_first = true };
    const auto observer = givm::test::with_passive_skill(group_source{ &log });
    const givm::test::initialized_character_source character{ "Victim" };
    std::array damages{
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 2, .type = givm::damage_type::physical },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 1 }, .value = 3, .type = givm::damage_type::physical }
    };
    std::vector<givm::any_command> commands;
    if(dynamic) commands.emplace_back(givm::test_command{});
    else commands.emplace_back(givm::deal_damage{ .damages = damages });
    commands.emplace_back(givm::end_game{ givm::game_result::both_loss });
    const auto compiled = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        commands, std::tuple{}, observer, character);
    // Compiling a fixed description owns its values independently of the source array.
    damages[0].value = 9;
    damages[1].value = 9;
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    const auto character_id = compiled.id_map.get_id<givm::character_view>(character.name());
    load_deck(table, compiled.library,
        { .characters = { compiled.id_map.get_id<givm::character_view>(observer.name()) } },
        { .characters = { character_id, character_id } });
    givm::executor executor;
    executor.enter_entry(compiled.library);
    zero_random random;
    REQUIRE(executor.step(compiled.library, table, random) == givm::execution_state::finished);
    CHECK(log.order == std::vector<std::pair<phase, std::size_t>>{
        { phase::calculation, 0 }, { phase::effect, 0 }, { phase::calculation, 1 }, { phase::effect, 1 },
        { phase::after_damage, 0 }, { phase::after_damage, 1 }
    });
    CHECK(log.after_health == std::vector<std::vector<std::uint32_t>>{ { 8, 7 }, { 8, 7 } });
    CHECK(log.after_second_aura == std::vector{ givm::element_aura::none, givm::element_aura::pyro });
}

TEST_CASE("all damage broadcast phases resume after their response programs", "[deal_damage][group][nested][observation]")
{
    const bool dynamic = GENERATE(false, true);
    const bool observed = GENERATE(false, true);
    group_log log{ .enchant = true, .invoke_each_phase = true };
    const auto observer = givm::test::with_passive_skill(group_source{ &log });
    const givm::test::initialized_character_source front{ "Front",
        { .max_health = 10, .health = 10, .aura = givm::element_aura::cryo } };
    const givm::test::initialized_character_source back{ "Back" };
    const std::array damages{
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 2, .type = givm::damage_type::physical },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 1 }, .value = 3, .type = givm::damage_type::physical }
    };
    std::vector<givm::any_command> commands;
    if(dynamic) commands.emplace_back(givm::test_command{});
    else commands.emplace_back(givm::deal_damage{ .damages = damages });
    commands.emplace_back(givm::end_game{ givm::game_result::both_loss });
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        commands, std::tuple{}, observer, front, back);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { ids.get_id<givm::character_view>(front.name()), ids.get_id<givm::character_view>(back.name()) } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    std::size_t response_pauses = 0;
    std::size_t observed_damage = 0;
    for(;;)
    {
        const auto state = executor.step(library, table, random);
        if(state == givm::execution_state::finished) break;
        if(state == givm::execution_state::card_selection)
        {
            ++response_pauses;
            executor.view_in<givm::execution_state::card_selection>().select({});
        }
        else
        {
            REQUIRE(observed);
            REQUIRE(state == givm::execution_state::health_reduced);
            ++observed_damage;
        }
    }
    CHECK(log.order == std::vector<std::pair<phase, std::size_t>>{
        { phase::reaction, 0 }, { phase::calculation, 0 }, { phase::effect, 0 },
        { phase::calculation, 1 }, { phase::effect, 1 }, { phase::calculation, 1 }, { phase::effect, 1 },
        { phase::after_reaction, 0 }, { phase::after_damage, 0 }, { phase::after_damage, 1 }, { phase::after_damage, 1 }
    });
    CHECK(table.state().round_number == 0);
    CHECK(response_pauses == 11);
    CHECK(observed_damage == (observed ? 3 : 0));
    CHECK(table[victim(0)].state().health == 7);
    CHECK(table[victim(1)].state().health == 6);
}

TEST_CASE("defeat checks game end before clearing attachments and energy", "[deal_damage][group][defeat][observation]")
{
    const bool observed = GENERATE(false, true);
    const bool terminal = GENERATE(false, true);
    death_log log;
    const auto dying = givm::test::with_passive_skill(dying_character_source{ &log });
    const givm::test::initialized_character_source character{ "Alive" };
    const std::array damages{
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::physical },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = terminal ? givm::relative_character_target{ givm::relative_player::self, 0 } : givm::relative_character_target{ givm::relative_player::opponent, 1 },
            .value = 1, .type = givm::damage_type::physical }
    };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::test_command{}, givm::deal_damage{ .damages = damages },
            givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, dying, character,
        givm::test::named_definition_source<givm::skill_view>{ "RetainedSkill" },
        death_attachment_source{ &log, "RemovedAttachment", {} },
        death_attachment_source{ &log, "RemovedArtifact", { "artifact" } });
    givm::linked_deck defending_deck{ .characters = { ids.get_id<givm::character_view>(dying.name()) } };
    if(!terminal) defending_deck.characters.push_back(ids.get_id<givm::character_view>(character.name()));
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(character.name()) } }, defending_deck);
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    auto state = executor.step(library, table, random);
    if(observed)
    {
        REQUIRE(state == givm::execution_state::health_reduced);
        CHECK(executor.view_in<givm::execution_state::health_reduced>().target() == victim(0));
        CHECK(table[victim(0)].state().health == 0);
        CHECK(table[victim(0)].state().energy == 3);
        CHECK(std::ranges::distance(table[victim(0)].attachments()) == 2);
        CHECK(table[victim(0)].has(givm::equipment_type::artifact));
        state = executor.step(library, table, random);
        if(!terminal)
        {
            REQUIRE(state == givm::execution_state::health_reduced);
            CHECK(executor.view_in<givm::execution_state::health_reduced>().target() == victim(1));
            CHECK(table[victim(0)].state().energy == 0);
            CHECK(std::ranges::empty(table[victim(0)].attachments()));
            CHECK(log.completed.empty());
            state = executor.step(library, table, random);
        }
    }
    REQUIRE(state == givm::execution_state::finished);
    CHECK(table[victim(0)].is_valid());
    CHECK(table[victim(0)].state().health == 0);
    CHECK(std::ranges::distance(table[victim(0)].skills()) == 2);
    CHECK(log.attachment_responses == 0);
    CHECK(table[attacker].state().health == 10);
    if(terminal)
    {
        CHECK(table[victim(0)].state().energy == 3);
        CHECK(std::ranges::distance(table[victim(0)].attachments()) == 2);
        CHECK(table[victim(0)].has(givm::equipment_type::artifact));
        CHECK(log.completed.empty());
        CHECK(executor.view_in<givm::execution_state::finished>().result() == givm::game_result::player_0_win);
    }
    else
    {
        CHECK(table[victim(0)].state().energy == 0);
        CHECK(std::ranges::empty(table[victim(0)].attachments()));
        CHECK_FALSE(table[victim(0)].has(givm::equipment_type::artifact));
        CHECK(log.completed == std::vector{ victim(0), victim(1) });
        CHECK(table[victim(1)].state().health == 9);
    }
}

TEST_CASE("reactions use the calculated element and expand over the living opposing characters", "[deal_damage][group][reaction]")
{
    const auto aura = GENERATE(givm::element_aura::cryo, givm::element_aura::hydro);
    const std::size_t count = GENERATE(2uz, 5uz);
    group_log log{ .enchant = true };
    const auto observer = givm::test::with_passive_skill(group_source{ &log });
    const givm::test::initialized_character_source front{ "Front", { .max_health = 10, .health = 1, .aura = aura } };
    const givm::test::initialized_character_source back{ "Back" };
    const std::array damages{ givm::fixed_damage{
        .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::physical } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::set_active_character{ .target = givm::relative_character_target{ givm::relative_player::opponent, 0 } }, givm::deal_damage{ .damages = damages },
            givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, observer, front, back);
    givm::linked_deck defending_deck{ .characters = { ids.get_id<givm::character_view>(front.name()) } };
    for(std::size_t i = 1; i < count; ++i)
        defending_deck.characters.push_back(ids.get_id<givm::character_view>(back.name()));
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } }, defending_deck);
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    std::vector<std::uint32_t> expected_health(count, 9);
    expected_health[0] = 0;
    REQUIRE(log.after_health.size() == count + 1);
    for(const auto& health : log.after_health) CHECK(health == expected_health);
    REQUIRE(log.final_types.size() == count);
    CHECK(log.final_types[0] == givm::damage_type::electro);
    for(std::size_t i = 1; i < count; ++i) CHECK(log.final_types[i] == givm::damage_type::piercing);
    CHECK(table[defending_player].state().active_character == victim(0));
    CHECK(table[victim(0)].state().aura == givm::element_aura::none);
}

TEST_CASE("reaction damage finishes before the next initial description", "[deal_damage][group][reaction]")
{
    group_log log;
    const auto observer = givm::test::with_passive_skill(group_source{ &log });
    const givm::test::initialized_character_source character{ "Victim",
        { .max_health = 10, .health = 10, .aura = givm::element_aura::cryo } };
    const std::array damages{
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::electro },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 1 }, .value = 5, .type = givm::damage_type::physical }
    };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::deal_damage{ .damages = damages }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, observer, character);
    const auto id = ids.get_id<givm::character_view>(character.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { id, id, id } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(targets_at(log, phase::effect) == std::vector<std::size_t>{ 0, 1, 2, 1 });
    CHECK(targets_at(log, phase::after_damage) == std::vector<std::size_t>{ 0, 1, 2, 1 });
    for(const auto& health : log.after_health) CHECK(health == std::vector<std::uint32_t>{ 8, 4, 9 });
}

TEST_CASE("swirled damage can expand another reaction inside the same group", "[deal_damage][group][reaction]")
{
    group_log log;
    const auto observer = givm::test::with_passive_skill(group_source{ &log });
    const givm::test::initialized_character_source front{ "Front",
        { .max_health = 10, .health = 10,
            .aura = givm::element_aura::cryo } };
    const givm::test::initialized_character_source electro{ "Electro",
        { .max_health = 10, .health = 10, .aura = givm::element_aura::electro } };
    const givm::test::initialized_character_source empty{ "Empty" };
    const std::array damages{ givm::fixed_damage{
        .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1,
        .type = givm::damage_type::anemo } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::deal_damage{ .damages = damages }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, observer, front, electro, empty);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { ids.get_id<givm::character_view>(front.name()), ids.get_id<givm::character_view>(electro.name()),
            ids.get_id<givm::character_view>(empty.name()) } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(targets_at(log, phase::effect) == std::vector<std::size_t>{ 0, 1, 2, 0, 2 });
    CHECK(targets_at(log, phase::after_damage) == std::vector<std::size_t>{ 0, 1, 2, 0, 2 });
    CHECK(targets_at(log, phase::after_reaction) == std::vector<std::size_t>{ 0, 1 });
    for(const auto& health : log.after_health) CHECK(health == std::vector<std::uint32_t>{ 8, 8, 8 });
    CHECK(table[victim(2)].state().aura == givm::element_aura::cryo);
}

TEST_CASE("replacing a reaction suppresses its extra damage while consuming the aura", "[deal_damage][group][reaction]")
{
    group_log log{ .take_over = true };
    const auto observer = givm::test::with_passive_skill(group_source{ &log });
    const givm::test::initialized_character_source character{ "Victim",
        { .max_health = 10, .health = 10, .aura = givm::element_aura::cryo } };
    const std::array damages{ givm::fixed_damage{
        .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::electro } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::deal_damage{ .damages = damages }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, observer, character);
    const auto id = ids.get_id<givm::character_view>(character.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { id, id, id } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(targets_at(log, phase::effect) == std::vector<std::size_t>{ 0 });
    CHECK(table[victim(0)].state().aura == givm::element_aura::none);
    for(const auto& health : log.after_health) CHECK(health == std::vector<std::uint32_t>{ 9, 10, 10 });
}

TEST_CASE("relative and other-character damage targets skip defeated characters and wrap", "[deal_damage][group][target]")
{
    group_log log;
    const auto observer = givm::test::with_passive_skill(group_source{ &log });
    const givm::test::initialized_character_source alive{ "Alive" };
    const givm::test::initialized_character_source dead{ "Dead", { .max_health = 10, .health = 0 } };
    const std::array damages{
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 1 },
            .value = 1, .type = givm::damage_type::physical },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, -1 },
            .value = 2, .type = givm::damage_type::physical },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0, givm::character_selection::others },
            .value = 3, .type = givm::damage_type::piercing }
    };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::set_active_character{ .target = givm::relative_character_target{ givm::relative_player::opponent, 2 } }, givm::deal_damage{ .damages = damages },
            givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, observer, alive, dead);
    const auto alive_id = ids.get_id<givm::character_view>(alive.name());
    const auto dead_id = ids.get_id<givm::character_view>(dead.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { alive_id, dead_id, alive_id, dead_id } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(targets_at(log, phase::effect) == std::vector<std::size_t>{ 0, 2, 0 });
    for(const auto& health : log.after_health) CHECK(health == std::vector<std::uint32_t>{ 6, 0, 8, 0 });
}

TEST_CASE("relative damage selections share a living anchor and visit each selected character once", "[deal_damage][group][target]")
{
    const bool observed = GENERATE(false, true);
    const bool lone_survivor = GENERATE(false, true);
    const auto offset = GENERATE(-1, -6);
    const auto selection = GENERATE(givm::character_selection::character,
        givm::character_selection::others,
        givm::character_selection::all);
    group_log log;
    const auto observer = givm::test::with_passive_skill(group_source{ &log });
    const givm::test::initialized_character_source alive{ "Alive" };
    const givm::test::initialized_character_source dead{ "Dead", { .max_health = 10, .health = 0 } };
    const std::array damages{ givm::fixed_damage{
        .source = { givm::relative_player::self, 0 }, .target = { givm::relative_player::opponent, offset, selection },
        .value = 1, .type = givm::damage_type::physical } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::deal_damage{ .damages = damages }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, observer, alive, dead);
    const auto alive_id = ids.get_id<givm::character_view>(alive.name());
    const auto dead_id = ids.get_id<givm::character_view>(dead.name());
    const auto other_id = lone_survivor ? dead_id : alive_id;
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = attacker }, { .active_character = victim(2) } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { other_id, dead_id, alive_id, dead_id, other_id } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    std::vector<std::size_t> observations;
    for(;;)
    {
        const auto state = executor.step(library, table, random);
        if(state == givm::execution_state::finished) break;
        REQUIRE(observed);
        REQUIRE(state == givm::execution_state::health_reduced);
        observations.push_back(executor.view_in<givm::execution_state::health_reduced>().target().index);
    }
    std::vector<std::size_t> expected;
    if(selection != givm::character_selection::others) expected.push_back(2);
    if(selection != givm::character_selection::character && not lone_survivor)
    {
        expected.push_back(4);
        expected.push_back(0);
    }
    CHECK(targets_at(log, phase::effect) == expected);
    CHECK(targets_at(log, phase::after_damage) == expected);
    CHECK(observations == (observed ? expected : std::vector<std::size_t>{}));
    CHECK(table[victim(2)].state().health == (selection == givm::character_selection::others ? 10 : 9));
    for(const auto index : { 0uz, 4uz })
        CHECK(table[victim(index)].state().health == (lone_survivor ? 0
            : selection == givm::character_selection::character ? 10 : 9));
    CHECK(table[victim(1)].state().health == 0);
    CHECK(table[victim(3)].state().health == 0);
}

TEST_CASE("damage groups copied at health observation resume independently", "[deal_damage][group][observation]")
{
    group_log log;
    const auto observer = givm::test::with_passive_skill(group_source{ &log });
    const givm::test::initialized_character_source character{ "Victim" };
    const std::array damages{
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 2, .type = givm::damage_type::physical },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 1 }, .value = 3, .type = givm::damage_type::physical }
    };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::observed,
        std::tuple{ givm::deal_damage{ .damages = damages }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, observer, character);
    const auto id = ids.get_id<givm::character_view>(character.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { id, id } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    REQUIRE(executor.step(library, table, random) == givm::execution_state::health_reduced);
    CHECK(executor.view_in<givm::execution_state::health_reduced>().target() == victim(0));
    CHECK(table[victim(0)].state().health == 8);
    CHECK(table[victim(1)].state().health == 10);
    CHECK(log.after_health.empty());
    auto copied_executor = executor;
    auto copied_table = table;
    const auto saved_order = log.order;
    const auto resume = [&](givm::executor& current_executor, givm::table& current_table)
    {
        REQUIRE(current_executor.step(library, current_table, random) == givm::execution_state::health_reduced);
        CHECK(current_executor.view_in<givm::execution_state::health_reduced>().target() == victim(1));
        CHECK(log.after_health.empty());
        REQUIRE(current_executor.step(library, current_table, random) == givm::execution_state::finished);
        CHECK(log.after_health == std::vector<std::vector<std::uint32_t>>{ { 8, 7 }, { 8, 7 } });
    };
    resume(executor, table);
    const auto completed_order = log.order;
    log.order = saved_order;
    log.after_health.clear();
    resume(copied_executor, copied_table);
    CHECK(log.order == completed_order);
    CHECK(copied_table[victim(0)].state().health == table[victim(0)].state().health);
    CHECK(copied_table[victim(1)].state().health == table[victim(1)].state().health);
}

TEST_CASE("a nested damage command completes its own group before resuming the caller", "[deal_damage][group][nested]")
{
    group_log log{ .nested = true };
    const auto observer = givm::test::with_passive_skill(group_source{ &log });
    const givm::test::initialized_character_source character{ "Victim" };
    const std::array damages{
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 2, .type = givm::damage_type::physical },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 1 }, .value = 3, .type = givm::damage_type::physical }
    };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::deal_damage{ .damages = damages }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, observer, character);
    const auto id = ids.get_id<givm::character_view>(character.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { id, id, id } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(log.order == std::vector<std::pair<phase, std::size_t>>{
        { phase::calculation, 0 }, { phase::calculation, 2 }, { phase::effect, 2 }, { phase::after_damage, 2 },
        { phase::effect, 0 }, { phase::calculation, 1 }, { phase::effect, 1 },
        { phase::after_damage, 0 }, { phase::after_damage, 1 }
    });
    CHECK(log.after_health == std::vector<std::vector<std::uint32_t>>{ { 10, 10, 6 }, { 8, 7, 6 }, { 8, 7, 6 } });
}

TEST_CASE("queued damage skips a target defeated by an earlier response and continues", "[deal_damage][group][nested][target]")
{
    const bool observed = GENERATE(false, true);
    group_log log{ .nested = true };
    const auto observer = givm::test::with_passive_skill(group_source{ &log });
    const givm::test::initialized_character_source alive{ "Alive" };
    const givm::test::initialized_character_source fragile{ "Fragile", { .max_health = 10, .health = 1 } };
    const std::array damages{ givm::fixed_damage{
        .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 4, givm::character_selection::others },
        .value = 1, .type = givm::damage_type::physical } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::deal_damage{ .damages = damages }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, observer, alive, fragile);
    const auto alive_id = ids.get_id<givm::character_view>(alive.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { alive_id, alive_id, ids.get_id<givm::character_view>(fragile.name()), alive_id, alive_id } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    std::vector<std::size_t> observed_targets;
    for(;;)
    {
        const auto state = executor.step(library, table, random);
        if(state == givm::execution_state::finished) break;
        REQUIRE(observed);
        REQUIRE(state == givm::execution_state::health_reduced);
        observed_targets.push_back(executor.view_in<givm::execution_state::health_reduced>().target().index);
        REQUIRE(observed_targets.size() <= 4);
    }
    CHECK(targets_at(log, phase::calculation) == std::vector<std::size_t>{ 0, 2, 1, 3 });
    CHECK(targets_at(log, phase::effect) == std::vector<std::size_t>{ 2, 0, 1, 3 });
    CHECK(targets_at(log, phase::after_damage) == std::vector<std::size_t>{ 2, 0, 1, 3 });
    CHECK(log.after_health == std::vector<std::vector<std::uint32_t>>{
        { 10, 10, 0, 10, 10 }, { 9, 9, 0, 9, 10 }, { 9, 9, 0, 9, 10 }, { 9, 9, 0, 9, 10 }
    });
    CHECK(observed_targets == (observed ? std::vector<std::size_t>{ 2, 0, 1, 3 } : std::vector<std::size_t>{}));
}

TEST_CASE("standalone element application has reaction effects without damage", "[apply_element][group][reaction]")
{
    const bool observed = GENERATE(false, true);
    const auto incoming = GENERATE(givm::element::electro, givm::element::anemo, givm::element::pyro);
    const auto aura = GENERATE(givm::element_aura::cryo, givm::element_aura::hydro);
    group_log log;
    const auto observer = givm::test::with_passive_skill(group_source{ &log });
    const givm::test::initialized_character_source character{ "Victim",
        { .max_health = 10, .health = 10, .aura = aura } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = incoming },
            givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, observer, character);
    const auto id = ids.get_id<givm::character_view>(character.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { id, id, id } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(targets_at(log, phase::calculation).empty());
    CHECK(targets_at(log, phase::effect).empty());
    CHECK(targets_at(log, phase::after_damage).empty());
    CHECK(targets_at(log, phase::reaction) == std::vector<std::size_t>{ 0 });
    CHECK(targets_at(log, phase::after_reaction) == std::vector<std::size_t>{ 0 });
    CHECK(log.after_health == std::vector<std::vector<std::uint32_t>>{ { 10, 10, 10 } });
    CHECK(table[victim(0)].state().aura == givm::element_aura::none);
    CHECK(table[victim(1)].state().aura == aura);
    CHECK(table[victim(2)].state().aura == aura);
}

TEST_CASE("a damage group stops before completion responses when the last character is defeated", "[deal_damage][group][game-result]")
{
    group_log log;
    const auto observer = givm::test::with_passive_skill(group_source{ &log });
    const givm::test::initialized_character_source character{ "Victim", { .max_health = 10, .health = 1 } };
    const std::array damages{
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::physical },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 1 }, .value = 1, .type = givm::damage_type::physical }
    };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::deal_damage{ .damages = damages }, givm::replace_cards{ .player = attacking_player },
            givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, observer, character);
    const auto id = ids.get_id<givm::character_view>(character.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { id, id } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(executor.view_in<givm::execution_state::finished>().result() == givm::game_result::player_0_win);
    CHECK(targets_at(log, phase::effect) == std::vector<std::size_t>{ 0, 1 });
    CHECK(targets_at(log, phase::after_damage).empty());
    CHECK(log.after_health.empty());
    CHECK(table.state().round_number == 0);
}

TEST_CASE("an independent nested damage group can end the game before its caller resumes", "[deal_damage][group][nested][game-result]")
{
    group_log log{ .nested_after_first = true };
    const auto observer = givm::test::with_passive_skill(group_source{ &log });
    const givm::test::initialized_character_source character{ "Victim", { .max_health = 10, .health = 1 } };
    const std::array damages{
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::physical },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 1 }, .value = 1, .type = givm::damage_type::physical }
    };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::deal_damage{ .damages = damages }, givm::replace_cards{ .player = attacking_player },
            givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, observer, character);
    const auto id = ids.get_id<givm::character_view>(character.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { id, id, id } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(executor.view_in<givm::execution_state::finished>().result() == givm::game_result::player_0_win);
    CHECK(targets_at(log, phase::effect) == std::vector<std::size_t>{ 0, 1, 2 });
    CHECK(targets_at(log, phase::after_damage) == std::vector<std::size_t>{ 0 });
    CHECK(log.after_health == std::vector<std::vector<std::uint32_t>>{ { 0, 0, 1 } });
    CHECK(table.state().round_number == 0);
}

TEST_CASE("elemental reaction identity survives aura changes during damage effect responses", "[deal_damage][group][reaction]")
{
    group_log log{ .change_aura_during_effect = true };
    const auto observer = givm::test::with_passive_skill(group_source{ &log });
    const givm::test::initialized_character_source front{ "Front",
        { .max_health = 10, .health = 10, .aura = givm::element_aura::cryo } };
    const givm::test::initialized_character_source back{ "Back" };
    const std::array damages{ givm::fixed_damage{
        .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::electro } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::deal_damage{ .damages = damages }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, observer, front, back);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { ids.get_id<givm::character_view>(front.name()), ids.get_id<givm::character_view>(back.name()) } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(log.reactions == std::vector{ givm::elemental_reaction::superconduct });
    CHECK(log.final_types == std::vector{ givm::damage_type::electro, givm::damage_type::piercing });
    CHECK(table[victim(0)].state().aura == givm::element_aura::pyro);
    CHECK(targets_at(log, phase::after_damage) == std::vector<std::size_t>{ 0, 1 });
    for(const auto& health : log.after_health) CHECK(health == std::vector<std::uint32_t>{ 8, 9 });
}

TEST_CASE("applying no element clears an existing aura without reaction or damage notifications", "[apply_element]")
{
    group_log log;
    const auto observer = givm::test::with_passive_skill(group_source{ &log });
    const givm::test::initialized_character_source character{ "Victim",
        { .max_health = 10, .health = 10, .aura = givm::element_aura::cryo } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::none },
            givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, observer, character);
    const auto id = ids.get_id<givm::character_view>(character.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } },
        { .characters = { id, id } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(table[victim(0)].state().aura == givm::element_aura::none);
    CHECK(table[victim(0)].state().health == 10);
    CHECK(log.order.empty());
}
