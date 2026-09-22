#include <array>
#include <cstdint>
#include <span>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"
#include "../test_character_source.hpp"

namespace
{
    constexpr givm::character_id attacker{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id target(std::size_t index) { return { givm::player_id{ 1 }, index }; }
    enum class replacement { none, first, second, clear };

    struct reaction_log
    {
        replacement mode = replacement::none;
        bool recording = false;
        bool change_current = false;
        bool nested_damage = false;
        bool action_taken = false;
        bool pause_switch = false;
        bool paused = false;
        std::vector<givm::tag_id> previous_tags;
        std::vector<givm::tag_id> calculated_tags;
        std::vector<givm::tag_id> effect_tags;
        std::vector<givm::tag_id> reaction_tags;
        std::vector<givm::tag_id> completion_tags;
        std::vector<givm::elemental_reaction> original_reactions;
        std::vector<std::size_t> active_at_calculation;
        std::vector<std::size_t> active_at_completion;
        std::vector<std::size_t> switches;
        std::vector<std::size_t> switch_responder_order;
        std::vector<std::uint32_t> values;
    };

    struct reacting_skill
    {
        using definition_category = givm::skill_view;
        struct definition_type
        {
            reaction_log* log;
            bool first;
            givm::tag_id tag;
            givm::program_entry change;
            givm::program_entry nested;
            givm::program_entry pause;
        };
        reaction_log* log;
        bool first;
        std::string_view name() const { return first ? "FirstReactionWriter" : "LastReactionWriter"; }
        auto tags() const { return std::array{ std::string_view{ first ? "FirstReplacement" : "LastReplacement" } }; }
        auto tag_dependencies() const { return tags(); }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, first, context.resolve_tag(tags()[0]),
                context.add_program(std::tuple{ givm::set_active_character{} }),
                context.add_program(std::tuple{ givm::deal_damage{} }),
                context.add_program(std::tuple{ givm::replace_cards{ givm::player_id{ 0 } } }) };
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::elemental_reaction_will_occur& event, givm::handle_context&)
        {
            data.log->recording = true;
            if(data.first)
            {
                if(data.log->mode != replacement::none) event.replacement_reaction = data.tag;
            }
            else
            {
                data.log->previous_tags.push_back(event.replacement_reaction);
                if(data.log->mode == replacement::second) event.replacement_reaction = data.tag;
                if(data.log->mode == replacement::clear) event.replacement_reaction = {};
            }
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::damage_calculation& event, givm::handle_context& context)
        {
            if(data.first) return {};
            data.log->calculated_tags.push_back(event.replacement_reaction);
            data.log->original_reactions.push_back(event.reaction);
            data.log->active_at_calculation.push_back(context.table()[givm::player_id{ 1 }].state().active_character->index);
            if(not data.log->action_taken && (data.log->change_current || data.log->nested_damage))
            {
                data.log->action_taken = true;
                if(data.log->change_current)
                    return context.invoke(data.change, givm::active_character_changed{ .current = target(1) });
                return context.invoke(data.nested, givm::damage{
                    .source = attacker, .target = target(0), .value = 1, .type = givm::damage_type::pyro
                });
            }
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::damage_effect& event, givm::handle_context&)
        {
            if(not data.first) data.log->effect_tags.push_back(event.replacement_reaction);
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::after_elemental_reaction& event, givm::handle_context&)
        {
            if(not data.first) data.log->reaction_tags.push_back(event.replacement_reaction);
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::after_damage& event, givm::handle_context& context)
        {
            if(data.first) return {};
            data.log->completion_tags.push_back(event.replacement_reaction);
            data.log->values.push_back(event.value);
            data.log->active_at_completion.push_back(context.table()[givm::player_id{ 1 }].state().active_character->index);
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::active_character_changed& event, givm::handle_context& context)
        {
            if(data.first || not data.log->recording) return {};
            data.log->switches.push_back(event.current.index);
            if(data.log->pause_switch && not data.log->paused)
            {
                data.log->paused = true;
                return context.invoke(data.pause);
            }
            return {};
        }
    };

    struct reacting_character
    {
        using definition_category = givm::character_view;
        struct definition_type { std::array<givm::definition_id<givm::skill_view>, 2> skills; };
        std::string_view name() const { return "OverloadSource"; }
        auto skill_dependencies() const
        {
            return std::array{ std::string_view{ "FirstReactionWriter" }, std::string_view{ "LastReactionWriter" } };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { { context.resolve_id<givm::skill_view>("FirstReactionWriter"),
                context.resolve_id<givm::skill_view>("LastReactionWriter") } };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 20, .health = 20 };
        }
        static givm::definition_id<givm::skill_view> query(const definition_type& data, const givm::character_initial_skill& query)
        {
            return query.skill_index < data.skills.size() ? data.skills[query.skill_index] : givm::definition_id<givm::skill_view>{};
        }
    };

    struct switch_observer_character
    {
        using definition_category = givm::character_view;
        struct definition_type { reaction_log* log; givm::character_state state; };
        reaction_log* log;
        std::string_view source_name;
        givm::character_state state;
        std::string_view name() const { return source_name; }
        definition_type compile(givm::definition_compile_context&) const { return { log, state }; }
        static givm::character_state query(const definition_type& data, const givm::character_initial_state&)
        {
            return data.state;
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            givm::active_character_changed& event, givm::handle_context& context)
        {
            if(data.log->recording)
            {
                CHECK(context.table()[event.current.player_id].state().active_character == event.current);
                data.log->switch_responder_order.push_back(self.id().index);
            }
            return {};
        }
    };

    template<class TProgram>
    auto compile_scenario(const TProgram& program, reaction_log& log, bool observed, std::uint32_t health = 20)
    {
        return givm::test::compile_definitions_with_program(
            observed ? givm::compile_mode::observed : givm::compile_mode::normal, program, std::tuple{},
            reacting_character{}, reacting_skill{ &log, true }, reacting_skill{ &log, false },
            givm::test::with_passive_skill(switch_observer_character{ &log, "Front",
                { .max_health = 20, .health = health, .aura = givm::element_aura::electro } }),
            givm::test::with_passive_skill(switch_observer_character{ &log, "Reserve",
                { .max_health = 20, .health = 20, .aura = givm::element_aura::electro } }),
            givm::test::with_passive_skill(switch_observer_character{ &log, "Defeated", { .max_health = 20, .health = 0 } }));
    }

    void load_scenario(givm::table& table, const givm::definition_library& library,
        const givm::issued_id_map& ids, std::initializer_list<std::string_view> characters)
    {
        givm::linked_deck opponents;
        for(const auto name : characters) opponents.characters.push_back(ids.get_id<givm::character_view>(name));
        load_deck(table, library, { .characters = { ids.get_id<givm::character_view>("OverloadSource") } }, opponents);
    }

    auto damage_program(std::span<const givm::damage> damages)
    {
        return std::tuple{ givm::set_active_character{ target(0) }, givm::deal_damage{ .damages = damages },
            givm::end_game{ givm::game_result::both_loss } };
    }

    struct zero_random { std::uint32_t operator()() const { return 0; } };
}

