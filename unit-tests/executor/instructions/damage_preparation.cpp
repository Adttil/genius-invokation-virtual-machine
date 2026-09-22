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
    constexpr givm::character_id source{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id front{ givm::player_id{ 1 }, 0 };
    constexpr givm::character_id back{ givm::player_id{ 1 }, 1 };

    struct preparation_log
    {
        std::vector<givm::elemental_reaction> calculated;
        std::vector<givm::element_aura> reacted_auras;
        std::vector<givm::elemental_reaction> applied;
        std::vector<givm::elemental_reaction> completed;
        std::vector<givm::elemental_reaction> side_effects;
        std::vector<std::array<std::uint32_t, 2>> health_at_completion;
        std::size_t normal_bonuses = 0;
        std::size_t burst_bonuses = 0;
        bool replace_reaction_bonus = false;
        bool change_aura_in_calculation = false;
    };

    struct damage_bonus_source
    {
        using definition_category = givm::combat_status_view;
        struct definition_type { preparation_log* log; givm::program_entry change_aura; givm::tag_id replacement; };
        preparation_log* log;
        std::string_view name() const { return "DamageBonus"; }
        auto tags() const { return std::array{ std::string_view{ "PreparedReactionReplacement" } }; }
        auto tag_dependencies() const { return tags(); }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_program(std::tuple{
                givm::set_element_aura{ .target = front, .aura = givm::element_aura::pyro }
            }), context.resolve_tag("PreparedReactionReplacement") };
        }
        static givm::program_entry handle(const definition_type& data, const givm::combat_status_view&,
            givm::damage_calculation& event, givm::handle_context& context)
        {
            data.log->calculated.push_back(event.reaction);
            data.log->reacted_auras.push_back(event.reacted_aura);
            if(event.type == givm::damage_type::cryo) ++event.value;
            if(event.flags.contains(givm::damage_flag_bits::normal_attack))
            {
                event.value += 2;
                ++data.log->normal_bonuses;
            }
            if(event.flags.contains(givm::damage_flag_bits::elemental_burst))
            {
                event.value += 3;
                ++data.log->burst_bonuses;
            }
            if(event.replacement_reaction == data.replacement)
            {
                event.value += 5;
            }
            if(data.log->change_aura_in_calculation && event.target == front)
                return context.invoke(data.change_aura);
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::combat_status_view&,
            givm::damage_effect& event, givm::handle_context&)
        {
            data.log->applied.push_back(event.reaction);
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::combat_status_view&,
            givm::elemental_reaction_will_occur& event, givm::handle_context&)
        {
            data.log->side_effects.push_back(event.reaction);
            CHECK_FALSE(event.replacement_reaction.is_valid());
            if(data.log->replace_reaction_bonus) event.replacement_reaction = data.replacement;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::combat_status_view&,
            givm::after_damage& event, givm::handle_context& context)
        {
            data.log->completed.push_back(event.reaction);
            data.log->health_at_completion.push_back({ context.table()[front].state().health,
                context.table()[back].state().health });
            return {};
        }
    };

    struct infusion_source
    {
        using definition_category = givm::combat_status_view;
        struct definition_type { givm::program_entry pause; givm::damage_type type; bool classify; };
        givm::damage_type type;
        bool classify;
        std::string_view name() const { return "Infusion"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.add_program(std::tuple{ givm::start_round{} }), type, classify };
        }
        static givm::program_entry handle(const definition_type& data, const givm::combat_status_view&,
            givm::damage_preparation& event, givm::handle_context& context)
        {
            if(event.type != givm::damage_type::physical) return {};
            event.type = data.type;
            if(data.classify)
            {
                event.flags.set(givm::damage_flag_bits::normal_attack);
                event.flags.set(givm::damage_flag_bits::elemental_burst);
            }
            return context.invoke(data.pause);
        }
    };

    struct preparation_driver
    {
        using definition_category = givm::skill_view;
        struct definition_type { givm::program_entry entry; };
        std::span<const givm::damage> damages;
        std::string_view name() const { return "PreparationDriver"; }
        auto combat_status_dependencies() const
        {
            return std::array{ std::string_view{ "DamageBonus" }, std::string_view{ "Infusion" } };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.add_program(std::tuple{
                givm::add_combat_status{ .definition = context.resolve_id<givm::combat_status_view>("DamageBonus") },
                givm::add_combat_status{ .definition = context.resolve_id<givm::combat_status_view>("Infusion") },
                givm::deal_damage{ .damages = damages }
            }) };
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::test_event&, givm::handle_context& context)
        {
            return context.invoke(data.entry);
        }
    };

    struct preparation_character
    {
        using definition_category = givm::character_view;
        struct definition_type { givm::definition_id<givm::skill_view> driver; };
        std::string_view name() const { return "PreparationCharacter"; }
        auto skill_dependencies() const { return std::array{ std::string_view{ "PreparationDriver" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_id<givm::skill_view>("PreparationDriver") };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 20, .health = 20 };
        }
        static givm::definition_id<givm::skill_view> query(const definition_type& data, const givm::character_initial_skill& query)
        {
            return query.skill_index == 0 ? data.driver : givm::definition_id<givm::skill_view>{};
        }
    };

    struct zero_random { std::uint32_t operator()() const { return 0; } };
}

