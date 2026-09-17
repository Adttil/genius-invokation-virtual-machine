#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../../table/test_definition_library.hpp"
#include "../test_character_source.hpp"

namespace
{
    using skill_targets = std::array<givm::skill_target_id, 2>;

    struct skill_log
    {
        std::vector<std::size_t> initial_skill_indices;
        std::vector<std::size_t> target_counts;
        std::vector<skill_targets> validated_targets;
        std::vector<skill_targets> effect_targets;
        std::vector<skill_targets> used_targets;
        std::vector<givm::skill_id> effects;
        std::vector<std::string> events;
        std::vector<std::array<std::uint32_t, 2>> resources_at_broadcast;
        std::vector<std::array<std::uint32_t, 2>> resources_at_draw;
        std::uint32_t initial_cost_queries = 0;
        std::uint32_t cost_queries = 0;
        std::uint32_t passive_responses = 0;
        std::uint32_t card_effects = 0;
        std::uint32_t extra_energy = 0;
        std::uint8_t extra_dice = 0;
        bool nested = false;
        bool cancelled = false;
        bool record = false;
        bool fast = false;
    };

    std::array<std::uint32_t, 2> resources(const givm::table& table)
    {
        const auto player = table[givm::player_id{ 0 }];
        return { player.state().dice.total(), table[*player.state().active_character].state().energy };
    }

    struct active_skill_source
    {
        using definition_category = givm::skill_view;
        struct definition_type
        {
            skill_log* log;
            givm::action_cost_requirement cost;
            givm::program_entry<givm::skill_effect> effect;
        };
        skill_log* log;
        std::uint8_t dice = 1;
        std::uint32_t energy = 2;

        std::string_view name() const noexcept { return "ActiveSkill"; }
        auto tags() const { return std::array{ std::string_view{ "elemental_burst" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, { .dice_requirement = { .any = dice }, .energy = energy },
                context.add_program<givm::skill_effect>(std::tuple{ givm::draw_cards{ .count = 1 } }) };
        }
        static givm::action_cost_requirement query(const definition_type& data, const givm::skill_initial_cost&)
        {
            ++data.log->initial_cost_queries;
            return data.cost;
        }
        static givm::target_validation query(const definition_type& data, const givm::skill_target_validation& query)
        {
            data.log->target_counts.push_back(query.target_count);
            data.log->validated_targets.push_back(query.targets);
            CHECK(query.table[query.skill.id()].definition_id() == query.skill.definition_id());
            if(query.target_count == 0) return givm::target_validation::valid_complete_or_continue;
            const auto* current = std::get_if<givm::character_id>(&query.targets[query.target_count - 1]);
            if(current == nullptr || current->player_id == query.skill.player().id())
                return givm::target_validation::invalid;
            return query.target_count == 1 ? givm::target_validation::valid_incomplete
                : givm::target_validation::valid_complete;
        }
        static givm::program_entry<givm::skill_effect> handle(
            const definition_type& data, const givm::skill_view& self,
            givm::skill_effect& event, const givm::table& table, givm::random_fn&)
        {
            CHECK(event.skill == self.id());
            CHECK(table[event.skill].is_valid());
            data.log->effects.push_back(event.skill);
            data.log->effect_targets.push_back(event.targets);
            data.log->events.push_back("effect");
            return data.log->nested ? data.effect : givm::program_entry<givm::skill_effect>::null();
        }
    };

    struct untargeted_skill_source
    {
        using definition_category = givm::skill_view;
        struct definition_type { skill_log* log; };
        skill_log* log;
        std::string_view name() const noexcept { return "UntargetedSkill"; }
        definition_type compile(givm::definition_compile_context&) const { return { log }; }
        static givm::program_entry<givm::skill_effect> handle(
            const definition_type& data, const givm::skill_view& self,
            givm::skill_effect& event, const givm::table&, givm::random_fn&)
        {
            CHECK(event.skill == self.id());
            data.log->effects.push_back(event.skill);
            data.log->effect_targets.push_back(event.targets);
            return givm::program_entry<givm::skill_effect>::null();
        }
    };

