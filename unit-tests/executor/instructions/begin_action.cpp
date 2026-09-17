#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../../table/test_definition_library.hpp"
#include "../test_character_source.hpp"

namespace
{
    static_assert(std::is_const_v<decltype(givm::cost_of_switch::target)>);

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
            givm::handler_program_entry_t<givm::cost_of_switch> payment;
            bool free_switch;
        };
        action_log* log;
        givm::action_speed speed = givm::action_speed::combat;
        bool terminal_payment = false;
        bool draw_payment = false;
        bool free_switch = false;

        std::string_view name() const noexcept { return "ActionObserver"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            givm::handler_program_entry_t<givm::cost_of_switch> payment;
            if(terminal_payment)
                payment = context.add_program<givm::handler_program_context_t<givm::cost_of_switch>>(std::tuple{
                    givm::end_game{ .result = givm::game_result::player_1_win }
                });
            else if(draw_payment)
                payment = context.add_program<givm::handler_program_context_t<givm::cost_of_switch>>(std::tuple{
                    givm::draw_cards{ .count = 1 }
                });
            return { log, speed, payment, free_switch };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry<givm::before_action> handle(
            const definition_type& data, const givm::character_view&, givm::before_action&,
            const givm::table& table, givm::random_fn&)
        {
            data.log->opportunities.push_back(table.state().active_player);
            return givm::program_entry<givm::before_action>::null();
        }
        static givm::handler_program_entry_t<givm::cost_of_switch> handle(
            const definition_type& data, const givm::character_view&, givm::cost_of_switch& event,
            const givm::table&, givm::random_fn&)
        {
            ++data.log->previews;
            event.requirement.speed = data.speed;
            if(data.free_switch) event.requirement.dice_requirement.any = 0;
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

    struct quote_control
    {
        std::vector<givm::character_id> quoted;
        givm::elemental_dice_requirement requirement{};
        bool replace_requirement = false;
        bool add_target_index = false;
        bool enable_payment = false;
    };

    struct quote_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            quote_control* control;
            givm::dice_counts initial_dice;
            givm::handler_program_entry_t<givm::cost_of_switch> first_payment;
            givm::handler_program_entry_t<givm::cost_of_switch> second_payment;
        };
        quote_control* control;
        std::string_view source_name = "Quote";
        givm::dice_counts initial_dice{};

        std::string_view name() const noexcept { return source_name; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return {
                control,
                initial_dice,
                context.add_program<givm::handler_program_context_t<givm::cost_of_switch>>(std::tuple{
                    givm::draw_cards{ .count = 1 }
                }),
                context.add_program<givm::handler_program_context_t<givm::cost_of_switch>>(std::tuple{
                    givm::draw_cards{ .count = 2 }
                })
            };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::handler_program_entry_t<givm::dice_roll_preparation> handle(
            const definition_type& data, const givm::character_view&, givm::dice_roll_preparation& event,
            const givm::table&, givm::random_fn&)
        {
            if(data.initial_dice.total() != 0)
            {
                event.fixed_dice[0] = data.initial_dice;
            }
            return givm::handler_program_entry_t<givm::dice_roll_preparation>::null();
        }
        static givm::handler_program_entry_t<givm::cost_of_switch> handle(
            const definition_type& data, const givm::character_view&, givm::cost_of_switch& event,
            const givm::table&, givm::random_fn&)
        {
            auto& control = *data.control;
            control.quoted.push_back(event.target);
            if(control.replace_requirement)
            {
                event.requirement.dice_requirement = control.requirement;
            }
            if(control.add_target_index)
            {
                event.requirement.dice_requirement.any += static_cast<std::uint8_t>(event.target.index);
            }
            if(not control.enable_payment)
            {
                return givm::handler_program_entry_t<givm::cost_of_switch>::null();
            }
            return event.target.index == 1 ? data.first_payment : data.second_payment;
        }
    };

    struct counting_random
    {
        std::uint32_t calls = 0;
        std::uint32_t operator()() noexcept
        {
            ++calls;
            return 0;
        }
    };

    givm::dice_counts dice(std::initializer_list<std::pair<givm::elemental_dice, std::uint8_t>> values)
    {
        givm::dice_counts result;
        for(const auto& [element, count] : values) result[element] = count;
        return result;
    }

    auto action_setup(std::uint32_t dice_count = 4, std::size_t active_index = 0)
    {
        return std::tuple{
            givm::initialize_characters{ givm::player_id{ 0 } }, givm::initialize_characters{ givm::player_id{ 1 } },
            givm::set_active_character{ givm::character_id{ givm::player_id{ 0 }, active_index } },
            givm::set_active_character{ givm::character_id{ givm::player_id{ 1 }, 0 } },
            givm::start_dice_roll_phase{ .count = dice_count, .reroll_count = { 0, 0 } }
        };
    }

    template<class TRandom>
    void reach_action_start(givm::executor& target, const givm::definition_library& library, givm::table& table, TRandom& random)
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
        givm::compile_mode::observed,
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
    REQUIRE(target.step(library, table, random) == givm::execution_state::action_selection);
    CHECK(log.opportunities == std::vector{ givm::player_id{ 0 } });
    log.switches.clear();

    const auto switch_active = [&](givm::player_id next_player)
    {
        const auto player = table.state().active_player;
        const auto next = target.view_in<givm::execution_state::action_selection>().switch_target(0);
        const auto previous = table[player].state().active_character;
        const auto dice_before = table[player].state().dice.total();
        const auto switch_count = log.switches.size();
        const auto before_count = log.opportunities.size();
        givm::dice_counts paid;
        paid[givm::elemental_dice::omni] = 1;
        target.view_in<givm::execution_state::action_selection>().switch_active_character(library, table, 0, paid);
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
        REQUIRE(target.step(library, table, random) == givm::execution_state::action_selection);
        CHECK(table[player].state().active_character == next);
        CHECK(table.state().active_player == next_player);
        REQUIRE(log.switches.size() == switch_count + 1);
        CHECK(log.switches.back() == next);
        REQUIRE(log.opportunities.size() == before_count + 1);
        CHECK(log.opportunities.back() == next_player);
    };
    const auto first_ended = speed == givm::action_speed::combat ? givm::player_id{ 1 } : givm::player_id{ 0 };
    switch_active(first_ended);
    target.view_in<givm::execution_state::action_selection>().declare_round_end();
    REQUIRE(target.step(library, table, random) == givm::execution_state::round_end_declared);
    CHECK(table.state().first_ended);
    CHECK(log.declarations.empty());
    REQUIRE(target.step(library, table, random) == givm::execution_state::action_started);
    CHECK(log.declarations == std::vector{ first_ended });
    const auto continuing = other_player(first_ended);
    CHECK(table.state().active_player == continuing);
    REQUIRE(target.step(library, table, random) == givm::execution_state::action_selection);
    switch_active(continuing);
    switch_active(continuing);
    target.view_in<givm::execution_state::action_selection>().declare_round_end();
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
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
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
    REQUIRE(target.step(library, table, random) == givm::execution_state::action_selection);
    CHECK(log.previews == 0);
    const auto opportunities = log.opportunities;
    for(std::uint32_t count = 1; count <= 2; ++count)
    {
        const auto action = target.view_in<givm::execution_state::action_selection>();
        const auto& cost = action.calculate_switch_cost(library, table, 0);
        CHECK(log.previews == count);
        CHECK(cost.target == givm::character_id{ givm::player_id{ 0 }, 1 });
        CHECK(cost.requirement.dice_requirement.any == 1);
        CHECK(action.switch_payment_validate(table, 0, dice({ { givm::elemental_dice::omni, 1 } })) == givm::switch_payment_validation::valid);
        CHECK(action.switch_payment_validate(table, 0, {}) == givm::switch_payment_validation::requirement_mismatch);
        CHECK(log.previews == count);
        CHECK(log.opportunities == opportunities);
        REQUIRE(action.switch_target_count() == 1);
        const auto& cached_cost = action.switch_cost(0);
        CHECK(cached_cost.target == givm::character_id{ givm::player_id{ 0 }, 1 });
        CHECK(cached_cost.requirement.dice_requirement.any == 1);
        CHECK(table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 0 });
        CHECK(table[givm::player_id{ 0 }].state().dice.total() == 4);
    }
    CHECK(log.previews == 2);
    CHECK(log.opportunities == opportunities);
    givm::dice_counts paid;
    paid[givm::elemental_dice::omni] = 1;
    target.view_in<givm::execution_state::action_selection>().switch_active_character(0, paid);
    REQUIRE(target.step(library, table, random) == givm::execution_state::finished);
    CHECK(target.view_in<givm::execution_state::finished>().result() == givm::game_result::player_1_win);
    CHECK(log.previews == 2);
    CHECK(table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 0 });
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 4);
}

