#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
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
    using card_targets = std::array<givm::card_target_id, 2>;

    struct play_log
    {
        std::uint32_t initial_cost_queries = 0;
        std::vector<givm::hand_card_id> quoted;
        std::vector<givm::hand_card_id> effects;
        std::vector<card_targets> effect_targets;
        std::vector<card_targets> played_targets;
        std::vector<std::size_t> target_validation_steps;
        std::vector<card_targets> checked_targets;
        std::vector<std::string> events;
        std::vector<std::uint32_t> dice_at_draw;
        std::optional<givm::hand_card_id> selected;
        std::uint32_t target_validations = 0;
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
            bool single_target;
            bool optional_targets;
            givm::program_entry effect;
        };
        play_log* log;
        std::string_view source_name;
        std::uint8_t cost = 1;
        givm::action_speed speed = givm::action_speed::fast;
        bool single_target = false;
        bool optional_targets = false;

        std::string_view name() const noexcept { return source_name; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, cost, speed, single_target, optional_targets,
                context.add_program(std::tuple{ givm::draw_cards{ .count = 1 } }) };
        }
        static givm::card_state query(const definition_type& data, const givm::card_initial_state&)
        {
            ++data.log->initial_cost_queries;
            return { .cost = { .dice_requirement = { .any = data.cost }, .speed = data.speed } };
        }
        static givm::target_validation query(const definition_type& data, const givm::card_target_validation& parameters)
        {
            ++data.log->target_validations;
            data.log->target_validation_steps.push_back(parameters.target_count);
            data.log->checked_targets.push_back(parameters.targets);
            if(parameters.target_count == 0)
            {
                if(not data.log->condition_met)
                    return givm::target_validation::invalid;
                return data.optional_targets ? givm::target_validation::valid_complete_or_continue
                    : givm::target_validation::valid_incomplete;
            }
            if(parameters.target_count == 1)
            {
                const auto* first = std::get_if<givm::character_id>(&parameters.targets[0]);
                if(first == nullptr || first->player_id != parameters.card.id().player_id || not data.log->condition_met)
                    return givm::target_validation::invalid;
                return data.single_target ? givm::target_validation::valid_complete
                    : data.optional_targets ? givm::target_validation::valid_complete_or_continue
                    : givm::target_validation::valid_incomplete;
            }
            if(data.single_target) return givm::target_validation::invalid;
            const auto first = std::get<givm::character_id>(parameters.targets[0]);
            const auto* second = std::get_if<givm::character_id>(&parameters.targets[1]);
            return second != nullptr && second->player_id != first.player_id
                ? givm::target_validation::valid_complete : givm::target_validation::invalid;
        }
        static givm::program_entry handle(
            const definition_type& data, const givm::hand_card_view& self,
            givm::card_effect& event, givm::handle_context& context)
        {
            CHECK(event.card == self.id());
            check_removed_card(context.table(), event.card, self.definition_id());
            (void)self.state();
            data.log->effects.push_back(event.card);
            data.log->effect_targets.push_back(event.targets);
            if(data.log->record) data.log->events.push_back("effect");
            if(data.log->nested) return context.invoke(data.effect);
            return {};
        }
        template<class TEvent>
            requires(std::same_as<TEvent, givm::card_drawn>
                || std::same_as<TEvent, givm::card_will_be_played>
                || std::same_as<TEvent, givm::card_played>)
        static givm::program_entry handle(
            const definition_type& data, const givm::hand_card_view& self,
            TEvent&, givm::handle_context&)
        {
            if(data.log->record && data.log->selected == self.id()) ++data.log->removed_card_broadcasts;
            return {};
        }
    };

    struct play_observer_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            play_log* log;
            givm::program_entry first_payment;
            givm::program_entry second_payment;
            givm::program_entry before;
            givm::program_entry after;
            givm::program_entry selection;
        };
        play_log* log;
        std::string_view name() const noexcept { return "PlayObserver"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return {
                log,
                context.add_program(
                    std::tuple{ givm::draw_cards{ .count = 1 } }),
                context.add_program(
                    std::tuple{ givm::draw_cards{ .count = 2 } }),
                context.add_program(std::tuple{ givm::draw_cards{ .count = 1 } }),
                context.add_program(std::tuple{ givm::draw_cards{ .count = 1 } }),
                context.add_program(std::tuple{ givm::replace_cards{ .player = givm::player_id{ 0 } } })
            };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry handle(
            const definition_type& data, const givm::character_view&, givm::cost_of_card& event,
            givm::handle_context& context)
        {
            data.log->quoted.push_back(event.card);
            event.requirement.dice_requirement.any += data.log->extra_cost;
            if(not data.log->enable_payment) return {};
            return context.invoke(givm::substack_t{}, event.requirement.dice_requirement.any > 2 ? data.second_payment : data.first_payment);
        }
        static givm::program_entry handle(
            const definition_type& data, const givm::character_view&, givm::dice_removed& event,
            givm::handle_context& context)
        {
            if(data.log->record)
            {
                data.log->events.push_back("dice");
                CHECK(event.player == givm::player_id{ 0 });
                CHECK(event.dice.total() == 1);
                CHECK(context.table()[event.player].state().dice.total() == 3);
            }
            return {};
        }
        static givm::program_entry handle(
            const definition_type& data, const givm::character_view&, givm::card_will_be_played& event,
            givm::handle_context& context)
        {
            check_removed_card(context.table(), event.card, event.definition_id);
            if(data.log->record) data.log->events.push_back("will");
            event.effect_cancelled = data.log->cancel_effect;
            if(data.log->nested) return context.invoke(data.before);
            return {};
        }
        static givm::program_entry handle(
            const definition_type& data, const givm::character_view&, givm::card_played& event,
            givm::handle_context& context)
        {
            check_removed_card(context.table(), event.card, event.definition_id);
            data.log->played_targets.push_back(event.targets);
            if(data.log->record) data.log->events.push_back("played");
            if(data.log->nested) return context.invoke(data.after);
            return {};
        }
        static givm::program_entry handle(
            const definition_type& data, const givm::character_view&, givm::card_drawn& event,
            givm::handle_context& context)
        {
            if(not data.log->record) return {};
            data.log->events.push_back("draw");
            data.log->dice_at_draw.push_back(context.table()[event.card.player_id].state().dice.total());
            if(data.log->nested) return context.invoke(data.selection);
            return {};
        }
    };

    struct untargeted_card_source
    {
        using definition_category = givm::card_definition;
        struct definition_type { play_log* log; };
        play_log* log;
        std::string_view name() const noexcept { return "ZeroCostCard"; }
        definition_type compile(givm::definition_compile_context&) const { return { log }; }
        static givm::program_entry handle(
            const definition_type& data, const givm::hand_card_view& self,
            givm::card_effect& event, givm::handle_context& context)
        {
            check_removed_card(context.table(), event.card, self.definition_id());
            data.log->effects.push_back(event.card);
            data.log->effect_targets.push_back(event.targets);
            return {};
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
    const auto observer = givm::test::with_passive_skill(play_observer_source{ &log });
    const givm::test::initialized_character_source character;
    const givm::test::named_definition_source<givm::card_definition> filler{ "PlayFiller" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        mode, setup(2), std::tuple{}, first_source, second_source, observer, character, filler);
    const auto filler_id = ids.get_id<givm::card_definition>(filler.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, {
        .cards = { filler_id, filler_id, filler_id,
            ids.get_id<givm::card_definition>(second_source.name()), ids.get_id<givm::card_definition>(first_source.name()) },
        .characters = { ids.get_id<givm::character_view>(observer.name()) }
    }, { .characters = { ids.get_id<givm::character_view>(character.name()) } });
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    const auto action = target.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.card_count() == 2);
    const auto first = action.card_id(0);
    const auto second = action.card_id(1);
    const card_targets targets{ givm::character_id{ givm::player_id{ 0 }, 0 }, givm::character_id{ givm::player_id{ 1 }, 0 } };
    REQUIRE(table[first].definition_id() == ids.get_id<givm::card_definition>(first_source.name()));
    REQUIRE(table[second].definition_id() == ids.get_id<givm::card_definition>(second_source.name()));
    CHECK(log.initial_cost_queries == 2);
    CHECK(log.quoted.empty());
    const auto random_calls = random.calls;
    if(quote_both) CHECK(action.calculate_card_cost(library, table, 1).requirement.dice_requirement.any == 3);
    CHECK(action.calculate_card_cost(library, table, 0).requirement.dice_requirement.any == 2);
    CHECK(action.card_cost(0).requirement.dice_requirement.any == 2);
    const auto expected_quotes = quote_both ? std::vector{ second, first } : std::vector{ first };
    CHECK(log.initial_cost_queries == 2);
    CHECK(log.quoted == expected_quotes);
    CHECK(action.card_cost(1).requirement.dice_requirement.any == (quote_both ? 3 : 0));
    CHECK(action.card_payment_validate(table, 0, pay(2)) == givm::card_payment_validation::valid);
    CHECK(action.card_payment_validate(table, 0, {}) == givm::card_payment_validation::requirement_mismatch);
    CHECK(action.card_payment_validate(table, 0, pay(3)) == givm::card_payment_validation::requirement_mismatch);
    CHECK(action.card_payment_validate(table, 0, pay(2, givm::elemental_dice::dendro))
        == givm::card_payment_validation::insufficient_dice);
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
        CHECK(branch_action.card_payment_validate(branch_table, index, paid) == givm::card_payment_validation::valid);
        branch_action.play_card(index, paid, targets);
        REQUIRE(advance(branch, library, branch_table, random) == givm::execution_state::action_selection);
        CHECK(log.effects == std::vector{ selected });
        CHECK(log.effect_targets == std::vector{ targets });
        CHECK(log.played_targets == std::vector{ targets });
        CHECK(log.quoted == expected_quotes);
        CHECK(log.target_validations == 0);
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

TEST_CASE("card target queries advance one step at a time and default to no targets", "[play_card][targets][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool single_target = GENERATE(false, true);
    play_log log{ .enable_payment = false };
    const playable_card_source source{ &log, "TargetedCard", 0, givm::action_speed::fast, single_target };
    const auto observer = givm::test::with_passive_skill(play_observer_source{ &log });
    const givm::test::initialized_character_source character;
    const untargeted_card_source plain{ &log };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        mode, setup(2), std::tuple{}, source, observer, character, plain);
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, {
        .cards = { ids.get_id<givm::card_definition>(plain.name()), ids.get_id<givm::card_definition>(source.name()) },
        .characters = { ids.get_id<givm::character_view>(observer.name()) }
    }, { .characters = { ids.get_id<givm::character_view>(character.name()) } });
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    const auto action = target.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.card_count() == 2);
    const auto targeted = action.card_id(0);
    const auto plain_card = action.card_id(1);
    const card_targets targets{ givm::character_id{ givm::player_id{ 0 }, 0 }, givm::character_id{ givm::player_id{ 1 }, 0 } };
    const auto first_target = std::span{ targets }.first(1);
    CHECK(action.card_cost(0).card == targeted);
    CHECK(action.card_cost(1).card == plain_card);
    const auto random_calls = random.calls;
    if(not single_target)
    {
        CHECK(action.card_targets_validate(library, table, 0, targets) == givm::target_validation::valid_complete);
        CHECK(log.target_validation_steps == std::vector<std::size_t>{ 2 });
        log.target_validation_steps.clear();
        log.checked_targets.clear();
        log.target_validations = 0;
    }
    CHECK(action.card_targets_validate(library, table, 0) == givm::target_validation::valid_incomplete);
    CHECK(log.checked_targets.back() == card_targets{});
    log.condition_met = false;
    CHECK(action.card_targets_validate(library, table, 0) == givm::target_validation::invalid);
    CHECK(action.card_targets_validate(library, table, 0, first_target) == givm::target_validation::invalid);
    log.condition_met = true;
    const std::array invalid_first{ targets[1] };
    CHECK(action.card_targets_validate(library, table, 0, invalid_first) == givm::target_validation::invalid);
    CHECK(action.card_targets_validate(library, table, 0, first_target)
        == (single_target ? givm::target_validation::valid_complete : givm::target_validation::valid_incomplete));
    CHECK(log.checked_targets.back() == card_targets{ targets[0], {} });
    if(not single_target)
    {
        const card_targets repeated{ targets[0], targets[0] };
        CHECK(action.card_targets_validate(library, table, 0, repeated)
            == givm::target_validation::invalid);
        CHECK(action.card_targets_validate(library, table, 0, targets) == givm::target_validation::valid_complete);
    }
    else
    {
        CHECK(action.card_targets_validate(library, table, 0, targets) == givm::target_validation::invalid);
    }
    const auto expected_steps = single_target ? std::vector<std::size_t>{ 0, 0, 1, 1, 1, 2 }
        : std::vector<std::size_t>{ 0, 0, 1, 1, 1, 2, 2 };
    CHECK(log.target_validation_steps == expected_steps);
    CHECK(log.target_validations == expected_steps.size());
    CHECK(random.calls == random_calls);
    CHECK(log.quoted.empty());
    CHECK(log.effects.empty());
    const auto& cost = action.calculate_card_cost(library, table, 0);
    CHECK(cost.requirement.dice_requirement.any == 0);
    CHECK(action.card_payment_validate(table, 0, {}) == givm::card_payment_validation::valid);
    CHECK(action.card_payment_validate(table, 0, pay(1)) == givm::card_payment_validation::requirement_mismatch);
    const auto selected_targets = single_target ? first_target : std::span{ targets };
    const card_targets submitted_targets{ targets[0], single_target ? givm::card_target_id{} : targets[1] };
    action.play_card(0, {}, selected_targets);
    CHECK(log.quoted == std::vector{ targeted });
    CHECK(log.target_validations == expected_steps.size());
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    CHECK(log.effects == std::vector{ targeted });
    CHECK(log.effect_targets == std::vector{ submitted_targets });
    CHECK(log.played_targets == std::vector{ submitted_targets });
    CHECK(log.target_validations == expected_steps.size());
    CHECK(log.quoted == std::vector{ targeted });
    CHECK(table.state().active_player == givm::player_id{ 0 });
    const auto next_action = target.view_in<givm::execution_state::action_selection>();
    REQUIRE(next_action.card_count() == 1);
    CHECK_FALSE(table[targeted].is_valid());
    CHECK(next_action.card_id(0) == plain_card);
    CHECK(next_action.card_cost(0).card == plain_card);
    const auto& plain_cost = next_action.calculate_card_cost(library, table, 0);
    CHECK(plain_cost.requirement.dice_requirement.any == 0);
    CHECK(plain_cost.requirement.speed == givm::action_speed::fast);
    CHECK(next_action.card_targets_validate(library, table, 0) == givm::target_validation::valid_complete);
    CHECK(next_action.card_targets_validate(library, table, 0, targets) == givm::target_validation::invalid);
    CHECK(next_action.card_payment_validate(table, 0, {}) == givm::card_payment_validation::valid);
    next_action.play_card(0, {});
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 0);
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 4);
    CHECK(table.state().active_player == givm::player_id{ 0 });
    CHECK(log.target_validations == expected_steps.size());
    CHECK(log.target_validation_steps == expected_steps);
    CHECK(log.effects == std::vector{ targeted, plain_card });
    CHECK(log.effect_targets == std::vector{ submitted_targets, card_targets{} });
}

