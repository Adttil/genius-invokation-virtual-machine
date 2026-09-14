#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../../table/test_definition_library.hpp"
#include "../test_character_source.hpp"

using namespace givm;

namespace
{
    struct action_log
    {
        std::vector<player_id> opportunities;
        std::vector<player_id> declarations;
        std::vector<player_id> endings;
        std::vector<character_id> switches;
        std::uint32_t previews = 0;
    };

    struct action_source
    {
        using definition_category = character_view;
        struct definition_type
        {
            action_log* log;
            action_speed speed;
            program_entry<onpay_context<cost_of_switch>> payment;
        };
        action_log* log;
        action_speed speed = action_speed::combat;
        bool terminal_payment = false;

        std::string_view name() const noexcept { return "ActionObserver"; }
        definition_type compile(definition_compile_context& context) const
        {
            return { log, speed, terminal_payment
                ? context.add_program<onpay_context<cost_of_switch>>(std::tuple{
                    end_game{ .result = game_result::player_1_win }
                }) : program_entry<onpay_context<cost_of_switch>>::null() };
        }
        static handler_program_entry_t<character_initialization> handle(
            const definition_type&, const character_view&, character_initialization& event,
            const card_table&, random_fn&)
        {
            event.state = { .max_health = 10, .health = 10 };
            return handler_program_entry_t<character_initialization>::null();
        }
        static program_entry<before_action> handle(
            const definition_type& data, const character_view&, before_action&,
            const card_table& table, random_fn&)
        {
            data.log->opportunities.push_back(table.state().active_player);
            return program_entry<before_action>::null();
        }
        static program_entry<onpay_context<cost_of_switch>> handle(
            const definition_type& data, const character_view&, cost_of_switch& event,
            const card_table&, random_fn&)
        {
            ++data.log->previews;
            event.requirement.speed = data.speed;
            return data.payment;
        }
        static program_entry<active_character_changed> handle(
            const definition_type& data, const character_view&, active_character_changed& event,
            const card_table& table, random_fn&)
        {
            CHECK(table[event.current.player_id].state().active_character == event.current);
            data.log->switches.push_back(event.current);
            return program_entry<active_character_changed>::null();
        }
        static program_entry<round_end_declared> handle(
            const definition_type& data, const character_view&, round_end_declared&,
            const card_table& table, random_fn&)
        {
            data.log->declarations.push_back(table.state().active_player);
            return program_entry<round_end_declared>::null();
        }
        static program_entry<round_ended> handle(
            const definition_type& data, const character_view&, round_ended&,
            const card_table& table, random_fn&)
        {
            data.log->endings.push_back(table.state().active_player);
            return program_entry<round_ended>::null();
        }
    };
    struct zero_random { std::uint32_t operator()() const noexcept { return 0; } };

    auto action_setup()
    {
        return std::tuple{
            initialize_characters{ player_id{ 0 } }, initialize_characters{ player_id{ 1 } },
            set_active_character{ character_id{ player_id{ 0 }, 0 } },
            set_active_character{ character_id{ player_id{ 1 }, 0 } },
            start_dice_roll_phase{ .count = 4, .reroll_count = { 0, 0 } }
        };
    }

    void reach_action_start(executor& target, const definition_library& library, card_table& table, zero_random& random)
    {
        REQUIRE(target.step(library, table, random) == execution_state::active_character_changed);
        REQUIRE(target.step(library, table, random) == execution_state::active_character_changed);
        REQUIRE(target.step(library, table, random) == execution_state::action_started);
    }
}