    struct passive_skill_source
    {
        using definition_category = givm::skill_view;
        struct definition_type { skill_log* log; };
        skill_log* log;
        std::string_view name() const noexcept { return "PassiveSkill"; }
        definition_type compile(givm::definition_compile_context&) const { return { log }; }
        static givm::program_entry<givm::before_action> handle(
            const definition_type& data, const givm::skill_view&, givm::before_action&, const givm::table&, givm::random_fn&)
        {
            ++data.log->passive_responses;
            return givm::program_entry<givm::before_action>::null();
        }
    };

    struct skill_character_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            skill_log* log;
            givm::definition_id<givm::skill_view> active;
            givm::definition_id<givm::skill_view> passive;
            givm::definition_id<givm::skill_view> untargeted;
            givm::handler_program_entry_t<givm::cost_of_skill> payment;
            givm::program_entry<givm::skill_will_be_used> before;
            givm::program_entry<givm::skill_used> after;
            givm::program_entry<givm::dice_removed> dice_removed;
            givm::program_entry<givm::energy_changed> energy_changed;
            givm::program_entry<givm::card_drawn> selection;
        };
        skill_log* log;
        std::string_view name() const noexcept { return "SkillCharacter"; }
        auto skill_dependencies() const
        {
            return std::array{ std::string_view{ "ActiveSkill" }, std::string_view{ "PassiveSkill" },
                std::string_view{ "UntargetedSkill" } };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return {
                log,
                context.resolve_id<givm::skill_view>("ActiveSkill"),
                context.resolve_id<givm::skill_view>("PassiveSkill"),
                context.resolve_id<givm::skill_view>("UntargetedSkill"),
                context.add_program<givm::handler_program_context_t<givm::cost_of_skill>>(
                    std::tuple{ givm::draw_cards{ .count = 1 } }),
                context.add_program<givm::skill_will_be_used>(std::tuple{ givm::draw_cards{ .count = 1 } }),
                context.add_program<givm::skill_used>(std::tuple{ givm::draw_cards{ .count = 1 } }),
                context.add_program<givm::dice_removed>(std::tuple{ givm::draw_cards{ .count = 1 } }),
                context.add_program<givm::energy_changed>(std::tuple{ givm::draw_cards{ .count = 1 } }),
                context.add_program<givm::card_drawn>(std::tuple{ givm::replace_cards{ .player = givm::player_id{ 0 } } })
            };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .max_energy = 3, .health = 10, .energy = 3 };
        }
        static givm::definition_id<givm::skill_view> query(const definition_type& data, const givm::character_initial_skill& query)
        {
            data.log->initial_skill_indices.push_back(query.skill_index);
            switch(query.skill_index)
            {
            case 0: return data.active;
            case 1: return data.passive;
            case 2: return data.untargeted;
            default: return {};
            }
        }
        static givm::handler_program_entry_t<givm::cost_of_skill> handle(
            const definition_type& data, const givm::character_view&, givm::cost_of_skill& event,
            const givm::table&, givm::random_fn&)
        {
            ++data.log->cost_queries;
            event.requirement.dice_requirement.any += data.log->extra_dice;
            event.requirement.energy += data.log->extra_energy;
            return data.log->nested ? data.payment : givm::handler_program_entry_t<givm::cost_of_skill>::null();
        }
        static givm::handler_program_entry_t<givm::cost_of_switch> handle(
            const definition_type& data, const givm::character_view&, givm::cost_of_switch& event,
            const givm::table&, givm::random_fn&)
        {
            event.requirement.dice_requirement.any = data.log->extra_dice;
            event.requirement.energy = data.log->extra_energy;
            return givm::handler_program_entry_t<givm::cost_of_switch>::null();
        }
        static givm::program_entry<givm::dice_removed> handle(
            const definition_type& data, const givm::character_view&, givm::dice_removed&,
            const givm::table& table, givm::random_fn&)
        {
            data.log->events.push_back("dice");
            data.log->resources_at_broadcast.push_back(resources(table));
            return data.log->nested ? data.dice_removed : givm::program_entry<givm::dice_removed>::null();
        }
        static givm::program_entry<givm::energy_changed> handle(
            const definition_type& data, const givm::character_view&, givm::energy_changed& event,
            const givm::table& table, givm::random_fn&)
        {
            CHECK(event.target == givm::character_id{ givm::player_id{ 0 }, 0 });
            CHECK(event.previous == 3);
            CHECK(event.current == table[event.target].state().energy);
            data.log->events.push_back("energy");
            data.log->resources_at_broadcast.push_back(resources(table));
            return data.log->nested ? data.energy_changed : givm::program_entry<givm::energy_changed>::null();
        }
        static givm::program_entry<givm::skill_will_be_used> handle(
            const definition_type& data, const givm::character_view&, givm::skill_will_be_used& event,
            const givm::table&, givm::random_fn&)
        {
            data.log->events.push_back("will");
            event.effect_cancelled = data.log->cancelled;
            if(data.log->fast) event.speed = givm::action_speed::fast;
            return data.log->nested ? data.before : givm::program_entry<givm::skill_will_be_used>::null();
        }
        static givm::program_entry<givm::skill_used> handle(
            const definition_type& data, const givm::character_view&, givm::skill_used& event,
            const givm::table&, givm::random_fn&)
        {
            CHECK(event.effect_cancelled == data.log->cancelled);
            CHECK(event.speed == (data.log->fast ? givm::action_speed::fast : givm::action_speed::combat));
            data.log->used_targets.push_back(event.targets);
            data.log->events.push_back("used");
            return data.log->nested ? data.after : givm::program_entry<givm::skill_used>::null();
        }
        static givm::program_entry<givm::card_drawn> handle(
            const definition_type& data, const givm::character_view&, givm::card_drawn&,
            const givm::table& table, givm::random_fn&)
        {
            if(not data.log->record) return givm::program_entry<givm::card_drawn>::null();
            data.log->events.push_back("draw");
            data.log->resources_at_draw.push_back(resources(table));
            return data.log->nested ? data.selection : givm::program_entry<givm::card_drawn>::null();
        }
    };

    struct energy_card_source
    {
        using definition_category = givm::card_definition;
        struct definition_type { skill_log* log; std::uint8_t dice; std::uint32_t energy; };
        skill_log* log;
        std::uint8_t dice;
        std::uint32_t energy;
        std::string_view name() const noexcept { return "EnergyCard"; }
        definition_type compile(givm::definition_compile_context&) const { return { log, dice, energy }; }
        static givm::action_cost_requirement query(const definition_type& data, const givm::card_initial_cost&)
        {
            return { .dice_requirement = { .any = data.dice }, .energy = data.energy, .speed = givm::action_speed::fast };
        }
        static givm::program_entry<givm::card_effect> handle(
            const definition_type& data, const givm::hand_card_view&, givm::card_effect&, const givm::table&, givm::random_fn&)
        {
            ++data.log->card_effects;
            return givm::program_entry<givm::card_effect>::null();
        }
    };

    struct enter_skill_character_source
    {
        using definition_category = givm::character_view;
        struct definition_type { givm::program_entry<givm::test_event> entry; };
        std::string_view name() const noexcept { return "EnterSkillCharacter"; }
        auto character_dependencies() const { return std::array{ std::string_view{ "SkillCharacter" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.add_program<givm::test_event>(std::tuple{
                givm::enter_character{ .player = givm::player_id{ 1 },
                    .definition = context.resolve_id<givm::character_view>("SkillCharacter") }
            }) };
        }
        static givm::program_entry<givm::test_event> handle(
            const definition_type& data, const givm::character_view&, givm::test_event&, const givm::table&, givm::random_fn&)
        {
            return data.entry;
        }
    };

    struct counting_random
    {
        std::uint32_t calls = 0;
        std::uint32_t operator()() noexcept { ++calls; return 0; }
    };

    givm::dice_counts pay(std::uint8_t count, givm::elemental_dice kind = givm::elemental_dice::omni)
    {
        givm::dice_counts result;
        result[kind] = count;
        return result;
    }

    auto setup(std::uint32_t cards = 0)
    {
        return std::tuple{
            givm::set_active_character{ givm::character_id{ givm::player_id{ 0 }, 0 } },
            givm::set_active_character{ givm::character_id{ givm::player_id{ 1 }, 0 } },
            givm::draw_cards{ .count = cards },
            givm::start_dice_roll_phase{ .count = 4, .reroll_count = { 0, 0 } },
            givm::begin_action{}
        };
    }

    givm::execution_state advance(givm::executor& target, const givm::definition_library& library,
                                 givm::table& table, counting_random& random)
    {
        auto state = target.step(library, table, random);
        while(state == givm::execution_state::active_character_changed || state == givm::execution_state::action_started)
            state = target.step(library, table, random);
        return state;
    }
}