TEST_CASE("switch choices include only living standby characters", "[begin_action]")
{
    const bool living_standby = GENERATE(false, true);
    const bool active_in_middle = GENERATE(false, true);
    const std::size_t active_index = active_in_middle ? 1 : 0;
    const givm::test::initialized_character_source living;
    const givm::test::initialized_character_source defeated{ "Defeated", { .max_health = 10, .health = 0 } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        givm::compile_mode::normal,
        std::tuple_cat(action_setup(4, active_index), std::tuple{ givm::begin_action{} }), std::tuple{}, living, defeated
    );
    const auto alive = ids.get_id<givm::character_view>(living.name());
    const auto dead = ids.get_id<givm::character_view>(defeated.name());
    givm::table table;
    table.load_deck(givm::player_id{ 0 }, {
        .characters = { active_in_middle ? dead : alive, active_in_middle ? alive : dead, living_standby ? alive : dead }
    });
    table.load_deck(givm::player_id{ 1 }, { .characters = { alive } });
    givm::executor target;
    target.enter_entry(library);
    zero_random random;
    REQUIRE(target.step(library, table, random) == givm::execution_state::action_selection);
    const auto action = target.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.switch_target_count() == (living_standby ? 1 : 0));
    CHECK(table[givm::character_id{ givm::player_id{ 0 }, 1 - active_index }].state().health == 0);
    if(living_standby)
    {
        const givm::character_id next{ givm::player_id{ 0 }, 2 };
        CHECK(action.switch_cost(0).target == next);
        CHECK(action.switch_target(0) == next);
        const auto& cost = action.calculate_switch_cost(library, table, 0);
        CHECK(cost.target == next);
        CHECK(cost.requirement.dice_requirement.any == 1);
        const auto paid = dice({ { givm::elemental_dice::omni, 1 } });
        CHECK(action.switch_payment_validate(table, 0, paid) == givm::switch_payment_validation::valid);
        action.switch_active_character(0, paid);
        REQUIRE(target.step(library, table, random) == givm::execution_state::action_selection);
        CHECK(table[givm::player_id{ 0 }].state().active_character == next);
        CHECK(table[givm::player_id{ 0 }].state().dice.total() == 3);
    }
}

