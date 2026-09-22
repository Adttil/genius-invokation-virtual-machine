#include <array>
#include <cstdint>
#include <string_view>
#include <string>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../../table/test_definition_library.hpp"
#include "../test_character_source.hpp"

namespace
{
    struct technique_log
    {
        std::vector<std::string> events;
        bool cancelled = false;
    };

    struct technique_source
    {
        using definition_category = givm::attachment_view;
        struct definition_type
        {
            technique_log* log;
            givm::program_entry pause;
            givm::program_entry payment;
        };
        technique_log* log;
        constexpr std::string_view name() const { return "Technique"; }
        constexpr auto tags() const { return std::array{ std::string_view{ "technique" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log,
                context.add_program(std::tuple{ givm::replace_cards{ .player = givm::player_id{ 0 } } }),
                context.add_program(std::tuple{ givm::modify_attachment_state{} }) };
        }
        static givm::action_cost_requirement query(const definition_type&, const givm::technique_initial_cost&)
        {
            return { .dice_requirement = { .any = 2 }, .energy = 1 };
        }
        static givm::target_validation query(const definition_type&, const givm::technique_target_validation& query)
        {
            if(query.target_count == 0) return givm::target_validation::valid_incomplete;
            const auto* target = std::get_if<givm::character_id>(&query.targets[0]);
            return target && target->player_id != query.technique.player().id()
                ? givm::target_validation::valid_complete : givm::target_validation::invalid;
        }
        static givm::program_entry handle(const definition_type& data, const givm::attachment_view& self,
                                         givm::cost_of_technique& event, givm::handle_context& context)
        {
            --event.requirement.dice_requirement.any;
            data.log->events.push_back("quote");
            return context.invoke(givm::substack_t{}, data.payment,
                givm::attachment_state_modification{ .attachment = self.id(), .count = -1 });
        }
        static givm::program_entry handle(const definition_type& data, const givm::attachment_view&,
                                         givm::technique_will_be_used& event, givm::handle_context&)
        {
            data.log->events.push_back("before");
            event.effect_cancelled = data.log->cancelled;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::attachment_view& self,
                                         givm::technique_effect& event, givm::handle_context& context)
        {
            CHECK(event.technique == self.id());
            CHECK(std::get<givm::character_id>(event.targets[0]).player_id == givm::player_id{ 1 });
            CHECK(self.state().count == 1);
            CHECK(context.table()[self.character().id()].state().energy == 2);
            data.log->events.push_back("effect");
            return context.invoke(data.pause);
        }
        static givm::program_entry handle(const definition_type& data, const givm::attachment_view&,
                                         givm::technique_used& event, givm::handle_context&)
        {
            CHECK(event.effect_cancelled == data.log->cancelled);
            data.log->events.push_back("after");
            return {};
        }
    };

    struct technique_owner
    {
        using definition_category = givm::character_view;
        struct definition_type { givm::program_entry equip; };
        constexpr std::string_view name() const { return "TechniqueOwner"; }
        constexpr auto attachment_dependencies() const { return std::array{ std::string_view{ "Technique" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.add_program(std::tuple{ givm::add_attachment{
                .definition = context.resolve_id<givm::attachment_view>("Technique"), .state = { 2 }
            } }) };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .max_energy = 3, .health = 10, .energy = 3 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
                                         givm::action_phase_started&, givm::handle_context& context)
        {
            return context.invoke(data.equip);
        }
    };

    inline givm::execution_state advance(givm::executor& executor, const givm::definition_library& library, givm::table& table)
    {
        auto random = []() -> std::uint32_t { return 0; };
        auto state = executor.step(library, table, random);
        while(state == givm::execution_state::active_character_changed || state == givm::execution_state::action_started)
            state = executor.step(library, table, random);
        return state;
    }
}

TEST_CASE("technique selection pays cached costs and resumes effect and notification independently", "[technique][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool cancelled = GENERATE(false, true);
    technique_log log{ .cancelled = cancelled };
    const auto owner = givm::test::with_passive_skill(technique_owner{});
    const givm::test::initialized_character_source plain;
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{
            givm::set_active_character{ givm::character_id{ givm::player_id{ 0 }, 0 } },
            givm::set_active_character{ givm::character_id{ givm::player_id{ 1 }, 0 } },
            givm::start_dice_roll_phase{ .count = 4, .reroll_count = { 0, 0 } }, givm::begin_action{}
        }, std::tuple{}, owner, plain, technique_source{ &log });
    givm::table table;
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(owner.name()) } },
        { .characters = { ids.get_id<givm::character_view>(plain.name()) } });
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    const auto action = executor.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.has_technique());
    CHECK_FALSE(action.is_controlled(library, table));
    const auto technique = action.technique_id();
    CHECK(table[technique].character().get(givm::equipment_type::technique).id() == technique);
    const auto& cost = action.calculate_technique_cost(library, table);
    CHECK(cost.requirement.dice_requirement.any == 1);
    CHECK(cost.requirement.energy == 1);
    CHECK(action.technique_cost().technique == technique);
    CHECK(table[technique].state().count == 2);
    CHECK(log.events == std::vector<std::string>{ "quote" });
    givm::dice_counts payment;
    payment[givm::elemental_dice::omni] = 1;
    CHECK(action.technique_payment_validate(table, {}) == givm::technique_payment_validation::requirement_mismatch);
    CHECK(action.technique_payment_validate(table, payment) == givm::technique_payment_validation::valid);
    CHECK(action.technique_targets_validate(library, table) == givm::target_validation::valid_incomplete);
    const std::array<givm::technique_target_id, 1> targets{ givm::character_id{ givm::player_id{ 1 }, 0 } };
    CHECK(action.technique_targets_validate(library, table, targets) == givm::target_validation::valid_complete);
    action.use_technique(payment, targets);
    if(not cancelled)
    {
        REQUIRE(advance(executor, library, table) == givm::execution_state::card_selection);
        auto branch = executor;
        auto branch_table = table;
        branch.view_in<givm::execution_state::card_selection>().select({});
        REQUIRE(advance(branch, library, branch_table) == givm::execution_state::action_selection);
        CHECK_FALSE(branch.view_in<givm::execution_state::action_selection>().has_technique());
        log.events.pop_back();
        executor.view_in<givm::execution_state::card_selection>().select({});
    }
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    CHECK(table[technique].state().count == 1);
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 3);
    CHECK(table[technique.character_id].state().energy == 2);
    CHECK(log.events == (cancelled ? std::vector<std::string>{ "quote", "before", "after" }
        : std::vector<std::string>{ "quote", "before", "effect", "after" }));
    CHECK_FALSE(executor.view_in<givm::execution_state::action_selection>().has_technique());
}