TEST_CASE("action and round observations precede their handlers and ended players are skipped", "[begin_action][execution-view]")
{
    const auto speed = GENERATE(action_speed::combat, action_speed::fast);
    action_log log;
    const action_source observer{ &log, speed };
    const test::initialized_character_source character;
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple_cat(action_setup(), std::tuple{
            begin_action{}, end_round{}, end_game{ .result = game_result::both_loss }
        }), std::tuple{}, observer, character
    );
    card_table table;
    const auto plain = ids.get_id<character_view>(character.name());
    table.load_deck(player_id{ 0 }, { .characters = { ids.get_id<character_view>(observer.name()), plain } });
    table.load_deck(player_id{ 1 }, { .characters = { plain, plain } });
    executor target;
    target.enter_entry(library);
    zero_random random;
    reach_action_start(target, library, table, random);
    CHECK(log.opportunities.empty());
    REQUIRE(target.step(library, table, random) == execution_state::action);
    CHECK(log.opportunities == std::vector{ player_id{ 0 } });
    log.switches.clear();

    const auto switch_active = [&](player_id next_player)
    {
        const auto player = table.state().active_player;
        const auto next = target.view_in<execution_state::action>().costs()[0].target;
        const auto previous = table[player].state().active_character;
        const auto dice_before = table[player].state().dice.total();
        const auto switch_count = log.switches.size();
        const auto before_count = log.opportunities.size();
        dice_counts paid;
        paid[elemental_dice::omni] = 1;
        target.view_in<execution_state::action>().execute_action(0, { .paid_dice = paid });
        REQUIRE(target.step(library, table, random) == execution_state::active_character_changed);
        CHECK(target.view_in<execution_state::active_character_changed>().character() == next);
        CHECK(table[player].state().active_character == previous);
        CHECK(table[player].state().dice.total() == dice_before - 1);
        CHECK(log.switches.size() == switch_count);
        if(speed == action_speed::combat)
        {
            REQUIRE(target.step(library, table, random) == execution_state::action_started);
            CHECK(table.state().active_player == next_player);
            CHECK(log.opportunities.size() == before_count);
        }
        REQUIRE(target.step(library, table, random) == execution_state::action);
        CHECK(table[player].state().active_character == next);
        CHECK(table.state().active_player == next_player);
        REQUIRE(log.switches.size() == switch_count + 1);
        CHECK(log.switches.back() == next);
        REQUIRE(log.opportunities.size() == before_count + 1);
        CHECK(log.opportunities.back() == next_player);
    };
    const auto first_ended = speed == action_speed::combat ? player_id{ 1 } : player_id{ 0 };
    switch_active(first_ended);
    target.view_in<execution_state::action>().declare_round_end();
    REQUIRE(target.step(library, table, random) == execution_state::round_end_declared);
    CHECK(table.state().first_ended);
    CHECK(log.declarations.empty());
    REQUIRE(target.step(library, table, random) == execution_state::action_started);
    CHECK(log.declarations == std::vector{ first_ended });
    const auto continuing = other_player(first_ended);
    CHECK(table.state().active_player == continuing);
    REQUIRE(target.step(library, table, random) == execution_state::action);
    switch_active(continuing);
    switch_active(continuing);
    target.view_in<execution_state::action>().declare_round_end();
    REQUIRE(target.step(library, table, random) == execution_state::round_end_declared);
    CHECK(log.declarations == std::vector{ first_ended });
    REQUIRE(target.step(library, table, random) == execution_state::round_ending);
    CHECK(log.declarations == std::vector{ first_ended, continuing });
    CHECK(log.endings.empty());
    REQUIRE(target.step(library, table, random) == execution_state::finished);
    CHECK(log.endings == std::vector{ first_ended });
    CHECK(table.state().active_player == first_ended);
    CHECK_FALSE(table.state().first_ended);
}