TEST_CASE("deck loading initializes indexed skills once and only active skills become action candidates", "[use_skill][deck][query][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    skill_log log;
    const active_skill_source active{ &log };
    const passive_skill_source passive{ &log };
    const untargeted_skill_source untargeted{ &log };
    const skill_character_source owner{ &log };
    const givm::test::initialized_character_source plain;
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        setup(),
        std::tuple{}, active, passive, untargeted, owner, plain);
    CHECK(log.initial_skill_indices.empty());
    CHECK(log.initial_cost_queries == 1);
    const auto owner_id = ids.get_id<givm::character_view>(owner.name());
    const auto plain_id = ids.get_id<givm::character_view>(plain.name());
    CHECK_FALSE(library[plain_id].query(givm::character_initial_skill{ 0 }).is_valid());
    givm::table table;
    load_deck(table, library, { .characters = { owner_id, plain_id } }, { .characters = { plain_id } });
    CHECK(log.initial_skill_indices == std::vector<std::size_t>{ 0, 1, 2, 3 });
    CHECK(table[givm::character_id{ givm::player_id{ 0 }, 1 }].skills().empty());
    CHECK(table[givm::character_id{ givm::player_id{ 1 }, 0 }].skills().empty());
    const auto character = table[givm::character_id{ givm::player_id{ 0 }, 0 }];
    CHECK(character.state().health == 10);
    CHECK(character.state().energy == 3);
    std::vector<givm::skill_id> skills;
    for(const auto skill : character.skills())
    {
        skills.push_back(skill.id());
        CHECK(skill.character().id() == character.id());
        CHECK(skill.state().count == 0);
    }
    REQUIRE(skills.size() == 3);
    CHECK(table[skills[0]].definition_id() == ids.get_id<givm::skill_view>(active.name()));
    CHECK(table[skills[1]].definition_id() == ids.get_id<givm::skill_view>(passive.name()));
    CHECK(table[skills[2]].definition_id() == ids.get_id<givm::skill_view>(untargeted.name()));
    CHECK(log.passive_responses == 0);
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    CHECK(log.initial_skill_indices == std::vector<std::size_t>{ 0, 1, 2, 3 });
    CHECK(log.passive_responses == 1);
    const auto action = target.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.skill_count() == 2);
    CHECK(action.skill_id(0) == skills[0]);
    CHECK(action.skill_id(1) == skills[2]);
    CHECK(action.skill_cost(0).skill == skills[0]);
    CHECK(action.skill_cost(1).skill == skills[2]);
    CHECK(log.cost_queries == 0);
    const auto& cost = action.calculate_skill_cost(library, table, 1);
    CHECK(cost.requirement.dice_requirement.fixed.total() == 0);
    CHECK(cost.requirement.dice_requirement.same == 0);
    CHECK(cost.requirement.dice_requirement.any == 0);
    CHECK(cost.requirement.energy == 0);
    CHECK(cost.requirement.speed == givm::action_speed::combat);
    CHECK(action.skill_targets_validate(library, table, 1) == givm::target_validation::valid_complete);
    const std::array<givm::skill_target_id, 1> invalid_targets{ character.id() };
    CHECK(action.skill_targets_validate(library, table, 1, invalid_targets) == givm::target_validation::invalid);
    action.use_skill(1, {});
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    CHECK(log.effects == std::vector{ skills[2] });
    CHECK(log.effect_targets == std::vector{ skill_targets{} });
    CHECK(table[skills[2]].is_valid());
    CHECK(table.state().active_player == givm::player_id{ 1 });
    CHECK(target.view_in<givm::execution_state::action_selection>().skill_count() == 0);
    CHECK(log.initial_cost_queries == 1);
    CHECK(log.initial_skill_indices.size() == 4);
}