TEST_CASE("optional targets may finish or continue and target spans ignore entries after the second", "[play_card][check][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto selected_count = GENERATE(std::size_t{ 0 }, std::size_t{ 1 }, std::size_t{ 2 }, std::size_t{ 3 });
    const bool automatic_quote = GENERATE(false, true);
    play_log log{ .enable_payment = false };
    const playable_card_source source{ &log, "OptionalTargetsCard", 0, givm::action_speed::fast, false, true };
    const auto observer = givm::test::with_passive_skill(play_observer_source{ &log });
    const givm::test::initialized_character_source character;
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        mode, setup(1), std::tuple{}, source, observer, character);
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, {
        .cards = { ids.get_id<givm::card_definition>(source.name()) },
        .characters = { ids.get_id<givm::character_view>(observer.name()) }
    }, { .characters = { ids.get_id<givm::character_view>(character.name()) } });
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    const auto action = target.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.card_count() == 1);
    const auto card = action.card_id(0);
    const std::array<givm::card_target_id, 3> targets{
        givm::character_id{ givm::player_id{ 0 }, 0 }, givm::character_id{ givm::player_id{ 1 }, 0 }, {}
    };
    const auto random_calls = random.calls;
    CHECK(action.card_targets_validate(library, table, 0) == givm::target_validation::valid_complete_or_continue);
    CHECK(action.card_targets_validate(library, table, 0, std::span{ targets }.first(1))
        == givm::target_validation::valid_complete_or_continue);
    CHECK(action.card_targets_validate(library, table, 0, std::span{ targets }.first(2))
        == givm::target_validation::valid_complete);
    CHECK(action.card_targets_validate(library, table, 0, targets) == givm::target_validation::valid_complete);
    CHECK(log.target_validation_steps == std::vector<std::size_t>{ 0, 1, 2, 2 });
    CHECK(log.checked_targets == std::vector<card_targets>{ {}, { targets[0], {} }, { targets[0], targets[1] }, { targets[0], targets[1] } });
    CHECK(random.calls == random_calls);
    CHECK(log.quoted.empty());
    CHECK(log.effects.empty());

    if(not automatic_quote)
        CHECK(action.calculate_card_cost(library, table, 0).requirement.dice_requirement.any == 0);
    if(selected_count == 0)
    {
        if(automatic_quote) action.play_card(library, table, 0, {});
        else action.play_card(0, {});
    }
    else
    {
        const auto selected = std::span{ targets }.first(selected_count);
        if(automatic_quote) action.play_card(library, table, 0, {}, selected);
        else action.play_card(0, {}, selected);
    }
    CHECK(log.target_validations == 4);
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    const card_targets expected_targets{
        selected_count > 0 ? targets[0] : givm::card_target_id{},
        selected_count > 1 ? targets[1] : givm::card_target_id{}
    };
    CHECK(log.effects == std::vector{ card });
    CHECK(log.effect_targets == std::vector{ expected_targets });
    CHECK(log.played_targets == std::vector{ expected_targets });
    CHECK(log.target_validations == 4);
    CHECK(target.view_in<givm::execution_state::action_selection>().card_count() == 0);
}