TEST_CASE("cost previews wait for confirmation before executing a terminal payment response", "[begin_action][onpay]")
{
    const bool observed = GENERATE(false, true);
    action_log log;
    const action_source observer{ &log, action_speed::combat, true };
    const test::initialized_character_source character;
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple_cat(action_setup(), std::tuple{ begin_action{} }), std::tuple{}, observer, character
    );
    card_table table;
    const auto plain = ids.get_id<character_view>(character.name());
    table.load_deck(player_id{ 0 }, { .characters = { ids.get_id<character_view>(observer.name()), plain } });
    table.load_deck(player_id{ 1 }, { .characters = { plain } });
    executor target;
    target.enter_entry(library);
    zero_random random;
    if(observed) reach_action_start(target, library, table, random);
    REQUIRE((observed ? target.step(library, table, random) : target.run(library, table, random)) == execution_state::action);
    CHECK(log.previews == 0);
    for(std::uint32_t count = 1; count <= 2; ++count)
    {
        target.view_in<execution_state::action>().request_cost(0);
        REQUIRE((observed ? target.step(library, table, random) : target.run(library, table, random)) == execution_state::action);
        CHECK(log.previews == count);
        const auto costs = target.view_in<execution_state::action>().costs();
        REQUIRE(costs.size() == 1);
        CHECK(costs[0].target == character_id{ player_id{ 0 }, 1 });
        CHECK(costs[0].requirement.dice_requirement.any == 1);
        CHECK(table[player_id{ 0 }].state().active_character == character_id{ player_id{ 0 }, 0 });
        CHECK(table[player_id{ 0 }].state().dice.total() == 4);
    }
    dice_counts paid;
    paid[elemental_dice::omni] = 1;
    target.view_in<execution_state::action>().execute_action_with_cost(0, { .paid_dice = paid });
    REQUIRE((observed ? target.step(library, table, random) : target.run(library, table, random)) == execution_state::finished);
    CHECK(target.view_in<execution_state::finished>().result() == game_result::player_1_win);
    CHECK(log.previews == 2);
    CHECK(table[player_id{ 0 }].state().active_character == character_id{ player_id{ 0 }, 0 });
    CHECK(table[player_id{ 0 }].state().dice.total() == 4);
}

TEST_CASE("switch choices include only living standby characters", "[begin_action]")
{
    const bool living_standby = GENERATE(false, true);
    const test::initialized_character_source living;
    const test::initialized_character_source defeated{ "Defeated", { .max_health = 10, .health = 0 } };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple_cat(action_setup(), std::tuple{ begin_action{} }), std::tuple{}, living, defeated
    );
    const auto alive = ids.get_id<character_view>(living.name());
    const auto dead = ids.get_id<character_view>(defeated.name());
    card_table table;
    table.load_deck(player_id{ 0 }, { .characters = { alive, dead, living_standby ? alive : dead } });
    table.load_deck(player_id{ 1 }, { .characters = { alive } });
    executor target;
    target.enter_entry(library);
    zero_random random;
    REQUIRE(target.run(library, table, random) == execution_state::action);
    const auto choices = target.view_in<execution_state::action>().costs();
    REQUIRE(choices.size() == (living_standby ? 1 : 0));
    if(living_standby) CHECK(choices[0].target == character_id{ player_id{ 0 }, 2 });
    CHECK(table[character_id{ player_id{ 0 }, 1 }].state().health == 0);
}

TEST_CASE("a round starts before its limit check and dice reset", "[start_round][execution-view]")
{
    const bool observed = GENERATE(false, true);
    const bool exceeds_limit = GENERATE(false, true);
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{
            start_dice_roll_phase{ .count = 3, .reroll_count = { 0, 0 } },
            start_round{ .max_rounds = exceeds_limit ? 0u : 1u },
            end_game{ .result = game_result::player_0_win }
        }, std::tuple{}
    );
    card_table table;
    executor target;
    target.enter_entry(library);
    zero_random random;
    if(observed)
    {
        REQUIRE(target.step(library, table, random) == execution_state::round_started);
        CHECK(table.state().round_number == 1);
        for(const auto player : table.players()) CHECK(player.state().dice.total() == 3);
    }
    REQUIRE((observed ? target.step(library, table, random) : target.run(library, table, random)) == execution_state::finished);
    CHECK(table.state().round_number == 1);
    CHECK(target.view_in<execution_state::finished>().result() == (exceeds_limit ? game_result::both_loss : game_result::player_0_win));
    for(const auto player : table.players()) CHECK(player.state().dice.total() == (exceeds_limit ? 3 : 0));
}