TEST_CASE("skill targets validate incrementally and submission ignores targets beyond two", "[use_skill][targets][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool fast = GENERATE(false, true);
    skill_log log;
    log.fast = fast;
    const active_skill_source active{ &log, 0, 0 };
    const passive_skill_source passive{ &log };
    const untargeted_skill_source untargeted{ &log };
    const skill_character_source owner{ &log };
    const givm::test::initialized_character_source plain;
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        mode, setup(), std::tuple{}, active, passive, untargeted, owner, plain);
    givm::table table;
    load_deck(table, library,
        { .characters = { ids.get_id<givm::character_view>(owner.name()) } },
        { .characters = { ids.get_id<givm::character_view>(plain.name()) } });
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    const auto action = target.view_in<givm::execution_state::action_selection>();
    const auto skill = action.skill_id(0);
    const givm::skill_target_id opponent = givm::character_id{ givm::player_id{ 1 }, 0 };
    const givm::skill_target_id own = givm::character_id{ givm::player_id{ 0 }, 0 };
    const std::array<givm::skill_target_id, 3> targets{ opponent, opponent, own };
    const auto calls = random.calls;
    CHECK(action.skill_targets_validate(library, table, 0, targets) == givm::target_validation::valid_complete);
    CHECK(action.skill_targets_validate(library, table, 0) == givm::target_validation::valid_complete_or_continue);
    CHECK(action.skill_targets_validate(library, table, 0, std::span{ targets }.first(1))
        == givm::target_validation::valid_incomplete);
    const std::array invalid{ own };
    CHECK(action.skill_targets_validate(library, table, 0, invalid) == givm::target_validation::invalid);
    CHECK(log.target_counts == std::vector<std::size_t>{ 2, 0, 1, 1 });
    CHECK(log.validated_targets == std::vector<skill_targets>{ { opponent, opponent }, {}, { opponent, {} }, { own, {} } });
    CHECK(random.calls == calls);
    CHECK(log.cost_queries == 0);
    action.use_skill(library, table, 0, {}, targets);
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    CHECK(log.effects == std::vector{ skill });
    CHECK(log.effect_targets == std::vector{ skill_targets{ opponent, opponent } });
    CHECK(log.used_targets == log.effect_targets);
    CHECK(log.target_counts.size() == 4);
    CHECK(log.cost_queries == 1);
    CHECK(log.initial_cost_queries == 1);
    CHECK(table.state().active_player == (fast ? givm::player_id{ 0 } : givm::player_id{ 1 }));
    const auto next_action = target.view_in<givm::execution_state::action_selection>();
    CHECK(next_action.skill_count() == (fast ? 2 : 0));
    if(fast)
    {
        CHECK(next_action.skill_id(0) == skill);
        next_action.calculate_skill_cost(library, table, 0);
        CHECK(log.cost_queries == 2);
        CHECK(log.initial_cost_queries == 1);
    }
}

