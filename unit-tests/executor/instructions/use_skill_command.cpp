#include <array>
#include <cstdint>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../../table/test_definition_library.hpp"

namespace
{
    constexpr givm::player_id owner{ 0 };
    constexpr givm::player_id opponent{ 1 };
    constexpr givm::character_id actor{ owner, 0 };
    constexpr givm::character_id enemy{ opponent, 0 };

    struct command_log
    {
        std::vector<std::string> events;
        givm::skill_id skill{};
        givm::skill_flags flags{};
        std::array<givm::skill_target_id, 2> targets{};
        givm::action_speed card_speed = givm::action_speed::fast;
        bool record = false;
        bool cancelled = false;
        bool empty_effect = false;
        std::uint32_t cost_broadcasts = 0;
        std::uint32_t payments = 0;
    };

    struct command_skill_source
    {
        using definition_category = givm::skill_view;
        struct definition_type { command_log* log; givm::program_entry effect; };
        command_log* log;
        std::string_view name() const { return "CommandSkill"; }
        auto tags() const { return std::array{ std::string_view{ "normal_attack" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_program(std::tuple{
                givm::draw_cards{ .count = 1 }, givm::replace_cards{ owner }
            }) };
        }
        static givm::action_cost_requirement query(const definition_type&, const givm::skill_initial_cost&)
        {
            return { .dice_requirement = { .any = 8 }, .speed = givm::action_speed::combat, .energy = 3 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view& self,
            givm::skill_effect& event, givm::handle_context& context)
        {
            CHECK(self.id() == data.log->skill);
            CHECK(event.skill == data.log->skill);
            CHECK(event.flags.value() == data.log->flags.value());
            CHECK(event.targets == data.log->targets);
            data.log->events.push_back("effect");
            return data.log->empty_effect ? givm::program_entry{} : context.invoke(data.effect);
        }
    };

    struct command_observer_source
    {
        using definition_category = givm::skill_view;
        struct definition_type { command_log* log; givm::program_entry before; givm::program_entry after; };
        command_log* log;
        std::string_view name() const { return "CommandObserver"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_program(std::tuple{ givm::replace_cards{ owner } }),
                context.add_program(std::tuple{ givm::draw_cards{ .count = 1 }, givm::replace_cards{ owner } }) };
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::skill_will_be_used& event, givm::handle_context& context)
        {
            CHECK(event.skill == data.log->skill);
            CHECK(event.flags.value() == data.log->flags.value());
            CHECK(event.targets == data.log->targets);
            CHECK(event.speed == givm::action_speed::fast);
            event.speed = data.log->card_speed == givm::action_speed::fast
                ? givm::action_speed::combat : givm::action_speed::fast;
            event.effect_cancelled = data.log->cancelled;
            data.log->events.push_back("will");
            return context.invoke(data.before);
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::skill_used& event, givm::handle_context& context)
        {
            CHECK(event.skill == data.log->skill);
            CHECK(event.flags.value() == data.log->flags.value());
            CHECK(event.targets == data.log->targets);
            CHECK(event.effect_cancelled == data.log->cancelled);
            CHECK(event.speed != data.log->card_speed);
            data.log->events.push_back("used");
            return context.invoke(data.after);
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::cost_of_skill&, givm::handle_context&)
        {
            ++data.log->cost_broadcasts;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::dice_removed&, givm::handle_context&)
        {
            ++data.log->payments;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::card_drawn& event, givm::handle_context&)
        {
            if(data.log->record)
                data.log->events.push_back(event.card.player_id == owner ? "draw:owner" : "draw:opponent");
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::card_played& event, givm::handle_context&)
        {
            CHECK(event.speed == data.log->card_speed);
            data.log->events.push_back("played");
            return {};
        }
    };