TEST_CASE("the final reaction replacement tag reaches every damage stage and preserves aura consumption", "[reaction-replacement]")
{
    const auto mode = GENERATE(replacement::first, replacement::second, replacement::clear);
    const bool observed = GENERATE(false, true);
    reaction_log log{ .mode = mode };
    const std::array damages{ givm::damage{ .source = attacker, .target = target(0), .value = 1, .type = givm::damage_type::pyro } };
    const auto [library, ids] = compile_scenario(damage_program(damages), log, observed);
    const auto expected_tag = mode == replacement::clear ? givm::tag_id{}
        : ids.get_tag_id(mode == replacement::first ? "FirstReplacement" : "LastReplacement");
    givm::table table;
    load_scenario(table, library, ids, { "Front", "Reserve" });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    std::size_t observed_damage = 0;
    for(;;)
    {
        const auto state = executor.step(library, table, random);
        if(state == givm::execution_state::finished) break;
        REQUIRE(observed);
        if(state == givm::execution_state::active_character_changed) continue;
        REQUIRE(state == givm::execution_state::health_reduced);
        ++observed_damage;
        const auto damage = executor.view_in<givm::execution_state::health_reduced>();
        CHECK(damage.reaction() == givm::elemental_reaction::overloaded);
        CHECK(damage.replacement_reaction() == expected_tag);
        CHECK(damage.value() == (mode == replacement::clear ? 3 : 1));
    }
    CHECK(observed_damage == (observed ? 1 : 0));
    CHECK(log.previous_tags == std::vector{ ids.get_tag_id("FirstReplacement") });
    CHECK(log.calculated_tags == std::vector{ expected_tag });
    CHECK(log.effect_tags == std::vector{ expected_tag });
    CHECK(log.reaction_tags == std::vector{ expected_tag });
    CHECK(log.completion_tags == std::vector{ expected_tag });
    CHECK(log.original_reactions == std::vector{ givm::elemental_reaction::overloaded });
    CHECK(table[target(0)].state().aura == givm::element_aura::none);
    CHECK(table[target(0)].state().health == (mode == replacement::clear ? 17 : 19));
    CHECK(log.switches == (mode == replacement::clear ? std::vector<std::size_t>{ 1 } : std::vector<std::size_t>{}));
    CHECK(log.switch_responder_order == (mode == replacement::clear ? std::vector<std::size_t>{ 1, 0 }
        : std::vector<std::size_t>{}));
}