TEST_CASE("skill payment validates dice before energy and pays energy without a separate selection", "[use_skill][payment][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const std::uint8_t dice = GENERATE(std::uint8_t{ 0 }, std::uint8_t{ 1 });
    skill_log log;
    const active_skill_source active{ &log, dice, 2 };
    const passive_skill_source passive{ &log };
    const untargeted_skill_source untargeted{ &log };
    const skill_character_source owner{ &log };
    const givm::test::initialized_character_source plain;
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        mode, setup(), std::tuple{}, active, passive, untargeted, owner, plain);
    givm::table table;
    load_deck(table, library,
        { .characters = { ids.get_id<givm::character_view>(owner.name()) } },
        { .characters = { ids.get_id<givm::character_view>(plain.name()) } });
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    const auto action = target.view_in<givm::execution_state::action_selection>();
    const auto calls = random.calls;
    log.extra_energy = 2;
    CHECK(action.calculate_skill_cost(library, table, 0).requirement.energy == 4);
    CHECK(action.skill_payment_validate(table, 0, pay(dice + 1)) == givm::skill_payment_validation::requirement_mismatch);
    if(dice != 0)
        CHECK(action.skill_payment_validate(table, 0, pay(dice, givm::elemental_dice::dendro))
            == givm::skill_payment_validation::insufficient_dice);
    CHECK(action.skill_payment_validate(table, 0, pay(dice)) == givm::skill_payment_validation::insufficient_energy);
    log.extra_energy = 0;
    CHECK(action.calculate_skill_cost(library, table, 0).requirement.energy == 2);
    CHECK(action.calculate_skill_cost(library, table, 0).requirement.energy == 2);
    CHECK(action.skill_payment_validate(table, 0, pay(dice)) == givm::skill_payment_validation::valid);
    CHECK(random.calls == calls);
    CHECK(resources(table) == std::array<std::uint32_t, 2>{ 4, 3 });
    action.use_skill(0, pay(dice));
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    CHECK(resources(table) == std::array<std::uint32_t, 2>{ 4u - dice, 1 });
    CHECK(log.events == (dice == 0 ? std::vector<std::string>{ "energy", "will", "effect", "used" }
        : std::vector<std::string>{ "dice", "energy", "will", "effect", "used" }));
    REQUIRE(log.resources_at_broadcast.size() == (dice == 0 ? 1 : 2));
    for(const auto snapshot : log.resources_at_broadcast)
        CHECK(snapshot == std::array<std::uint32_t, 2>{ 4u - dice, 1 });
    CHECK(log.cost_queries == 3);
    CHECK(log.target_counts.empty());
}