    struct command_character_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            givm::definition_id<givm::skill_view> active;
            givm::definition_id<givm::skill_view> observer;
        };
        std::string_view source_name;
        bool observes;
        bool active = true;
        std::string_view name() const { return source_name; }
        auto skill_dependencies() const
        {
            return std::array{ std::string_view{ "CommandSkill" }, std::string_view{ "CommandObserver" } };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { active ? context.resolve_id<givm::skill_view>("CommandSkill") : givm::definition_id<givm::skill_view>{},
                observes ? context.resolve_id<givm::skill_view>("CommandObserver") : givm::definition_id<givm::skill_view>{} };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .max_energy = 3, .health = 10, .energy = 3 };
        }
        static givm::definition_id<givm::skill_view> query(const definition_type& data, const givm::character_initial_skill& query)
        {
            if(query.skill_index == 0) return data.active;
            if(query.skill_index == 1) return data.observer;
            return {};
        }
    };

    struct command_card_source
    {
        using definition_category = givm::card_definition;
        struct definition_type { command_log* log; givm::program_entry effect; bool dynamic; };
        command_log* log;
        bool dynamic;
        std::string_view name() const { return "CommandTalent"; }
        auto skill_dependencies() const { return std::array{ std::string_view{ "CommandSkill" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto command = dynamic ? givm::use_skill{} : givm::use_skill{
                .player = givm::relative_player::opponent,
                .definition = context.resolve_id<givm::skill_view>("CommandSkill")
            };
            return { log, context.add_program(std::tuple{ command, givm::draw_cards{ .count = 1 } }), dynamic };
        }
        static givm::card_state query(const definition_type& data, const givm::card_initial_state&)
        {
            return { .cost = { .dice_requirement = { .any = 1 }, .speed = data.log->card_speed } };
        }
        static givm::program_entry handle(const definition_type& data, const givm::hand_card_view&,
            givm::card_effect& event, givm::handle_context& context)
        {
            data.log->events.push_back("card-effect");
            if(data.dynamic)
                return context.invoke(data.effect, givm::use_skill_input{
                    .skill = data.log->skill, .flags = data.log->flags, .targets = event.targets
                });
            return context.invoke(data.effect);
        }
    };

    struct zero_random { std::uint32_t operator()() noexcept { return 0; } };

    inline givm::execution_state advance(givm::executor& execution, const givm::definition_library& library,
        givm::table& table, zero_random& random)
    {
        auto state = execution.step(library, table, random);
        while(state == givm::execution_state::active_character_changed || state == givm::execution_state::action_started)
            state = execution.step(library, table, random);
        return state;
    }
}

TEST_CASE("use_skill commands finish all skill responses before the card notification without paying for the skill",
    "[use_skill][command][play_card][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool dynamic = GENERATE(false, true);
    const auto speed = GENERATE(givm::action_speed::fast, givm::action_speed::combat);
    const auto effect = GENERATE(0, 1, 2);
    command_log log{ .card_speed = speed, .cancelled = effect == 2, .empty_effect = effect == 1 };
    const command_skill_source skill{ &log };
    const command_observer_source observer{ &log };
    const command_character_source own_character{ "CommandOwner", false };
    const command_character_source other_character{ "CommandOpponent", true };
    const command_card_source card{ &log, dynamic };
    const givm::test::named_definition_source<givm::card_definition> filler{ "CommandFiller" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode, std::tuple{
        givm::draw_cards{ .count = 1 },
        givm::start_dice_roll_phase{ .count = 4, .reroll_count = { 0, 0 } },
        givm::begin_action{}
    }, std::tuple{}, skill, observer, own_character, other_character, card, filler);
    const auto filler_id = ids.get_id<givm::card_definition>(filler.name());
    givm::table table{ { .self_player = owner }, { .active_character = actor, .can_plunge = true },
        { .active_character = enemy, .can_plunge = true } };
    load_deck(table, library,
        { .cards = { filler_id, filler_id, ids.get_id<givm::card_definition>(card.name()) },
            .characters = { ids.get_id<givm::character_view>(own_character.name()) } },
        { .cards = { filler_id, filler_id },
            .characters = { ids.get_id<givm::character_view>(other_character.name()) } });
    log.skill = (*table[dynamic ? actor : enemy].skills().begin()).id();
    log.flags = dynamic ? givm::skill_flag_bits::elemental_burst | givm::skill_flag_bits::charged_attack
        | givm::skill_flag_bits::plunging_attack : givm::skill_flags{ givm::skill_flag_bits::normal_attack };
    log.targets = dynamic ? std::array<givm::skill_target_id, 2>{ enemy, actor }
        : std::array<givm::skill_target_id, 2>{};
    givm::executor execution;
    execution.enter_entry(library);
    zero_random random;
    REQUIRE(advance(execution, library, table, random) == givm::execution_state::action_selection);
    const auto action = execution.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.card_count() == 1);
    const auto played = action.card_id(0);
    givm::dice_counts payment;
    payment[givm::elemental_dice::omni] = 1;
    log.record = true;
    action.play_card(library, table, 0, payment, log.targets);
    REQUIRE(advance(execution, library, table, random) == givm::execution_state::card_selection);
    CHECK(log.events == std::vector<std::string>{ "card-effect", "will" });
    CHECK_FALSE(table[played].is_valid());
    const auto paused_log = log;
    auto copied_execution = execution;
    auto copied_table = table;
    const auto resume = [&](givm::executor& current, givm::table& current_table)
    {
        log = paused_log;
        const auto pauses = effect == 0 ? 3 : 2;
        for(int pause = 0; pause < pauses; ++pause)
        {
            const auto selection = current.view_in<givm::execution_state::card_selection>();
            CHECK(selection.player() == owner);
            selection.select({});
            REQUIRE(advance(current, library, current_table, random)
                == (pause + 1 == pauses ? givm::execution_state::action_selection : givm::execution_state::card_selection));
        }
        std::vector<std::string> expected{ "card-effect", "will" };
        if(effect != 2) expected.push_back("effect");
        if(effect == 0) expected.push_back(dynamic ? "draw:owner" : "draw:opponent");
        expected.insert(expected.end(), { "used", "draw:opponent", "draw:owner", "played" });
        CHECK(log.events == expected);
        CHECK(log.cost_broadcasts == 0);
        CHECK(log.payments == 1);
        CHECK(current_table[owner].state().dice.total() == 3);
        CHECK(current_table[opponent].state().dice.total() == 4);
        CHECK(current_table[actor].state().energy == 3);
        CHECK(current_table[enemy].state().energy == 3);
        CHECK(current_table[opponent].state().can_plunge);
        CHECK(current_table[owner].state().can_plunge == (speed == givm::action_speed::fast));
        CHECK(current_table.state().active_player == (speed == givm::action_speed::fast ? owner : opponent));
        CHECK(current_table.state().self_player == owner);
    };
    resume(execution, table);
    resume(copied_execution, copied_table);
}