TEST_CASE("a damage group overloads only once after all hits and ignores standby triggers", "[overload][group]")
{
    const bool observed = GENERATE(false, true);
    const bool standby_only = GENERATE(false, true);
    reaction_log log;
    const std::array group{
        givm::damage{ .source = attacker, .target = target(0), .value = 1, .type = givm::damage_type::pyro },
        givm::damage{ .source = attacker, .target = target(2), .value = 1, .type = givm::damage_type::pyro },
        givm::damage{ .source = attacker, .target = target(0), .value = 1, .type = givm::damage_type::electro },
        givm::damage{ .source = attacker, .target = target(0), .value = 1, .type = givm::damage_type::pyro }
    };
    const auto damages = standby_only ? std::span{ group }.subspan(1, 1) : std::span{ group }.subspan(0);
    const auto [library, ids] = compile_scenario(damage_program(damages), log, observed);
    givm::table table;
    load_scenario(table, library, ids, { "Front", "Defeated", "Reserve" });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    std::size_t observed_hits = 0;
    for(;;)
    {
        const auto state = executor.step(library, table, random);
        if(state == givm::execution_state::finished) break;
        REQUIRE(observed);
        if(state == givm::execution_state::health_reduced)
        {
            ++observed_hits;
            CHECK(table[givm::player_id{ 1 }].state().active_character == target(0));
            CHECK(log.values.empty());
        }
        else
        {
            REQUIRE(state == givm::execution_state::active_character_changed);
            if(log.recording)
            {
                CHECK(observed_hits == 4);
                CHECK(executor.view_in<givm::execution_state::active_character_changed>().character() == target(2));
                CHECK(log.values.empty());
            }
        }
    }
    const auto expected_values = standby_only ? std::vector<std::uint32_t>{ 3 } : std::vector<std::uint32_t>{ 3, 3, 1, 3 };
    CHECK(log.values == expected_values);
    CHECK(log.active_at_calculation == std::vector<std::size_t>(expected_values.size(), 0));
    CHECK(log.active_at_completion == std::vector<std::size_t>(expected_values.size(), standby_only ? 0 : 2));
    CHECK(log.switches == (standby_only ? std::vector<std::size_t>{} : std::vector<std::size_t>{ 2 }));
    CHECK(log.switch_responder_order == (standby_only ? std::vector<std::size_t>{} : std::vector<std::size_t>{ 2, 0, 1 }));
    CHECK(table[target(0)].state().health == (standby_only ? 20 : 13));
    CHECK(table[target(2)].state().health == 17);
}

TEST_CASE("overload follows the current active character after death or nested responses", "[overload][nested]")
{
    enum class response { defeated, changed, nested };
    const auto behavior = GENERATE(response::defeated, response::changed, response::nested);
    const bool observed = GENERATE(false, true);
    reaction_log log{ .change_current = behavior == response::changed, .nested_damage = behavior == response::nested };
    const std::array damages{ givm::damage{ .source = attacker, .target = target(0), .value = 1, .type = givm::damage_type::pyro } };
    const auto [library, ids] = compile_scenario(damage_program(damages), log, observed,
        behavior == response::defeated ? 1 : 20);
    givm::table table;
    load_scenario(table, library, ids, { "Front", "Reserve", "Reserve" });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    for(;;)
    {
        const auto state = executor.step(library, table, random);
        if(state == givm::execution_state::finished) break;
        REQUIRE(observed);
        REQUIRE((state == givm::execution_state::health_reduced || state == givm::execution_state::active_character_changed));
    }
    CHECK(log.switches == (behavior == response::defeated ? std::vector<std::size_t>{ 1 } : std::vector<std::size_t>{ 1, 2 }));
    const auto expected_active = behavior == response::defeated ? 1uz : 2uz;
    CHECK(table[givm::player_id{ 1 }].state().active_character == target(expected_active));
    CHECK(log.active_at_completion == (behavior == response::nested ? std::vector<std::size_t>{ 1, 2 }
        : std::vector<std::size_t>{ expected_active }));
    CHECK(table[target(0)].state().health == (behavior == response::defeated ? 0 : behavior == response::nested ? 14 : 17));
}

