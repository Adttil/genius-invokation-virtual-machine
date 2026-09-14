#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../../table/test_definition_library.hpp"
#include "../test_character_source.hpp"

namespace
{
    struct action_log
    {
        std::vector<givm::player_id> opportunities;
        std::vector<givm::player_id> declarations;
        std::vector<givm::player_id> endings;
        std::vector<givm::character_id> switches;
        std::uint32_t previews = 0;
    };

    struct action_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            action_log* log;
            givm::action_speed speed;
            givm::program_entry<givm::onpay_context<givm::cost_of_switch>> payment;
        };
        action_log* log;
        givm::action_speed speed = givm::action_speed::combat;
        bool terminal_payment = false;

        std::string_view name() const noexcept { return "ActionObserver"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, speed, terminal_payment
                ? context.add_program<givm::onpay_context<givm::cost_of_switch>>(std::tuple{
                    givm::end_game{ .result = givm::game_result::player_1_win }
                }) : givm::program_entry<givm::onpay_context<givm::cost_of_switch>>::null() };
        }
        static givm::handler_program_entry_t<givm::character_initialization> handle(
            const definition_type&, const givm::character_view&, givm::character_initialization& event,
            const givm::table&, givm::random_fn&)
        {
            event.state = { .max_health = 10, .health = 10 };
            return givm::handler_program_entry_t<givm::character_initialization>::null();
        }
        static givm::program_entry<givm::before_action> handle(
            const definition_type& data, const givm::character_view&, givm::before_action&,
            const givm::table& table, givm::random_fn&)
        {
            data.log->opportunities.push_back(table.state().active_player);
            return givm::program_entry<givm::before_action>::null();
        }
        static givm::program_entry<givm::onpay_context<givm::cost_of_switch>> handle(
            const definition_type& data, const givm::character_view&, givm::cost_of_switch& event,
            const givm::table&, givm::random_fn&)
        {
            ++data.log->previews;
            event.requirement.speed = data.speed;
            return data.payment;
        }
        static givm::program_entry<givm::active_character_changed> handle(
            const definition_type& data, const givm::character_view&, givm::active_character_changed& event,
            const givm::table& table, givm::random_fn&)
        {
            CHECK(table[event.current.player_id].state().active_character == event.current);
            data.log->switches.push_back(event.current);
            return givm::program_entry<givm::active_character_changed>::null();
        }
        static givm::program_entry<givm::round_end_declared> handle(
            const definition_type& data, const givm::character_view&, givm::round_end_declared&,
            const givm::table& table, givm::random_fn&)
        {
            data.log->declarations.push_back(table.state().active_player);
            return givm::program_entry<givm::round_end_declared>::null();
        }
        static givm::program_entry<givm::round_ended> handle(
            const definition_type& data, const givm::character_view&, givm::round_ended&,
            const givm::table& table, givm::random_fn&)
        {
            data.log->endings.push_back(table.state().active_player);
            return givm::program_entry<givm::round_ended>::null();
        }
    };
    struct zero_random { std::uint32_t operator()() const noexcept { return 0; } };

    auto action_setup()
    {
        return std::tuple{
            givm::initialize_characters{ givm::player_id{ 0 } }, givm::initialize_characters{ givm::player_id{ 1 } },
            givm::set_active_character{ givm::character_id{ givm::player_id{ 0 }, 0 } },
            givm::set_active_character{ givm::character_id{ givm::player_id{ 1 }, 0 } },
            givm::start_dice_roll_phase{ .count = 4, .reroll_count = { 0, 0 } }
        };
    }

    void reach_action_start(givm::executor& target, const givm::definition_library& library, givm::table& table, zero_random& random)
    {
        REQUIRE(target.step(library, table, random) == givm::execution_state::active_character_changed);
        REQUIRE(target.step(library, table, random) == givm::execution_state::active_character_changed);
        REQUIRE(target.step(library, table, random) == givm::execution_state::action_started);
    }
}