TEST_CASE("fixed use_skill skips a missing active skill without borrowing a standby character skill",
    "[use_skill][command][play_card][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    command_log log;
    const command_skill_source skill{ &log };
    const command_observer_source observer{ &log };
    const command_character_source own_character{ "CommandOwner", false };
    const command_character_source other_character{ "CommandOpponent", false, false };
    const command_character_source standby_character{ "CommandStandby", true };
    const command_card_source card{ &log, false };
    const givm::test::named_definition_source<givm::card_definition> filler{ "CommandFiller" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode, std::tuple{
        givm::draw_cards{ .count = 1 },
        givm::start_dice_roll_phase{ .count = 4, .reroll_count = { 0, 0 } },
        givm::begin_action{}
    }, std::tuple{}, skill, observer, own_character, other_character, standby_character, card, filler);
    givm::table table{ { .self_player = owner }, { .active_character = actor }, { .active_character = enemy } };
    load_deck(table, library,
        { .cards = { ids.get_id<givm::card_definition>(filler.name()), ids.get_id<givm::card_definition>(card.name()) },
            .characters = { ids.get_id<givm::character_view>(own_character.name()) } },
        { .characters = { ids.get_id<givm::character_view>(other_character.name()),
            ids.get_id<givm::character_view>(standby_character.name()) } });
    const givm::character_id standby{ opponent, 1 };
    REQUIRE(table[enemy].skills().empty());
    REQUIRE_FALSE(table[standby].skills().empty());
    log.skill = (*table[standby].skills().begin()).id();
    log.flags = givm::skill_flag_bits::normal_attack;
    givm::executor execution;
    execution.enter_entry(library);
    zero_random random;
    REQUIRE(advance(execution, library, table, random) == givm::execution_state::action_selection);
    const auto action = execution.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.card_count() == 1);
    givm::dice_counts payment;
    payment[givm::elemental_dice::omni] = 1;
    log.record = true;
    action.play_card(library, table, 0, payment);
    REQUIRE(advance(execution, library, table, random) == givm::execution_state::action_selection);
    CHECK(log.events == std::vector<std::string>{ "card-effect", "draw:owner", "played" });
    CHECK(log.cost_broadcasts == 0);
    CHECK(log.payments == 1);
    CHECK(table[owner].hand_card_count() == 1);
    CHECK(table[owner].state().dice.total() == 3);
    CHECK(table[enemy].state().energy == 3);
    CHECK(table[standby].state().energy == 3);
    CHECK(table.state().active_player == owner);
}