TEST_CASE("a round starts before its limit check and dice reset", "[start_round][execution-view]")
{
    const bool observed = GENERATE(false, true);
    const bool exceeds_limit = GENERATE(false, true);
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
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
    REQUIRE(target.step(library, table, random) == givm::execution_state::finished);
    CHECK(table.state().round_number == 1);
    CHECK(target.view_in<givm::execution_state::finished>().result() == (exceeds_limit ? givm::game_result::both_loss : givm::game_result::player_0_win));
    for(const auto player : table.players()) CHECK(player.state().dice.total() == (exceeds_limit ? 3 : 0));
}

TEST_CASE("confirmed nonterminal payment responses return before dice payment and switching", "[begin_action][onpay][compile-mode]")
{
    const bool observed = GENERATE(false, true);
    const bool free_switch = GENERATE(false, true);
    const bool automatic_quote = GENERATE(false, true);
    action_log log;
    const action_source observer{ &log, givm::action_speed::combat, false, true, free_switch };
    const givm::test::initialized_character_source character;
    const givm::test::named_definition_source<givm::card_definition> card{ "PaymentCard" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple_cat(action_setup(), std::tuple{ givm::begin_action{} }), std::tuple{}, observer, character, card
    );
    const auto plain = ids.get_id<givm::character_view>(character.name());
    givm::table table;
    table.load_deck(givm::player_id{ 0 }, {
        .cards = { ids.get_id<givm::card_definition>(card.name()) },
        .characters = { ids.get_id<givm::character_view>(observer.name()), plain }
    });
    table.load_deck(givm::player_id{ 1 }, { .characters = { plain } });
    givm::executor target;
    target.enter_entry(library);
    zero_random random;
    if(observed) reach_action_start(target, library, table, random);
    REQUIRE(target.step(library, table, random) == givm::execution_state::action_selection);
    log.switches.clear();

    const auto action = target.view_in<givm::execution_state::action_selection>();
    const auto paid = dice({ { givm::elemental_dice::omni, static_cast<std::uint8_t>(free_switch ? 0 : 1) } });
    CHECK(log.previews == 0);
    if(automatic_quote)
    {
        action.switch_active_character(library, table, 0, paid);
    }
    else
    {
        action.calculate_switch_cost(library, table, 0);
        CHECK(action.switch_payment_validate(table, 0, paid) == givm::switch_payment_validation::valid);
        action.switch_active_character(0, paid);
    }
    CHECK(log.previews == 1);
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 0);
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 4);
    CHECK(table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 0 });

    auto state = target.step(library, table, random);
    if(observed)
    {
        REQUIRE(state == givm::execution_state::active_character_changed);
        CHECK(table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 0 });
        CHECK(table[givm::player_id{ 0 }].hand_card_count() == 1);
        CHECK(table[givm::player_id{ 0 }].state().dice.total() == (free_switch ? 4 : 3));
        REQUIRE(target.step(library, table, random) == givm::execution_state::action_started);
        state = target.step(library, table, random);
    }
    REQUIRE(state == givm::execution_state::action_selection);
    CHECK(log.previews == 1);
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 1);
    CHECK(table[givm::player_id{ 0 }].deck_card_count() == 0);
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == (free_switch ? 4 : 3));
    CHECK(table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 1 });
    CHECK(table.state().active_player == givm::player_id{ 1 });
    CHECK(log.switches == std::vector{ givm::character_id{ givm::player_id{ 0 }, 1 } });
}