TEST_CASE("skill onpay and effect broadcasts resume after nested input and preserve cancellation", "[use_skill][onpay][broadcast][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool cancelled = GENERATE(false, true);
    skill_log log{ .nested = true, .cancelled = cancelled };
    const active_skill_source active{ &log };
    const passive_skill_source passive{ &log };
    const untargeted_skill_source untargeted{ &log };
    const skill_character_source owner{ &log };
    const givm::test::initialized_character_source plain;
    const givm::test::named_definition_source<givm::card_definition> filler{ "SkillDrawFiller" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        mode, setup(), std::tuple{}, active, passive, untargeted, owner, plain, filler);
    const auto card = ids.get_id<givm::card_definition>(filler.name());
    givm::table table;
    load_deck(table, library, {
        .cards = { card, card, card, card, card, card }, .characters = { ids.get_id<givm::character_view>(owner.name()) }
    }, { .characters = { ids.get_id<givm::character_view>(plain.name()) } });
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    log.record = true;
    const auto action = target.view_in<givm::execution_state::action_selection>();
    const auto skill = action.skill_id(0);
    action.use_skill(library, table, 0, pay(1));
    REQUIRE(advance(target, library, table, random) == givm::execution_state::card_selection);
    CHECK(log.events == std::vector<std::string>{ "draw" });
    CHECK(resources(table) == std::array<std::uint32_t, 2>{ 4, 3 });
    const auto saved_log = log;
    auto copied_target = target;
    auto copied_table = table;
    const auto finish = [&](givm::executor& execution, givm::table& current_table)
    {
        log = saved_log;
        const auto draws = cancelled ? 5u : 6u;
        for(std::uint32_t index = 0; index < draws; ++index)
        {
            execution.view_in<givm::execution_state::card_selection>().select({});
            REQUIRE(advance(execution, library, current_table, random)
                == (index + 1 == draws ? givm::execution_state::action_selection : givm::execution_state::card_selection));
        }
        CHECK(log.events == (cancelled
            ? std::vector<std::string>{ "draw", "dice", "draw", "energy", "draw", "will", "draw", "used", "draw" }
            : std::vector<std::string>{ "draw", "dice", "draw", "energy", "draw", "will", "draw", "effect", "draw", "used", "draw" }));
        CHECK(log.resources_at_draw == (cancelled
            ? std::vector<std::array<std::uint32_t, 2>>{ { 4, 3 }, { 3, 1 }, { 3, 1 }, { 3, 1 }, { 3, 1 } }
            : std::vector<std::array<std::uint32_t, 2>>{ { 4, 3 }, { 3, 1 }, { 3, 1 }, { 3, 1 }, { 3, 1 }, { 3, 1 } }));
        CHECK(log.effects == (cancelled ? std::vector<givm::skill_id>{} : std::vector{ skill }));
        CHECK(log.used_targets == std::vector{ skill_targets{} });
        CHECK(log.cost_queries == 1);
        CHECK(current_table[skill].is_valid());
        CHECK(resources(current_table) == std::array<std::uint32_t, 2>{ 3, 1 });
        CHECK(current_table[givm::player_id{ 0 }].hand_card_count() == draws);
        CHECK(current_table[givm::player_id{ 0 }].deck_card_count() == 6 - draws);
        CHECK(current_table.state().active_player == givm::player_id{ 1 });
    };
    finish(target, table);
    finish(copied_target, copied_table);
}