TEST_CASE("infusion precedes earlier bonuses and damage can count as both normal attack and burst", "[deal_damage][preparation]")
{
    const bool observed = GENERATE(false, true);
    const bool grouped = GENERATE(false, true);
    preparation_log log{ .change_aura_in_calculation = true };
    const std::array damages{
        givm::damage{ .source = source, .target = front, .value = 1,
            .multiplier_numerator = 3, .multiplier_denominator = 2, .type = givm::damage_type::physical },
        givm::damage{ .source = source, .target = back, .value = 1,
            .multiplier_numerator = 2, .multiplier_denominator = 3, .type = givm::damage_type::physical }
    };
    const preparation_character character;
    const preparation_driver driver{ std::span{ damages }.first(grouped ? 2 : 1) };
    const damage_bonus_source bonus{ &log };
    const infusion_source infusion{ givm::damage_type::cryo, true };
    const givm::test::initialized_character_source target{ "Target", { .max_health = 20, .health = 20,
        .aura = givm::element_aura::hydro } };
    const givm::test::initialized_character_source reserve{ "Reserve", { .max_health = 20, .health = 20 } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::test_command{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
        character, driver, bonus, infusion, target, reserve);
    givm::table table;
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(character.name()) } },
        { .characters = { ids.get_id<givm::character_view>(target.name()), ids.get_id<givm::character_view>(reserve.name()) } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    std::vector<givm::elemental_reaction> observed_reactions;
    std::size_t pauses = 0;
    for(;;)
    {
        const auto state = executor.step(library, table, random);
        if(state == givm::execution_state::finished) break;
        REQUIRE(observed);
        if(state == givm::execution_state::round_started) ++pauses;
        else
        {
            REQUIRE(state == givm::execution_state::health_reduced);
            const auto damage = executor.view_in<givm::execution_state::health_reduced>();
            observed_reactions.push_back(damage.reaction());
            CHECK(damage.value() == (damage.target() == front ? 10 : 4));
            CHECK(damage.type() == givm::damage_type::cryo);
            CHECK(damage.flags().contains(givm::damage_flag_bits::normal_attack));
            CHECK(damage.flags().contains(givm::damage_flag_bits::elemental_burst));
        }
    }
    const auto expected = grouped ? std::vector{ givm::elemental_reaction::frozen, givm::elemental_reaction::none }
        : std::vector{ givm::elemental_reaction::frozen };
    CHECK(log.calculated == expected);
    CHECK(log.applied == expected);
    CHECK(log.completed == expected);
    CHECK(log.reacted_auras.front() == givm::element_aura::hydro);
    CHECK(log.side_effects == std::vector{ givm::elemental_reaction::frozen });
    CHECK(log.normal_bonuses == (grouped ? 2 : 1));
    CHECK(log.burst_bonuses == (grouped ? 2 : 1));
    CHECK(table[front].state().health == 10);
    CHECK(table[back].state().health == (grouped ? 16 : 20));
    CHECK(table.state().round_number == (grouped ? 2 : 1));
    CHECK(pauses == (observed ? (grouped ? 2 : 1) : 0));
    CHECK(observed_reactions == (observed ? expected : std::vector<givm::elemental_reaction>{}));
    for(const auto health : log.health_at_completion)
        CHECK(health == std::array<std::uint32_t, 2>{ 10, grouped ? 16u : 20u });
}

TEST_CASE("replacement reaction numbers are applied without default secondary damage", "[deal_damage][preparation][reaction]")
{
    preparation_log log{ .replace_reaction_bonus = true };
    const std::array damages{ givm::damage{ .source = source, .target = front, .value = 2,
        .type = givm::damage_type::physical } };
    const preparation_character character;
    const preparation_driver driver{ damages };
    const damage_bonus_source bonus{ &log };
    const infusion_source infusion{ givm::damage_type::electro, false };
    const givm::test::initialized_character_source target{ "Target", { .max_health = 20, .health = 20,
        .aura = givm::element_aura::cryo } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::test_command{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
        character, driver, bonus, infusion, target);
    givm::table table;
    const auto target_id = ids.get_id<givm::character_view>(target.name());
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(character.name()) } },
        { .characters = { target_id, target_id } });
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(table[front].state().health == 13);
    CHECK(table[back].state().health == 20);
    CHECK(table[front].state().aura == givm::element_aura::none);
    CHECK(log.side_effects == std::vector{ givm::elemental_reaction::superconduct });
    const std::vector expected{ givm::elemental_reaction::superconduct };
    CHECK(log.calculated == expected);
    CHECK(log.applied == expected);
    CHECK(log.completed == expected);
    for(const auto health : log.health_at_completion) CHECK(health == std::array<std::uint32_t, 2>{ 13, 20 });
}
