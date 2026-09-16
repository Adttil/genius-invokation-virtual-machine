#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
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
    using card_targets = std::array<givm::card_target_id, 2>;

    struct play_log
    {
        std::vector<givm::hand_card_id> initialized;
        std::vector<givm::hand_card_id> quoted;
        std::vector<givm::hand_card_id> effects;
        std::vector<card_targets> effect_targets;
        std::vector<card_targets> played_targets;
        std::vector<std::string> events;
        std::vector<std::uint32_t> dice_at_draw;
        std::optional<givm::hand_card_id> selected;
        std::uint32_t target_checks = 0;
        std::uint32_t removed_card_broadcasts = 0;
        std::uint8_t extra_cost = 0;
        bool condition_met = true;
        bool enable_payment = true;
        bool nested = false;
        bool record = false;
        bool cancel_effect = false;
    };

    void check_removed_card(const givm::table& table, givm::hand_card_id id,
                            givm::definition_id<givm::card_definition> definition)
    {
        const auto card = table[id];
        CHECK_FALSE(card.is_valid());
        CHECK(card.id() == id);
        CHECK(card.definition_id() == definition);
        CHECK(card.player().id() == id.player_id);
        (void)card.state();
        for(const auto remaining : table[id.player_id].hand_cards()) CHECK(remaining.id() != id);
    }

    struct playable_card_source
    {
        using definition_category = givm::card_definition;
        struct definition_type
        {
            play_log* log;
            std::uint8_t cost;
            givm::action_speed speed;
            givm::program_entry<givm::card_effect> effect;
        };
        play_log* log;
        std::string_view source_name;
        std::uint8_t cost = 1;
        givm::action_speed speed = givm::action_speed::fast;

        std::string_view name() const noexcept { return source_name; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, cost, speed,
                context.add_program<givm::card_effect>(std::tuple{ givm::draw_cards{ .count = 1 } }) };
        }
        static givm::handler_program_entry_t<givm::card_cost_initialization> handle(
            const definition_type& data, const givm::hand_card_view& self,
            givm::card_cost_initialization& event, const givm::table&, givm::random_fn&)
        {
            CHECK(event.card == self.id());
            CHECK(event.requirement.dice_requirement.any == 0);
            CHECK(event.requirement.speed == givm::action_speed::fast);
            data.log->initialized.push_back(event.card);
            event.requirement.dice_requirement.any = data.cost;
            event.requirement.speed = data.speed;
            return givm::handler_program_entry_t<givm::card_cost_initialization>::null();
        }
        static givm::handler_program_entry_t<givm::card_target_check> handle(
            const definition_type& data, const givm::hand_card_view& self,
            givm::card_target_check& event, const givm::table&, givm::random_fn&)
        {
            CHECK(event.card == self.id());
            ++data.log->target_checks;
            const auto* first = std::get_if<givm::character_id>(&event.targets[0]);
            const auto* second = std::get_if<givm::character_id>(&event.targets[1]);
            if(first == nullptr || first->player_id != self.id().player_id)
                event.result = givm::card_target_check_result::invalid_first_target;
            else if(second == nullptr || second->player_id == self.id().player_id)
                event.result = givm::card_target_check_result::invalid_second_target;
            else if(not data.log->condition_met)
                event.result = givm::card_target_check_result::unmet_condition;
            return givm::handler_program_entry_t<givm::card_target_check>::null();
        }
        static givm::program_entry<givm::card_effect> handle(
            const definition_type& data, const givm::hand_card_view& self,
            givm::card_effect& event, const givm::table& table, givm::random_fn&)
        {
            CHECK(event.card == self.id());
            check_removed_card(table, event.card, self.definition_id());
            (void)self.state();
            data.log->effects.push_back(event.card);
            data.log->effect_targets.push_back(event.targets);
            if(data.log->record) data.log->events.push_back("effect");
            return data.log->nested ? data.effect : givm::program_entry<givm::card_effect>::null();
        }
        template<class TEvent>
            requires(std::same_as<TEvent, givm::card_drawn>
                || std::same_as<TEvent, givm::card_will_be_played>
                || std::same_as<TEvent, givm::card_played>)
        static givm::program_entry<TEvent> handle(
            const definition_type& data, const givm::hand_card_view& self,
            TEvent&, const givm::table&, givm::random_fn&)
        {
            if(data.log->record && data.log->selected == self.id()) ++data.log->removed_card_broadcasts;
            return givm::program_entry<TEvent>::null();
        }
    };

    struct play_observer_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            play_log* log;
            givm::handler_program_entry_t<givm::cost_of_card> first_payment;
            givm::handler_program_entry_t<givm::cost_of_card> second_payment;
            givm::program_entry<givm::card_will_be_played> before;
            givm::program_entry<givm::card_played> after;
            givm::program_entry<givm::card_drawn> selection;
        };
        play_log* log;
        std::string_view name() const noexcept { return "PlayObserver"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return {
                log,
                context.add_program<givm::handler_program_context_t<givm::cost_of_card>>(
                    std::tuple{ givm::draw_cards{ .count = 1 } }),
                context.add_program<givm::handler_program_context_t<givm::cost_of_card>>(
                    std::tuple{ givm::draw_cards{ .count = 2 } }),
                context.add_program<givm::card_will_be_played>(std::tuple{ givm::draw_cards{ .count = 1 } }),
                context.add_program<givm::card_played>(std::tuple{ givm::draw_cards{ .count = 1 } }),
                context.add_program<givm::card_drawn>(std::tuple{ givm::replace_cards{ .player = givm::player_id{ 0 } } })
            };
        }
        static givm::handler_program_entry_t<givm::character_initialization> handle(
            const definition_type&, const givm::character_view&, givm::character_initialization& event,
            const givm::table&, givm::random_fn&)
        {
            event.state = { .max_health = 10, .health = 10 };
            return givm::handler_program_entry_t<givm::character_initialization>::null();
        }
        static givm::handler_program_entry_t<givm::cost_of_card> handle(
            const definition_type& data, const givm::character_view&, givm::cost_of_card& event,
            const givm::table&, givm::random_fn&)
        {
            data.log->quoted.push_back(event.card);
            event.requirement.dice_requirement.any += data.log->extra_cost;
            if(not data.log->enable_payment) return givm::handler_program_entry_t<givm::cost_of_card>::null();
            return event.requirement.dice_requirement.any > 2 ? data.second_payment : data.first_payment;
        }
        static givm::program_entry<givm::dice_removed> handle(
            const definition_type& data, const givm::character_view&, givm::dice_removed& event,
            const givm::table& table, givm::random_fn&)
        {
            if(data.log->record)
            {
                data.log->events.push_back("dice");
                CHECK(event.player == givm::player_id{ 0 });
                CHECK(event.dice.total() == 1);
                CHECK(table[event.player].state().dice.total() == 3);
            }
            return givm::program_entry<givm::dice_removed>::null();
        }
        static givm::program_entry<givm::card_will_be_played> handle(
            const definition_type& data, const givm::character_view&, givm::card_will_be_played& event,
            const givm::table& table, givm::random_fn&)
        {
            check_removed_card(table, event.card, event.definition_id);
            if(data.log->record) data.log->events.push_back("will");
            event.effect_cancelled = data.log->cancel_effect;
            return data.log->nested ? data.before : givm::program_entry<givm::card_will_be_played>::null();
        }
        static givm::program_entry<givm::card_played> handle(
            const definition_type& data, const givm::character_view&, givm::card_played& event,
            const givm::table& table, givm::random_fn&)
        {
            check_removed_card(table, event.card, event.definition_id);
            data.log->played_targets.push_back(event.targets);
            if(data.log->record) data.log->events.push_back("played");
            return data.log->nested ? data.after : givm::program_entry<givm::card_played>::null();
        }
        static givm::program_entry<givm::card_drawn> handle(
            const definition_type& data, const givm::character_view&, givm::card_drawn& event,
            const givm::table& table, givm::random_fn&)
        {
            if(not data.log->record) return givm::program_entry<givm::card_drawn>::null();
            data.log->events.push_back("draw");
            data.log->dice_at_draw.push_back(table[event.card.player_id].state().dice.total());
            return data.log->nested ? data.selection : givm::program_entry<givm::card_drawn>::null();
        }
    };

    struct untargeted_card_source
    {
        using definition_category = givm::card_definition;
        struct definition_type { play_log* log; };
        play_log* log;
        std::string_view name() const noexcept { return "ZeroCostCard"; }
        definition_type compile(givm::definition_compile_context&) const { return { log }; }
        static givm::handler_program_entry_t<givm::card_cost_initialization> handle(
            const definition_type& data, const givm::hand_card_view& self,
            givm::card_cost_initialization& event, const givm::table&, givm::random_fn&)
        {
            CHECK(event.card == self.id());
            data.log->initialized.push_back(event.card);
            return givm::handler_program_entry_t<givm::card_cost_initialization>::null();
        }
        static givm::program_entry<givm::card_effect> handle(
            const definition_type& data, const givm::hand_card_view& self,
            givm::card_effect& event, const givm::table& table, givm::random_fn&)
        {
            check_removed_card(table, event.card, self.definition_id());
            data.log->effects.push_back(event.card);
            data.log->effect_targets.push_back(event.targets);
            return givm::program_entry<givm::card_effect>::null();
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

    auto setup(std::uint32_t cards)
    {
        return std::tuple{
            givm::initialize_characters{ givm::player_id{ 0 } },
            givm::initialize_characters{ givm::player_id{ 1 } },
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

TEST_CASE("card quotes remain independent and copied executions pay only for the selected card", "[play_card][payment][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool quote_both = GENERATE(false, true);
    play_log log{ .extra_cost = 1 };
    const playable_card_source first_source{ &log, "FirstPlayedCard", 1 };
    const playable_card_source second_source{ &log, "SecondPlayedCard", 2, givm::action_speed::combat };
    const play_observer_source observer{ &log };
    const givm::test::initialized_character_source character;
    const givm::test::named_definition_source<givm::card_definition> filler{ "PlayFiller" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        mode, setup(2), std::tuple{}, first_source, second_source, observer, character, filler);
    const auto filler_id = ids.get_id<givm::card_definition>(filler.name());
    givm::table table;
    table.load_deck(givm::player_id{ 0 }, {
        .cards = { filler_id, filler_id, filler_id,
            ids.get_id<givm::card_definition>(second_source.name()), ids.get_id<givm::card_definition>(first_source.name()) },
        .characters = { ids.get_id<givm::character_view>(observer.name()) }
    });
    table.load_deck(givm::player_id{ 1 }, { .characters = { ids.get_id<givm::character_view>(character.name()) } });
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    const auto action = target.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.card_costs().size() == 2);
    const auto first = action.card_costs()[0].card;
    const auto second = action.card_costs()[1].card;
    const card_targets targets{ givm::character_id{ givm::player_id{ 0 }, 0 }, givm::character_id{ givm::player_id{ 1 }, 0 } };
    REQUIRE(table[first].definition_id() == ids.get_id<givm::card_definition>(first_source.name()));
    REQUIRE(table[second].definition_id() == ids.get_id<givm::card_definition>(second_source.name()));
    CHECK(log.initialized.empty());
    CHECK(log.quoted.empty());
    const auto random_calls = random.calls;
    if(quote_both) CHECK(action.calculate_card_cost(library, table, second).requirement.dice_requirement.any == 3);
    CHECK(action.calculate_card_cost(library, table, first).requirement.dice_requirement.any == 2);
    CHECK(action.calculate_card_cost(library, table, first).requirement.dice_requirement.any == 2);
    const auto expected_quotes = quote_both ? std::vector{ second, first, first } : std::vector{ first, first };
    CHECK(log.initialized == expected_quotes);
    CHECK(log.quoted == expected_quotes);
    CHECK(action.card_costs()[1].requirement.dice_requirement.any == (quote_both ? 3 : 0));
    CHECK(action.check_card_payment(table, first, pay(2)) == givm::card_payment_check_result::valid);
    CHECK(action.check_card_payment(table, first, {}) == givm::card_payment_check_result::requirement_mismatch);
    CHECK(action.check_card_payment(table, first, pay(3)) == givm::card_payment_check_result::requirement_mismatch);
    CHECK(action.check_card_payment(table, first, pay(2, givm::elemental_dice::dendro))
        == givm::card_payment_check_result::insufficient_dice);
    CHECK(log.quoted == expected_quotes);
    CHECK(random.calls == random_calls);
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 2);
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 4);
    for(std::size_t index = 0; index < (quote_both ? 2 : 1); ++index)
    {
        auto branch = target;
        auto branch_table = table;
        log.effects.clear();
        log.effect_targets.clear();
        log.played_targets.clear();
        const auto selected = index == 0 ? first : second;
        const auto branch_action = branch.view_in<givm::execution_state::action_selection>();
        const auto paid = pay(static_cast<std::uint8_t>(index + 2));
        CHECK(branch_action.check_card_payment(branch_table, selected, paid) == givm::card_payment_check_result::valid);
        branch_action.play_card(selected, targets, paid);
        REQUIRE(advance(branch, library, branch_table, random) == givm::execution_state::action_selection);
        CHECK(log.effects == std::vector{ selected });
        CHECK(log.effect_targets == std::vector{ targets });
        CHECK(log.played_targets == std::vector{ targets });
        CHECK(log.quoted == expected_quotes);
        CHECK(log.target_checks == 0);
        CHECK_FALSE(branch_table[selected].is_valid());
        CHECK(branch_table[index == 0 ? second : first].is_valid());
        CHECK(branch_table[givm::player_id{ 0 }].hand_card_count() == index + 2);
        CHECK(branch_table[givm::player_id{ 0 }].deck_card_count() == 2 - index);
        CHECK(branch_table[givm::player_id{ 0 }].state().dice.total() == 2 - index);
        CHECK(branch_table.state().active_player == (index == 0 ? givm::player_id{ 0 } : givm::player_id{ 1 }));
    }
    CHECK(table[first].is_valid());
    CHECK(table[second].is_valid());
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 4);
}