TEST_CASE("copied overload switch responses resume before group completion exactly once", "[overload][observation]")
{
    reaction_log log{ .pause_switch = true };
    const std::array damages{ givm::damage{ .source = attacker, .target = target(0), .value = 1, .type = givm::damage_type::pyro } };
    const auto [library, ids] = compile_scenario(damage_program(damages), log, true);
    givm::table table;
    load_scenario(table, library, ids, { "Front", "Reserve" });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    for(;;)
    {
        const auto state = executor.step(library, table, random);
        if(state == givm::execution_state::card_selection) break;
        REQUIRE((state == givm::execution_state::health_reduced || state == givm::execution_state::active_character_changed));
    }
    CHECK(log.switches == std::vector<std::size_t>{ 1 });
    CHECK(log.values.empty());
    CHECK(table[givm::player_id{ 1 }].state().active_character == target(1));
    auto copy = executor;
    auto copy_table = table;
    executor.view_in<givm::execution_state::card_selection>().select({});
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(log.values == std::vector<std::uint32_t>{ 3 });
    CHECK(log.active_at_completion == std::vector<std::size_t>{ 1 });
    log.values.clear();
    log.active_at_completion.clear();
    copy.view_in<givm::execution_state::card_selection>().select({});
    REQUIRE(copy.step(library, copy_table, random) == givm::execution_state::finished);
    CHECK(log.values == std::vector<std::uint32_t>{ 3 });
    CHECK(log.active_at_completion == std::vector<std::size_t>{ 1 });
    CHECK(log.switches == std::vector<std::size_t>{ 1 });
    CHECK(copy_table.state().round_number == 0);
}

TEST_CASE("overload from element application respects missing alternatives and game termination", "[overload][apply_element]")
{
    enum class scenario { application, lone_application, lone_damage, terminal };
    const auto kind = GENERATE(scenario::application, scenario::lone_application, scenario::lone_damage, scenario::terminal);
    const bool observed = GENERATE(false, true);
    reaction_log log;
    const std::array damages{ givm::damage{ .source = attacker, .target = target(0), .value = 1, .type = givm::damage_type::pyro } };
    std::vector<givm::any_command> program{ givm::set_active_character{ target(0) } };
    if(kind == scenario::application || kind == scenario::lone_application)
        program.emplace_back(givm::apply_element{ .source = attacker, .target = target(0), .element = givm::element::pyro });
    else program.emplace_back(givm::deal_damage{ .damages = damages });
    program.emplace_back(givm::end_game{ givm::game_result::both_loss });
    const auto [library, ids] = compile_scenario(program, log, observed, kind == scenario::terminal ? 1 : 20);
    givm::table table;
    if(kind == scenario::application) load_scenario(table, library, ids, { "Front", "Reserve" });
    else load_scenario(table, library, ids, { "Front" });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    std::size_t overload_observations = 0;
    for(;;)
    {
        const auto state = executor.step(library, table, random);
        if(state == givm::execution_state::finished) break;
        REQUIRE(observed);
        if(state == givm::execution_state::active_character_changed)
        {
            if(log.recording) ++overload_observations;
        }
        else REQUIRE(state == givm::execution_state::health_reduced);
    }
    CHECK(log.switches == (kind == scenario::application ? std::vector<std::size_t>{ 1 } : std::vector<std::size_t>{}));
    CHECK(overload_observations == (observed && kind == scenario::application ? 1 : 0));
    CHECK(executor.view_in<givm::execution_state::finished>().result()
        == (kind == scenario::terminal ? givm::game_result::player_0_win : givm::game_result::both_loss));
    if(kind == scenario::terminal)
    {
        CHECK(log.values.empty());
        CHECK(log.reaction_tags.empty());
    }
    else
    {
        CHECK(table[target(0)].state().aura == givm::element_aura::none);
        CHECK(table[target(0)].state().health == (kind == scenario::lone_damage ? 17 : 20));
    }
}