TEST_CASE("cards and switches share energy requirements and charge the outgoing active character", "[action][payment][energy][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool switching = GENERATE(false, true);
    const std::uint8_t dice = GENERATE(std::uint8_t{ 0 }, std::uint8_t{ 1 });
    skill_log log{ .extra_energy = 2, .extra_dice = dice };
    const active_skill_source active{ &log };
    const passive_skill_source passive{ &log };
    const untargeted_skill_source untargeted{ &log };
    const skill_character_source owner{ &log };
    const givm::test::initialized_character_source plain;
    const energy_card_source card{ &log, dice, 2 };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        mode, setup(1), std::tuple{}, active, passive, untargeted, owner, plain, card);
    givm::table table;
    load_deck(table, library, {
        .cards = { ids.get_id<givm::card_definition>(card.name()) },
        .characters = { ids.get_id<givm::character_view>(owner.name()), ids.get_id<givm::character_view>(plain.name()) }
    }, { .characters = { ids.get_id<givm::character_view>(plain.name()) } });
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    const auto action = target.view_in<givm::execution_state::action_selection>();
    if(switching)
    {
        log.extra_energy = 4;
        CHECK(action.calculate_switch_cost(library, table, 0).requirement.energy == 4);
        CHECK(action.switch_payment_validate(table, 0, pay(dice)) == givm::switch_payment_validation::insufficient_energy);
        log.extra_energy = 2;
        CHECK(action.calculate_switch_cost(library, table, 0).requirement.energy == 2);
        CHECK(action.switch_payment_validate(table, 0, pay(dice)) == givm::switch_payment_validation::valid);
        action.switch_active_character(0, pay(dice));
    }
    else
    {
        CHECK(action.calculate_card_cost(library, table, 0).requirement.energy == 2);
        CHECK(action.card_payment_validate(table, 0, pay(dice)) == givm::card_payment_validation::valid);
        action.play_card(0, pay(dice));
    }
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    CHECK(table[givm::character_id{ givm::player_id{ 0 }, 0 }].state().energy == 1);
    CHECK(table[givm::character_id{ givm::player_id{ 0 }, 1 }].state().energy == 0);
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 4u - dice);
    CHECK(log.events == (dice == 0 ? std::vector<std::string>{ "energy" } : std::vector<std::string>{ "dice", "energy" }));
    for(const auto snapshot : log.resources_at_broadcast)
        CHECK(snapshot == std::array<std::uint32_t, 2>{ 4u - dice, 1 });
    CHECK(log.card_effects == (switching ? 0 : 1));
    CHECK(table[givm::player_id{ 0 }].state().active_character
        == givm::character_id{ givm::player_id{ 0 }, switching ? 1uz : 0uz });
}