TEST_CASE("synchronous quotes are independent and copied executions commit only their chosen candidate", "[begin_action][onpay][compile-mode]")
{
    const bool observed = GENERATE(false, true);
    const bool quote_both = GENERATE(false, true);
    quote_control control{ .add_target_index = true, .enable_payment = true };
    quote_control empty_control;
    const quote_source source{ &control };
    const quote_source empty_source{ &empty_control, "EmptyQuote" };
    const givm::test::initialized_character_source character;
    const givm::test::named_definition_source<givm::card_definition> card{ "QuotePaymentCard" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple_cat(action_setup(), std::tuple{ givm::begin_action{} }), std::tuple{}, source, empty_source, character, card
    );
    const auto plain = ids.get_id<givm::character_view>(character.name());
    const auto card_id = ids.get_id<givm::card_definition>(card.name());
    givm::table table;
    table.load_deck(givm::player_id{ 0 }, {
        .cards = { card_id, card_id, card_id },
        .characters = { ids.get_id<givm::character_view>(source.name()), plain, plain }
    });
    table.load_deck(givm::player_id{ 1 }, { .characters = { ids.get_id<givm::character_view>(empty_source.name()) } });
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    if(observed) reach_action_start(target, library, table, random);
    REQUIRE(target.step(library, table, random) == givm::execution_state::action_selection);
    const auto calls_before_queries = random.calls;
    const auto action = target.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.switch_target_count() == 2);
    const givm::character_id first{ givm::player_id{ 0 }, 1 };
    const givm::character_id second{ givm::player_id{ 0 }, 2 };
    CHECK(action.switch_target(0) == first);
    CHECK(action.switch_target(1) == second);
    if(quote_both)
    {
        CHECK(action.calculate_switch_cost(library, table, 1).requirement.dice_requirement.any == 3);
        CHECK(action.switch_cost(0).target == first);
        CHECK(action.switch_cost(0).requirement.dice_requirement.any == 1);
    }
    const auto& first_cost = action.calculate_switch_cost(library, table, 0);
    CHECK(first_cost.target == first);
    CHECK(first_cost.requirement.dice_requirement.any == 2);
    if(quote_both)
    {
        CHECK(action.calculate_switch_cost(library, table, 1).requirement.dice_requirement.any == 3);
    }
    CHECK(action.calculate_switch_cost(library, table, 0).requirement.dice_requirement.any == 2);
    CHECK(action.switch_cost(1).target == second);
    CHECK(action.switch_cost(1).requirement.dice_requirement.any == (quote_both ? 3 : 1));
    const auto expected_quotes = quote_both ? std::vector{ second, first, second, first } : std::vector{ first, first };
    CHECK(control.quoted == expected_quotes);
    CHECK(empty_control.quoted == expected_quotes);
    CHECK(random.calls == calls_before_queries);
    CHECK(action.switch_payment_validate(table, 0, dice({ { givm::elemental_dice::omni, 2 } })) == givm::switch_payment_validation::valid);
    if(quote_both)
    {
        CHECK(action.switch_payment_validate(table, 1, dice({ { givm::elemental_dice::omni, 3 } })) == givm::switch_payment_validation::valid);
    }
    CHECK(control.quoted == expected_quotes);
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 0);
    CHECK(table[givm::player_id{ 0 }].deck_card_count() == 3);
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 4);
    CHECK(table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 0 });
    CHECK(table.state().active_player == givm::player_id{ 0 });

    for(std::size_t index = 0; index < (quote_both ? 2 : 1); ++index)
    {
        auto branch = target;
        auto branch_table = table;
        const auto paid = dice({ { givm::elemental_dice::omni, static_cast<std::uint8_t>(index + 2) } });
        const auto branch_action = branch.view_in<givm::execution_state::action_selection>();
        CHECK(branch_action.switch_payment_validate(branch_table, index, paid) == givm::switch_payment_validation::valid);
        branch_action.switch_active_character(index, paid);
        auto state = branch.step(library, branch_table, random);
        if(observed)
        {
            REQUIRE(state == givm::execution_state::active_character_changed);
            CHECK(branch.view_in<givm::execution_state::active_character_changed>().character() == (index == 0 ? first : second));
            CHECK(branch_table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 0 });
            REQUIRE(branch.step(library, branch_table, random) == givm::execution_state::action_started);
            state = branch.step(library, branch_table, random);
        }
        REQUIRE(state == givm::execution_state::action_selection);
        CHECK(branch_table[givm::player_id{ 0 }].hand_card_count() == index + 1);
        CHECK(branch_table[givm::player_id{ 0 }].deck_card_count() == 2 - index);
        CHECK(branch_table[givm::player_id{ 0 }].state().dice.total() == 2 - index);
        CHECK(branch_table[givm::player_id{ 0 }].state().active_character == (index == 0 ? first : second));
        CHECK(branch_table.state().active_player == givm::player_id{ 1 });
        CHECK(control.quoted == expected_quotes);
        CHECK(empty_control.quoted == expected_quotes);
    }
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 0);
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 4);
    CHECK(target.view_in<givm::execution_state::action_selection>().switch_cost(0).requirement.dice_requirement.any == 2);
    CHECK(target.view_in<givm::execution_state::action_selection>().switch_cost(1).requirement.dice_requirement.any == (quote_both ? 3 : 1));
}