TEST_CASE("card target checks are explicit and untargeted cards need no target handler", "[play_card][targets][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    play_log log{ .enable_payment = false };
    const playable_card_source source{ &log, "TargetedCard", 0 };
    const play_observer_source observer{ &log };
    const givm::test::initialized_character_source character;
    const untargeted_card_source plain{ &log };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        mode, setup(2), std::tuple{}, source, observer, character, plain);
    givm::table table;
    table.load_deck(givm::player_id{ 0 }, {
        .cards = { ids.get_id<givm::card_definition>(plain.name()), ids.get_id<givm::card_definition>(source.name()) },
        .characters = { ids.get_id<givm::character_view>(observer.name()) }
    });
    table.load_deck(givm::player_id{ 1 }, { .characters = { ids.get_id<givm::character_view>(character.name()) } });
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    const auto action = target.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.card_costs().size() == 2);
    const auto targeted = action.card_costs()[0].card;
    const auto plain_card = action.card_costs()[1].card;
    const card_targets targets{ givm::character_id{ givm::player_id{ 0 }, 0 }, givm::character_id{ givm::player_id{ 1 }, 0 } };
    CHECK(action.check_card_targets(library, table, targeted, {}) == givm::card_target_check_result::invalid_first_target);
    CHECK(action.check_card_targets(library, table, targeted, { targets[0], targets[0] })
        == givm::card_target_check_result::invalid_second_target);
    log.condition_met = false;
    CHECK(action.check_card_targets(library, table, targeted, targets) == givm::card_target_check_result::unmet_condition);
    log.condition_met = true;
    CHECK(action.check_card_targets(library, table, targeted, targets) == givm::card_target_check_result::valid);
    CHECK(log.target_checks == 4);
    CHECK(log.quoted.empty());
    CHECK(log.effects.empty());
    const auto& cost = action.calculate_card_cost(library, table, targeted);
    CHECK(cost.requirement.dice_requirement.any == 0);
    CHECK(action.check_card_payment(table, targeted, {}) == givm::card_payment_check_result::valid);
    CHECK(action.check_card_payment(table, targeted, pay(1)) == givm::card_payment_check_result::requirement_mismatch);
    action.play_card(library, table, targeted, targets, {});
    CHECK(log.quoted == std::vector{ targeted, targeted });
    CHECK(log.target_checks == 4);
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    CHECK(log.effects == std::vector{ targeted });
    CHECK(log.effect_targets == std::vector{ targets });
    CHECK(log.played_targets == std::vector{ targets });
    CHECK(log.target_checks == 4);
    CHECK(log.quoted == std::vector{ targeted, targeted });
    CHECK(table.state().active_player == givm::player_id{ 0 });
    const auto next_action = target.view_in<givm::execution_state::action_selection>();
    REQUIRE(next_action.card_costs().size() == 1);
    CHECK(next_action.card_costs()[0].card == plain_card);
    const auto& plain_cost = next_action.calculate_card_cost(library, table, plain_card);
    CHECK(plain_cost.requirement.dice_requirement.any == 0);
    CHECK(plain_cost.requirement.speed == givm::action_speed::fast);
    CHECK(next_action.check_card_targets(library, table, plain_card, {}) == givm::card_target_check_result::valid);
    next_action.play_card(plain_card, {}, {});
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 0);
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 4);
    CHECK(table.state().active_player == givm::player_id{ 0 });
    CHECK(log.target_checks == 4);
    CHECK(log.effects == std::vector{ targeted, plain_card });
}

