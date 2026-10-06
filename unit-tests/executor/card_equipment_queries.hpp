#include <array>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <variant>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../table/test_definition_library.hpp"
#include "test_character_source.hpp"

namespace givm_test::executor::card_equipment_queries
{
namespace
{
    struct observation
    {
        bool initialized = false;
        unsigned ordinary_queries = 0;
        unsigned discounts = 0;
        unsigned card_effects = 0;
    };

    struct talent_source
    {
        using definition_category = givm::card_definition;
        struct definition_type
        {
            givm::definition_id<givm::character_view> character;
            bool active_only;
            observation* log;
        };
        std::string_view source_name;
        std::string_view character_name;
        bool active_only;
        observation* log;

        std::string_view name() const { return source_name; }
        auto character_dependencies() const { return std::array{ character_name }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_id<givm::character_view>(character_name), active_only, log };
        }
        static givm::card_state query(const definition_type&, const givm::card_initial_state&)
        {
            return { .cost = { .dice_requirement = { .any = 2 } } };
        }
        static bool query(const definition_type& data, const givm::card_equipment_target_validation& parameters)
        {
            const auto target = parameters.character;
            return parameters.card.player().id() == target.player().id()
                && target.definition_id() == data.character && target.state().health != 0
                && (not data.active_only || target.player().state().active_character == target.id());
        }
        static givm::target_validation query(const definition_type& data, const givm::card_target_validation& parameters)
        {
            if(parameters.target_count == 0) return givm::target_validation::valid_incomplete;
            if(parameters.target_count != 1) return givm::target_validation::invalid;
            const auto target = std::get_if<givm::character_id>(&parameters.targets[0]);
            return target && query(data, givm::card_equipment_target_validation{ parameters.card, parameters.table[*target] })
                ? givm::target_validation::valid_complete : givm::target_validation::invalid;
        }
        static givm::program_entry handle(const definition_type& data,
            givm::card_effect&, givm::handle_context<givm::hand_card_view>&, std::uint32_t = 0)
        {
            ++data.log->card_effects;
            return {};
        }
    };

    struct artifact_source
    {
        using definition_category = givm::attachment_view;
        struct definition_type { observation* log; givm::program_entry payment; };
        observation* log;

        std::string_view name() const { return "TalentDiscountArtifact"; }
        auto tags() const { return std::array<std::string_view, 1>{ "artifact" }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_program(std::tuple{ givm::modify_attachment_state{} }) };
        }
        static givm::program_entry handle(const definition_type& data,
            givm::cost_of_card& event, givm::handle_context<givm::attachment_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity();
            if(self.state().round_usages == 0 || event.requirement.dice_requirement.any == 0) return {};
            const auto card = context.table()[event.card];
            if(not context.query(card.definition_id(), givm::card_equipment_target_validation{ card, self.character() }))
                return {};
            ++data.log->discounts;
            --event.requirement.dice_requirement.any;
            return context.invoke(givm::substack_t{}, data.payment,
                givm::modify_attachment_state_input{ .attachment = self.id(), .round_usages = -1 });
        }
        static givm::program_entry handle(const definition_type& data,
            givm::card_drawn& event, givm::handle_context<givm::attachment_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity();
            const auto card = context.table()[event.card];
            ++data.log->ordinary_queries;
            (void)context.query(card.definition_id(), givm::card_equipment_target_validation{ card, self.character() });
            CHECK(context.query(self.character().definition_id(), givm::character_initial_state{}).health == 10);
            return {};
        }
    };

    struct setup_skill_source
    {
        using definition_category = givm::skill_view;
        struct definition_type
        {
            observation* log;
            givm::definition_id<givm::attachment_view> artifact;
            givm::program_entry setup;
        };
        observation* log;

        std::string_view name() const { return "TalentQuerySetup"; }
        auto attachment_dependencies() const { return std::array<std::string_view, 1>{ "TalentDiscountArtifact" }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.resolve_id<givm::attachment_view>("TalentDiscountArtifact"),
                context.add_program(std::tuple{ givm::add_attachment{}, givm::add_attachment{} }) };
        }
        static givm::program_entry handle(const definition_type& data,
            givm::battle_started&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity();
            if(self.player().id() != givm::player_id{ 0 } || data.log->initialized) return {};
            data.log->initialized = true;
            auto characters = self.player().characters();
            auto current = characters.begin();
            const auto first = (*current++).id();
            const auto second = (*current).id();
            return context.invoke(data.setup,
                givm::add_attachment_input{ first, data.artifact, { 1, 1 } },
                givm::add_attachment_input{ second, data.artifact, { 1, 1 } });
        }
    };

    struct character_source
    {
        using definition_category = givm::character_view;
        using definition_type = givm::definition_id<givm::skill_view>;

        std::string_view name() const { return "TalentCharacter"; }
        auto skill_dependencies() const { return std::array<std::string_view, 1>{ "TalentQuerySetup" }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return context.resolve_id<givm::skill_view>("TalentQuerySetup");
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static definition_type query(const definition_type& data, const givm::character_initial_skill& parameters)
        {
            return parameters.skill_index == 0 ? data : definition_type{};
        }
    };

    givm::execution_state advance(executor_driver& executor, const givm::definition_library& library, givm::table& table)
    {
        auto state = executor.advance(library, table, omni_random);
        while(state == givm::execution_state::action_started)
            state = executor.advance(library, table, omni_random);
        return state;
    }
}