TEST_CASE("entering a character loads its indexed initial skills without deck initialization", "[use_skill][enter_character][query]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    skill_log log;
    const active_skill_source active{ &log };
    const passive_skill_source passive{ &log };
    const untargeted_skill_source untargeted{ &log };
    const skill_character_source owner{ &log };
    const enter_skill_character_source enter;
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::test_command{}, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, active, passive, untargeted, owner, enter);
    givm::table table;
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(enter.name()) } }, {});
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    REQUIRE(target.step(library, table, random) == givm::execution_state::finished);
    const auto character = *table[givm::player_id{ 1 }].characters().begin();
    CHECK(character.definition_id() == ids.get_id<givm::character_view>(owner.name()));
    CHECK(character.state().health == 10);
    CHECK(character.state().energy == 3);
    std::vector<givm::definition_id<givm::skill_view>> skills;
    for(const auto skill : character.skills())
    {
        skills.push_back(skill.definition_id());
        CHECK(skill.character().id() == character.id());
        CHECK(skill.state().count == 0);
    }
    CHECK(skills == std::vector{ ids.get_id<givm::skill_view>(active.name()),
        ids.get_id<givm::skill_view>(passive.name()), ids.get_id<givm::skill_view>(untargeted.name()) });
    CHECK(log.initial_skill_indices == std::vector<std::size_t>{ 0, 1, 2, 3 });
    CHECK(random.calls == 0);
}

TEST_CASE("card energy validation follows dice requirement and ownership checks", "[play_card][payment][energy]")
{
    skill_log log;
    const givm::test::initialized_character_source character;
    const energy_card_source card{ &log, 1, 1 };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        givm::compile_mode::normal, setup(1), std::tuple{}, character, card);
    givm::table table;
    const auto character_id = ids.get_id<givm::character_view>(character.name());
    load_deck(table, library, {
        .cards = { ids.get_id<givm::card_definition>(card.name()) }, .characters = { character_id }
    }, { .characters = { character_id } });
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    const auto action = target.view_in<givm::execution_state::action_selection>();
    CHECK(action.calculate_card_cost(library, table, 0).requirement.energy == 1);
    CHECK(action.card_payment_validate(table, 0, {}) == givm::card_payment_validation::requirement_mismatch);
    CHECK(action.card_payment_validate(table, 0, pay(1, givm::elemental_dice::dendro))
        == givm::card_payment_validation::insufficient_dice);
    CHECK(action.card_payment_validate(table, 0, pay(1)) == givm::card_payment_validation::insufficient_energy);
    CHECK(log.card_effects == 0);
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 1);
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 4);
}