TEST_CASE("action and round observations precede their handlers and ended players are skipped", "[begin_action][execution-view]")
{
    const auto speed = GENERATE(givm::action_speed::combat, givm::action_speed::fast);
    action_log log;
    const action_source observer{ &log, speed };
    const givm::test::initialized_character_source character;
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        std::tuple_cat(action_setup(), std::tuple{
            givm::begin_action{}, givm::end_round{}, givm::end_game{ .result = givm::game_result::both_loss }
        }), std::tuple{}, observer, character
    );
    givm::table table;
    const auto plain = ids.get_id<givm::character_view>(character.name());
    table.load_deck(givm::player_id{ 0 }, { .characters = { ids.get_id<givm::character_view>(observer.name()), plain } });
    table.load_deck(givm::player_id{ 1 }, { .characters = { plain, plain } });
    givm::executor target;
    target.enter_entry(library);
    zero_random random;
    reach_action_start(target, library, table, random);
    CHECK(log.opportunities.empty());
    REQUIRE(target.step(library, table, random) == givm::execution_state::action);
    CHECK(log.opportunities == std::vector{ givm::player_id{ 0 } });
    log.switches.clear();

    const auto switch_active = [&](givm::player_id next_player)
    {
        const auto player = table.state().active_player;
        const auto next = target.view_in<givm::execution_state::action>().costs()[0].target;
        const auto previous = table[player].state().active_character;
        const auto dice_before = table[player].state().dice.total();
        const auto switch_count = log.switches.size();
        const auto before_count = log.opportunities.size();
        givm::dice_counts paid;
        paid[givm::elemental_dice::omni] = 1;
        target.view_in<givm::execution_state::action>().execute_action(0, { .paid_dice = paid });
        REQUIRE(target.step(library, table, random) == givm::execution_state::active_character_changed);
        CHECK(target.view_in<givm::execution_state::active_character_changed>().character() == next);
        CHECK(table[player].state().active_character == previous);
        CHECK(table[player].state().dice.total() == dice_before - 1);
        CHECK(log.switches.size() == switch_count);
        if(speed == givm::action_speed::combat)
        {
            REQUIRE(target.step(library, table, random) == givm::execution_state::action_started);
            CHECK(table.state().active_player == next_player);
            CHECK(log.opportunities.size() == before_count);
        }
        REQUIRE(target.step(library, table, random) == givm::execution_state::action);
        CHECK(table[player].state().active_character == next);
        CHECK(table.state().active_player == next_player);
        REQUIRE(log.switches.size() == switch_count + 1);
        CHECK(log.switches.back() == next);
        REQUIRE(log.opportunities.size() == before_count + 1);
        CHECK(log.opportunities.back() == next_player);
    };
    const auto first_ended = speed == givm::action_speed::combat ? givm::player_id{ 1 } : givm::player_id{ 0 };
    switch_active(first_ended);
    target.view_in<givm::execution_state::action>().declare_round_end();
    REQUIRE(target.step(library, table, random) == givm::execution_state::round_end_declared);
    CHECK(table.state().first_ended);
    CHECK(log.declarations.empty());
    REQUIRE(target.step(library, table, random) == givm::execution_state::action_started);
    CHECK(log.declarations == std::vector{ first_ended });
    const auto continuing = other_player(first_ended);
    CHECK(table.state().active_player == continuing);
    REQUIRE(target.step(library, table, random) == givm::execution_state::action);
    switch_active(continuing);
    switch_active(continuing);
    target.view_in<givm::execution_state::action>().declare_round_end();
    REQUIRE(target.step(library, table, random) == givm::execution_state::round_end_declared);
    CHECK(log.declarations == std::vector{ first_ended });
    REQUIRE(target.step(library, table, random) == givm::execution_state::round_ending);
    CHECK(log.declarations == std::vector{ first_ended, continuing });
    CHECK(log.endings.empty());
    REQUIRE(target.step(library, table, random) == givm::execution_state::finished);
    CHECK(log.endings == std::vector{ first_ended });
    CHECK(table.state().active_player == first_ended);
    CHECK_FALSE(table.state().first_ended);
}

