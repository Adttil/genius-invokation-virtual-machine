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

namespace givm_test::executor_instructions::damage_preparation
{
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
        std::vector<givm::elemental_reaction_mask> completed;
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
        struct definition_type { preparation_log* log; givm::immediate_effect change_aura; givm::tag_id replacement; };
        preparation_log* log;
        std::string_view name() const { return "DamageBonus"; }
        auto tags() const { return std::array{ std::string_view{ "PreparedReactionReplacement" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_immediate_effect(std::tuple{
                givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::none },
                    givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::pyro }
            }), *context.find_tag("PreparedReactionReplacement") };
        }
        static givm::immediate_effect handle(const definition_type& data,
            givm::damage_calculation& event, givm::handle_context<givm::combat_status_view, givm::event_category::immediate>& context, std::uint32_t = 0)
        {
            data.log->calculated.push_back(event.reaction.slot);
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
            if(data.log->replace_reaction_bonus)
            {
                event.value += 5;
                event.cancel_reaction_bonus = true;
            }
            if(data.log->change_aura_in_calculation && event.target == front)
                return context.invoke(data.change_aura);
            return {};
        }
        static givm::immediate_effect handle(const definition_type& data,
            givm::damage_effect& event, givm::handle_context<givm::combat_status_view, givm::event_category::immediate>&, std::uint32_t = 0)
        {
            data.log->applied.push_back(event.reaction.slot);
            return {};
        }
        static givm::immediate_effect handle(const definition_type& data,
            givm::elemental_reaction_will_occur& event, givm::handle_context<givm::combat_status_view, givm::event_category::immediate>&, std::uint32_t = 0)
        {
            data.log->side_effects.push_back(event.reaction.slot);
            CHECK_FALSE(event.cancel_default_effects);
            if(data.log->replace_reaction_bonus) event.cancel_default_effects = true;
            return {};
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::after_damage& event, givm::handle_context<givm::combat_status_view>& context, std::uint32_t = 0)
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
        struct definition_type { givm::immediate_effect pause; givm::damage_type type; bool classify; };
        givm::damage_type type;
        bool classify;
        std::string_view name() const { return "Infusion"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.add_immediate_effect(std::tuple{ givm::replace_cards{ .player = givm::player_id{ 0 } } }), type, classify };
        }
        static givm::immediate_effect handle(const definition_type& data,
            givm::damage_preparation& event, givm::handle_context<givm::combat_status_view, givm::event_category::immediate>& context, std::uint32_t = 0)
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
        struct definition_type { givm::normal_effect entry; };
        std::span<const givm::deal_damage> damages;
        std::string_view name() const { return "PreparationDriver"; }
        auto combat_status_dependencies() const
        {
            return std::array{ std::string_view{ "DamageBonus" }, std::string_view{ "Infusion" } };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            std::vector<givm::any_command> commands{
                givm::add_combat_status{ .definition = context.resolve_id<givm::combat_status_view>("DamageBonus") },
                givm::add_combat_status{ .definition = context.resolve_id<givm::combat_status_view>("Infusion") } };
            for(const auto& item : damages) commands.emplace_back(item);
            return { context.add_normal_effect(commands) };
        }

        static givm::normal_effect handle(const definition_type& data,
            givm::round_started&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
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
        givm::deal_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1,
            .multiplier_numerator = 3, .multiplier_denominator = 2, .type = givm::damage_type::physical },
        givm::deal_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 1 }, .value = 1,
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
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
        character, driver, bonus, infusion, target, reserve);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(character.name()) } },
        { .characters = { ids.get_id<givm::character_view>(target.name()), ids.get_id<givm::character_view>(reserve.name()) } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    std::vector<givm::elemental_reaction> observed_reactions;
    std::size_t pauses = 0;
    for(;;)
    {
        const auto state = executor.advance(library, table, random);
        if(state == givm::execution_state::finished) break;
        if(state == givm::execution_state::card_selection)
        {
            ++pauses;
            executor.submitted(executor.view_in<givm::execution_state::card_selection>().select(library, table, random, {}));
        }
        else
        {
            REQUIRE(observed);
            REQUIRE(state == givm::execution_state::health_reduced);
            const auto damage = executor.view_in<givm::execution_state::health_reduced>();
            observed_reactions.push_back(damage.reaction().slot);
            CHECK(damage.value() == (damage.target() == front ? 12 : 5));
            CHECK(damage.type() == givm::damage_type::cryo);
            CHECK(damage.flags().contains(givm::damage_flag_bits::normal_attack));
            CHECK(damage.flags().contains(givm::damage_flag_bits::elemental_burst));
        }
    }
    const auto expected = grouped ? std::vector{ givm::elemental_reaction::frozen, givm::elemental_reaction::none }
        : std::vector{ givm::elemental_reaction::frozen };
    CHECK(log.calculated == expected);
    CHECK(log.applied == expected);
    CHECK(log.completed == std::vector<givm::elemental_reaction_mask>(expected.begin(), expected.end()));
    CHECK(log.reacted_auras.front() == givm::element_aura::hydro);
    CHECK(log.side_effects == std::vector{ givm::elemental_reaction::frozen });
    CHECK(log.normal_bonuses == (grouped ? 2 : 1));
    CHECK(log.burst_bonuses == (grouped ? 2 : 1));
    CHECK(table[front].state().health == 8);
    CHECK(table[front].state().aura == givm::element_aura::none);
    CHECK(table[back].state().health == (grouped ? 15 : 20));
    CHECK(table.state().round_number == 0);
    CHECK(pauses == (grouped ? 2 : 1));
    CHECK(observed_reactions == (observed ? expected : std::vector<givm::elemental_reaction>{}));
    for(const auto health : log.health_at_completion)
        CHECK(health == std::array<std::uint32_t, 2>{ 8, grouped ? 15u : 20u });
}

TEST_CASE("replacement reaction numbers are applied without default secondary damage", "[deal_damage][preparation][reaction]")
{
    preparation_log log{ .replace_reaction_bonus = true };
    const std::array damages{ givm::deal_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 2,
        .type = givm::damage_type::physical } };
    const preparation_character character;
    const preparation_driver driver{ damages };
    const damage_bonus_source bonus{ &log };
    const infusion_source infusion{ givm::damage_type::electro, false };
    const givm::test::initialized_character_source target{ "Target", { .max_health = 20, .health = 20,
        .aura = givm::element_aura::cryo } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
        character, driver, bonus, infusion, target);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    const auto target_id = ids.get_id<givm::character_view>(target.name());
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(character.name()) } },
        { .characters = { target_id, target_id } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::card_selection);
    executor.submitted(executor.view_in<givm::execution_state::card_selection>().select(library, table, random, {}));
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::finished);
    CHECK(table[front].state().health == 13);
    CHECK(table[back].state().health == 20);
    CHECK(table[front].state().aura == givm::element_aura::none);
    CHECK(log.side_effects == std::vector{ givm::elemental_reaction::superconduct });
    const std::vector expected{ givm::elemental_reaction::superconduct };
    CHECK(log.calculated == expected);
    CHECK(log.applied == expected);
    CHECK(log.completed == std::vector<givm::elemental_reaction_mask>(expected.begin(), expected.end()));
    for(const auto health : log.health_at_completion) CHECK(health == std::array<std::uint32_t, 2>{ 13, 20 });
}
}