TEST_CASE("equipment target queries discount only the uniquely applicable talent and defer payment effects", "[query][payment][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool active_only = GENERATE(false, true);
    observation log;
    const character_source character;
    const givm::test::initialized_character_source other{ "OtherCharacter" };
    const givm::test::initialized_character_source absent{ "AbsentCharacter" };
    const talent_source talent{ "ApplicableTalent", character.name(), active_only, &log };
    const talent_source wrong{ "DifferentTalent", absent.name(), false, &log };
    const givm::test::named_definition_source<givm::card_definition> plain{ "OrdinaryCard" };
    // Destroy the original library before executing: every context must use its supplied copy.
    auto [library, ids] = [&]
    {
        const auto [original, map] = givm::test::compile_definitions_with_program(mode,
            std::tuple{ givm::start_battle{}, givm::settle{}, givm::draw_cards{ .position = 0, .count = 3 }, givm::settle{},
                givm::start_dice_roll_phase{ .count = 4, .reroll_count = { 0, 0 } }, givm::begin_action{} },
            std::tuple{}, character, other, absent, talent, wrong, plain, artifact_source{ &log }, setup_skill_source{ &log });
        return std::pair{ original, map };
    }();
    const auto character_id = ids.get_id<givm::character_view>(character.name());
    const auto other_id = ids.get_id<givm::character_view>(other.name());
    givm::table table{ { .round_number = 1, .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, {
        .cards = { ids.get_id<givm::card_definition>(plain.name()), ids.get_id<givm::card_definition>(wrong.name()),
            ids.get_id<givm::card_definition>(talent.name()) },
        .characters = { active_only ? character_id : other_id, character_id }
    }, { .characters = { other_id } });
    executor_driver executor;
    executor.start(library, table);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    REQUIRE(log.initialized);
    CHECK(log.ordinary_queries == 6);
    const auto action = executor.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.card_count() == 3);
    CHECK(action.calculate_card_cost(library, table, 0).requirement.dice_requirement.any == 1);
    CHECK(action.calculate_card_cost(library, table, 1).requirement.dice_requirement.any == 2);
    CHECK(action.calculate_card_cost(library, table, 2).requirement.dice_requirement.any == 0);
    CHECK(log.discounts == 1);
    const auto default_card = table[action.card_id(2)];
    const auto first_character = *table[givm::player_id{ 0 }].characters().begin();
    CHECK_FALSE(library.query(default_card.definition_id(), givm::card_equipment_target_validation{ default_card, first_character }));
    for(const auto target : table[givm::player_id{ 0 }].characters())
        CHECK(target.get(givm::equipment_type::artifact).state().round_usages == 1);

    const givm::character_id target{ givm::player_id{ 0 }, active_only ? 0u : 1u };
    const std::array<givm::card_target_id, 1> targets{ target };
    givm::dice_counts payment;
    payment[givm::elemental_dice::omni] = 1;
    executor.submitted(action.play_card_with_cached_cost(library, table, omni_random, 0, payment, targets));
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    CHECK(log.discounts == 1);
    CHECK(log.card_effects == 1);
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 3);
    for(const auto current : table[givm::player_id{ 0 }].characters())
        CHECK(current.get(givm::equipment_type::artifact).state().round_usages == (current.id() == target ? 0 : 1));
}
}