TEST_CASE("payment checks match exact dice requirements before checking the player's inventory", "[begin_action][payment][compile-mode]")
{
    using enum givm::elemental_dice;
    using enum givm::switch_payment_validation;
    const bool observed = GENERATE(false, true);
    const auto inventory = dice({ { omni, 4 }, { pyro, 3 }, { hydro, 2 }, { cryo, 1 }, { electro, 1 } });
    quote_control control{ .replace_requirement = true, .enable_payment = true };
    const quote_source source{ &control, "PaymentRequirements", inventory };
    const givm::test::initialized_character_source character;
    const givm::test::named_definition_source<givm::card_definition> card{ "UncommittedPaymentCard" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple_cat(action_setup(inventory.total()), std::tuple{ givm::begin_action{} }), std::tuple{}, source, character, card
    );
    const auto plain = ids.get_id<givm::character_view>(character.name());
    givm::table table;
    table.load_deck(givm::player_id{ 0 }, {
        .cards = { ids.get_id<givm::card_definition>(card.name()) },
        .characters = { ids.get_id<givm::character_view>(source.name()), plain }
    });
    table.load_deck(givm::player_id{ 1 }, { .characters = { plain } });
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    if(observed) reach_action_start(target, library, table, random);
    REQUIRE(target.step(library, table, random) == givm::execution_state::action_selection);
    REQUIRE(table[givm::player_id{ 0 }].state().dice == inventory);
    const auto random_calls = random.calls;

    struct payment_example
    {
        const char* description;
        givm::elemental_dice_requirement requirement;
        givm::dice_counts paid;
        givm::switch_payment_validation expected;
    };
    const std::vector<payment_example> examples{
        { "one arbitrary die", { .any = 1 }, dice({ { pyro, 1 } }), valid },
        { "underpayment", { .any = 1 }, {}, requirement_mismatch },
        { "overpayment", { .any = 1 }, dice({ { pyro, 2 } }), requirement_mismatch },
        { "matching fixed element", { .fixed = dice({ { pyro, 2 } }) }, dice({ { pyro, 2 } }), valid },
        { "omni substitutes for fixed element", { .fixed = dice({ { pyro, 2 } }) }, dice({ { omni, 2 } }), valid },
        { "wrong fixed element", { .fixed = dice({ { pyro, 2 } }) }, dice({ { hydro, 2 } }), requirement_mismatch },
        { "fixed element partly unmet", { .fixed = dice({ { pyro, 2 } }) }, dice({ { pyro, 1 }, { hydro, 1 } }), requirement_mismatch },
        { "omni completes same-element cost", { .same = 3 }, dice({ { pyro, 2 }, { omni, 1 } }), valid },
        { "mixed elements cannot meet same-element cost", { .same = 3 }, dice({ { pyro, 1 }, { hydro, 1 }, { cryo, 1 } }), requirement_mismatch },
        { "fixed same and arbitrary costs together",
            { .fixed = dice({ { pyro, 1 }, { hydro, 1 } }), .same = 2, .any = 1 },
            dice({ { pyro, 2 }, { hydro, 1 }, { omni, 1 }, { cryo, 1 } }), valid },
        { "omni covers fixed shortage and same-element cost",
            { .fixed = dice({ { pyro, 2 }, { hydro, 1 } }), .same = 2, .any = 1 },
            dice({ { pyro, 1 }, { hydro, 1 }, { omni, 3 }, { cryo, 1 } }), valid },
        { "fixed payment leaves no three matching dice",
            { .fixed = dice({ { pyro, 1 } }), .same = 3, .any = 1 },
            dice({ { pyro, 1 }, { hydro, 2 }, { cryo, 1 }, { electro, 1 } }), requirement_mismatch },
        { "requirement matches but die is not owned", { .any = 1 }, dice({ { dendro, 1 } }), insufficient_dice },
        { "requirement failure precedes unavailable inventory", { .fixed = dice({ { pyro, 2 } }) }, dice({ { dendro, 1 } }), requirement_mismatch },
        { "zero cost", {}, {}, valid },
        { "zero cost cannot be overpaid", {}, dice({ { omni, 1 } }), requirement_mismatch },
        { "explicit omni and fixed element", { .fixed = dice({ { omni, 1 }, { pyro, 1 } }) }, dice({ { omni, 1 }, { pyro, 1 } }), valid },
        { "ordinary dice cannot replace explicit omni", { .fixed = dice({ { omni, 1 }, { pyro, 1 } }) }, dice({ { pyro, 2 } }), requirement_mismatch },
        { "omni satisfies its explicit requirement and substitutes", { .fixed = dice({ { omni, 1 }, { pyro, 1 } }) }, dice({ { omni, 2 } }), valid },
        { "payment totals do not wrap at 255", { .same = 1, .any = 255 }, dice({ { omni, 255 }, { pyro, 1 } }), insufficient_dice }
    };
    const auto action = target.view_in<givm::execution_state::action_selection>();
    for(const auto& example : examples)
    {
        CAPTURE(example.description);
        control.requirement = example.requirement;
        action.calculate_switch_cost(library, table, 0);
        const auto quote_count = control.quoted.size();
        CHECK(action.switch_payment_validate(table, 0, example.paid) == example.expected);
        CHECK(action.switch_payment_validate(table, 0, example.paid) == example.expected);
        CHECK(control.quoted.size() == quote_count);
        CHECK(random.calls == random_calls);
        CHECK(table[givm::player_id{ 0 }].state().dice == inventory);
        CHECK(table[givm::player_id{ 0 }].hand_card_count() == 0);
        CHECK(table[givm::player_id{ 0 }].deck_card_count() == 1);
        CHECK(table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 0 });
    }
    CHECK(control.quoted.size() == examples.size());
    CHECK(random.calls == random_calls);
}