TEST_CASE("cost previews wait for confirmation before executing a terminal payment response", "[begin_action][onpay]")
{
    const bool observed = GENERATE(false, true);
    action_log log;
    const action_source observer{ &log, givm::action_speed::combat, true };
    const givm::test::initialized_character_source character;
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        std::tuple_cat(action_setup(), std::tuple{ givm::begin_action{} }), std::tuple{}, observer, character
    );
    givm::table table;
    const auto plain = ids.get_id<givm::character_view>(character.name());
    table.load_deck(givm::player_id{ 0 }, { .characters = { ids.get_id<givm::character_view>(observer.name()), plain } });
    table.load_deck(givm::player_id{ 1 }, { .characters = { plain } });
    givm::executor target;
    target.enter_entry(library);
    zero_random random;
    if(observed) reach_action_start(target, library, table, random);
    REQUIRE((observed ? target.step(library, table, random) : target.run(library, table, random)) == givm::execution_state::action);
    CHECK(log.previews == 0);
    for(std::uint32_t count = 1; count <= 2; ++count)
    {
        target.view_in<givm::execution_state::action>().request_cost(0);
        REQUIRE((observed ? target.step(library, table, random) : target.run(library, table, random)) == givm::execution_state::action);
        CHECK(log.previews == count);
        const auto costs = target.view_in<givm::execution_state::action>().costs();
        REQUIRE(costs.size() == 1);
        CHECK(costs[0].target == givm::character_id{ givm::player_id{ 0 }, 1 });
        CHECK(costs[0].requirement.dice_requirement.any == 1);
        CHECK(table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 0 });
        CHECK(table[givm::player_id{ 0 }].state().dice.total() == 4);
    }
    givm::dice_counts paid;
    paid[givm::elemental_dice::omni] = 1;
    target.view_in<givm::execution_state::action>().execute_action_with_cost(0, { .paid_dice = paid });
    REQUIRE((observed ? target.step(library, table, random) : target.run(library, table, random)) == givm::execution_state::finished);
    CHECK(target.view_in<givm::execution_state::finished>().result() == givm::game_result::player_1_win);
    CHECK(log.previews == 2);
    CHECK(table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 0 });
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 4);
}

TEST_CASE("switch choices include only living standby characters", "[begin_action]")
{
    const bool living_standby = GENERATE(false, true);
    const givm::test::initialized_character_source living;
    const givm::test::initialized_character_source defeated{ "Defeated", { .max_health = 10, .health = 0 } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        std::tuple_cat(action_setup(), std::tuple{ givm::begin_action{} }), std::tuple{}, living, defeated
    );
    const auto alive = ids.get_id<givm::character_view>(living.name());
    const auto dead = ids.get_id<givm::character_view>(defeated.name());
    givm::table table;
    table.load_deck(givm::player_id{ 0 }, { .characters = { alive, dead, living_standby ? alive : dead } });
    table.load_deck(givm::player_id{ 1 }, { .characters = { alive } });
    givm::executor target;
    target.enter_entry(library);
    zero_random random;
    REQUIRE(target.run(library, table, random) == givm::execution_state::action);
    const auto choices = target.view_in<givm::execution_state::action>().costs();
    REQUIRE(choices.size() == (living_standby ? 1 : 0));
    if(living_standby) CHECK(choices[0].target == givm::character_id{ givm::player_id{ 0 }, 2 });
    CHECK(table[givm::character_id{ givm::player_id{ 0 }, 1 }].state().health == 0);
}

TEST_CASE("a round starts before its limit check and dice reset", "[start_round][execution-view]")
{
    const bool observed = GENERATE(false, true);
    const bool exceeds_limit = GENERATE(false, true);
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        std::tuple{
            givm::start_dice_roll_phase{ .count = 3, .reroll_count = { 0, 0 } },
            givm::start_round{ .max_rounds = exceeds_limit ? 0u : 1u },
            givm::end_game{ .result = givm::game_result::player_0_win }
        }, std::tuple{}
    );
    givm::table table;
    givm::executor target;
    target.enter_entry(library);
    zero_random random;
    if(observed)
    {
        REQUIRE(target.step(library, table, random) == givm::execution_state::round_started);
        CHECK(table.state().round_number == 1);
        for(const auto player : table.players()) CHECK(player.state().dice.total() == 3);
    }
    REQUIRE((observed ? target.step(library, table, random) : target.run(library, table, random)) == givm::execution_state::finished);
    CHECK(table.state().round_number == 1);
    CHECK(target.view_in<givm::execution_state::finished>().result() == (exceeds_limit ? givm::game_result::both_loss : givm::game_result::player_0_win));
    for(const auto player : table.players()) CHECK(player.state().dice.total() == (exceeds_limit ? 3 : 0));
}