TEST_CASE("card payment and broadcasts resume in order after removal even when its effect is countered", "[play_card][onpay][broadcast][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool countered = GENERATE(false, true);
    const bool automatic_quote = GENERATE(false, true);
    play_log log{ .nested = true, .cancel_effect = countered };
    const playable_card_source source{ &log, "NestedPlayedCard", 1, givm::action_speed::combat };
    const play_observer_source observer{ &log };
    const givm::test::initialized_character_source character;
    const givm::test::named_definition_source<givm::card_definition> filler{ "NestedFiller" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        mode, setup(1), std::tuple{}, source, observer, character, filler);
    const auto card_definition = ids.get_id<givm::card_definition>(source.name());
    const auto filler_definition = ids.get_id<givm::card_definition>(filler.name());
    givm::table table{ givm::game_parameters{ .hand_limit = 4 } };
    table.load_deck(givm::player_id{ 0 }, {
        .cards = { filler_definition, filler_definition, filler_definition, filler_definition, card_definition },
        .characters = { ids.get_id<givm::character_view>(observer.name()) }
    });
    table.load_deck(givm::player_id{ 1 }, { .characters = { ids.get_id<givm::character_view>(character.name()) } });
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    const auto action = target.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.card_costs().size() == 1);
    const auto card = action.card_costs()[0].card;
    const card_targets targets{ givm::character_id{ givm::player_id{ 0 }, 0 }, givm::character_id{ givm::player_id{ 1 }, 0 } };
    log.selected = card;
    log.record = true;
    if(automatic_quote) action.play_card(library, table, card, targets, pay(1));
    else
    {
        action.calculate_card_cost(library, table, card);
        action.play_card(card, targets, pay(1));
    }
    CHECK(log.quoted == std::vector{ card });
    CHECK(log.events.empty());
    REQUIRE(advance(target, library, table, random) == givm::execution_state::card_selection);
    CHECK(log.events == std::vector<std::string>{ "draw" });
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 1);
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 4);
    check_removed_card(table, card, card_definition);
    const auto paused_log = log;
    auto copied_target = target;
    auto copied_table = table;
    const auto resume = [&](givm::executor& execution, givm::table& current_table)
    {
        log = paused_log;
        const std::uint32_t draws = countered ? 3 : 4;
        for(std::uint32_t index = 0; index < draws; ++index)
        {
            CHECK(execution.view_in<givm::execution_state::card_selection>().player() == givm::player_id{ 0 });
            CHECK(current_table[givm::player_id{ 0 }].hand_card_count() == index + 1);
            check_removed_card(current_table, card, card_definition);
            execution.view_in<givm::execution_state::card_selection>().select({});
            REQUIRE(advance(execution, library, current_table, random)
                == (index + 1 == draws ? givm::execution_state::action_selection : givm::execution_state::card_selection));
        }
        CHECK(log.events == (countered
            ? std::vector<std::string>{ "draw", "dice", "will", "draw", "played", "draw" }
            : std::vector<std::string>{ "draw", "dice", "will", "draw", "effect", "draw", "played", "draw" }));
        CHECK(log.dice_at_draw == (countered ? std::vector<std::uint32_t>{ 4, 3, 3 } : std::vector<std::uint32_t>{ 4, 3, 3, 3 }));
        CHECK(log.effects.size() == (countered ? 0 : 1));
        CHECK(log.effect_targets == (countered ? std::vector<card_targets>{} : std::vector{ targets }));
        CHECK(log.played_targets == std::vector{ targets });
        CHECK(log.quoted == std::vector{ card });
        CHECK(log.removed_card_broadcasts == 0);
        CHECK(log.target_checks == 0);
        CHECK(current_table[givm::player_id{ 0 }].state().dice.total() == 3);
        CHECK(current_table[givm::player_id{ 0 }].hand_card_count() == draws);
        CHECK(current_table[givm::player_id{ 0 }].deck_card_count() == 4 - draws);
        CHECK(current_table.state().active_player == givm::player_id{ 1 });
    };
    resume(target, table);
    resume(copied_target, copied_table);
}