TEST_CASE("recalculating a candidate replaces a previously nonempty payment response", "[begin_action][onpay][compile-mode]")
{
    const bool observed = GENERATE(false, true);
    const bool automatic_quote = GENERATE(false, true);
    quote_control control{ .enable_payment = true };
    const quote_source source{ &control };
    const givm::test::initialized_character_source character;
    const givm::test::named_definition_source<givm::card_definition> card{ "DiscardedPaymentResponse" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple_cat(action_setup(), std::tuple{ givm::begin_action{} }), std::tuple{}, source, character, card
    );
    const auto plain = ids.get_id<givm::character_view>(character.name());
    givm::table table;
    table.load_deck(givm::player_id{ 0 }, {
        .cards = { ids.get_id<givm::card_definition>(card.name()) },
        .characters = { ids.get_id<givm::character_view>(source.name()), plain }
    });
    table.load_deck(givm::player_id{ 1 }, { .characters = { plain } });
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    if(observed) reach_action_start(target, library, table, random);
    REQUIRE(target.step(library, table, random) == givm::execution_state::action_selection);
    const auto random_calls = random.calls;
    const auto action = target.view_in<givm::execution_state::action_selection>();
    const auto paid = dice({ { givm::elemental_dice::omni, 1 } });
    action.calculate_switch_cost(library, table, 0);
    REQUIRE(control.quoted.size() == 1);
    CHECK(random.calls == random_calls);
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 4);
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 0);
    CHECK(table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 0 });

    control.enable_payment = false;
    if(automatic_quote)
    {
        action.switch_active_character(library, table, 0, paid);
    }
    else
    {
        CHECK(action.calculate_switch_cost(library, table, 0).requirement.dice_requirement.any == 1);
        CHECK(action.switch_payment_validate(table, 0, paid) == givm::switch_payment_validation::valid);
        action.switch_active_character(0, paid);
    }
    REQUIRE(control.quoted.size() == 2);
    auto state = target.step(library, table, random);
    if(observed)
    {
        REQUIRE(state == givm::execution_state::active_character_changed);
        REQUIRE(target.step(library, table, random) == givm::execution_state::action_started);
        state = target.step(library, table, random);
    }
    REQUIRE(state == givm::execution_state::action_selection);
    CHECK(control.quoted.size() == 2);
    CHECK(random.calls == random_calls);
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 0);
    CHECK(table[givm::player_id{ 0 }].deck_card_count() == 1);
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 3);
    CHECK(table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 1 });
    CHECK(table.state().active_player == givm::player_id{ 1 });
}