TEST_CASE("card payment and broadcasts resume in order after removal even when its effect is countered", "[play_card][onpay][broadcast][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool countered = GENERATE(false, true);
    const bool automatic_quote = GENERATE(false, true);
    play_log log{ .nested = true, .cancel_effect = countered };
    const playable_card_source source{ &log, "NestedPlayedCard", 1, givm::action_speed::combat };
    const auto observer = givm::test::with_passive_skill(play_observer_source{ &log });
    const givm::test::initialized_character_source character;
    const givm::test::named_definition_source<givm::card_definition> filler{ "NestedFiller" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        mode, setup(1), std::tuple{}, source, observer, character, filler);
    const auto card_definition = ids.get_id<givm::card_definition>(source.name());
    const auto filler_definition = ids.get_id<givm::card_definition>(filler.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 }, .hand_limit = 4 },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 }, .hand_limit = 4 } };
    load_deck(table, library, {
        .cards = { filler_definition, filler_definition, filler_definition, filler_definition, card_definition },
        .characters = { ids.get_id<givm::character_view>(observer.name()) }
    }, { .characters = { ids.get_id<givm::character_view>(character.name()) } });
    givm::executor target;
    target.enter_entry(library);
    counting_random random;
    REQUIRE(advance(target, library, table, random) == givm::execution_state::action_selection);
    const auto action = target.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.card_count() == 1);
    const auto card = action.card_id(0);
    const card_targets targets{ givm::character_id{ givm::player_id{ 0 }, 0 }, givm::character_id{ givm::player_id{ 1 }, 0 } };
    log.selected = card;
    log.record = true;
    if(automatic_quote) action.play_card(library, table, 0, pay(1), targets);
    else
    {
        action.calculate_card_cost(library, table, 0);
        action.play_card(0, pay(1), targets);
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
        CHECK(log.target_validations == 0);
        CHECK(current_table[givm::player_id{ 0 }].state().dice.total() == 3);
        CHECK(current_table[givm::player_id{ 0 }].hand_card_count() == draws);
        CHECK(current_table[givm::player_id{ 0 }].deck_card_count() == 4 - draws);
        CHECK(current_table.state().active_player == givm::player_id{ 1 });
    };
    resume(target, table);
    resume(copied_target, copied_table);
}
