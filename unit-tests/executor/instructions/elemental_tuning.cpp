#include <array>
#include <concepts>
#include <cstdint>
#include <string_view>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../../table/test_definition_library.hpp"

namespace
{
    struct tuning_opponent_source
    {
        using definition_category = givm::character_view;
        struct definition_type {};

        std::string_view name() const noexcept { return "TuningOpponent"; }
        definition_type compile(givm::definition_compile_context&) const { return {}; }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
    };

    struct tuning_log
    {
        std::uint32_t quotes = 0;
        std::uint32_t modifications = 0;
        std::uint32_t completions = 0;
        std::uint32_t resource_events = 0;
        std::uint32_t removed_card_responses = 0;
        givm::elemental_dice result = givm::elemental_dice::pyro;
        bool nested = false;
    };

    struct tuning_card_source
    {
        using definition_category = givm::card_definition;
        struct definition_type { tuning_log* log; bool allowed; bool empowered; };
        tuning_log* log;
        std::string_view source_name;
        bool allowed = true;
        bool empowered = false;

        std::string_view name() const noexcept { return source_name; }
        definition_type compile(givm::definition_compile_context&) const { return { log, allowed, empowered }; }
        static givm::card_state query(const definition_type& data, const givm::card_initial_state&)
        {
            return { .cost = { .dice_requirement = { .any = 2 }, .speed = givm::action_speed::fast },
                .elemental_tuning_allowed = data.allowed };
        }
        static givm::program_entry handle(const definition_type& data, const givm::hand_card_view& self,
            givm::elemental_tuning_modification& event, givm::handle_context&)
        {
            if(event.card == self.id())
            {
                REQUIRE(self.is_valid());
                CHECK(self.state().cost.dice_requirement.any == 2);
                if(data.empowered) event.to = givm::elemental_dice::omni;
            }
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::hand_card_view& self,
            givm::elemental_tuning_completed& event, givm::handle_context&)
        {
            if(event.card == self.id()) ++data.log->removed_card_responses;
            return {};
        }
    };

    struct tuning_character_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            tuning_log* log;
            givm::element element;
            givm::program_entry pause;
            givm::program_entry payment;
        };
        tuning_log* log;
        givm::element element = givm::element::pyro;

        std::string_view name() const noexcept { return "TuningCharacter"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, element,
                context.add_program(std::tuple{ givm::replace_cards{ .player = givm::player_id{ 0 } } }),
                context.add_program(std::tuple{ givm::draw_cards{ .count = 1 } }) };
        }
        static givm::character_state query(const definition_type& data, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10, .element = data.element };
        }
        static givm::program_entry handle(const definition_type&, const givm::character_view&,
            givm::dice_roll_preparation& event, givm::handle_context&)
        {
            for(auto& dice : event.fixed_dice)
            {
                dice[givm::elemental_dice::cryo] = 1;
                dice[givm::elemental_dice::pyro] = 1;
                dice[givm::elemental_dice::omni] = 1;
            }
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::cost_of_card& event, givm::handle_context& context)
        {
            ++data.log->quotes;
            --event.requirement.dice_requirement.any;
            return context.invoke(givm::substack_t{}, data.payment);
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::elemental_tuning_modification& event, givm::handle_context& context)
        {
            ++data.log->modifications;
            CHECK(context.table()[event.card].is_valid());
            CHECK(event.from == givm::elemental_dice::cryo);
            CHECK(context.table()[event.card.player_id].state().dice[givm::elemental_dice::cryo] == 1);
            return data.log->nested ? context.invoke(data.pause) : givm::program_entry{};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::elemental_tuning_completed& event, givm::handle_context& context)
        {
            ++data.log->completions;
            data.log->result = event.to;
            const auto card = context.table()[event.card];
            CHECK_FALSE(card.is_valid());
            CHECK(card.state().cost.dice_requirement.any == 2);
            CHECK(context.table()[event.card.player_id].state().dice[givm::elemental_dice::cryo] == 0);
            return data.log->nested ? context.invoke(data.pause) : givm::program_entry{};
        }
        template<class TEvent>
            requires(std::same_as<TEvent, givm::dice_added> || std::same_as<TEvent, givm::dice_removed>
                || std::same_as<TEvent, givm::dice_converted> || std::same_as<TEvent, givm::card_played>
                || std::same_as<TEvent, givm::hand_card_discarded>
                || std::same_as<TEvent, givm::deck_card_discarded>)
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            TEvent&, givm::handle_context&)
        {
            ++data.log->resource_events;
            return {};
        }
    };

    auto setup()
    {
        return std::tuple{
            givm::draw_cards{ .count = 2 },
            givm::start_dice_roll_phase{ .count = 3, .reroll_count = { 0, 0 } },
            givm::begin_action{}
        };
    }

    givm::execution_state advance(givm::executor& execution, const givm::definition_library& library, givm::table& table)
    {
        auto random = []() -> std::uint32_t { return 0; };
        auto state = execution.step(library, table, random);
        while(state == givm::execution_state::active_character_changed || state == givm::execution_state::action_started)
            state = execution.step(library, table, random);
        return state;
    }
}

TEST_CASE("elemental tuning shares card candidates and validates card attributes and individual dice", "[elemental_tuning]")
{
    const auto element = GENERATE(givm::element::pyro, givm::element::none);
    tuning_log log;
    const auto character = givm::test::with_passive_skill(tuning_character_source{ &log, element });
    const tuning_card_source allowed{ &log, "TunableCard", true, true };
    const tuning_card_source blocked{ &log, "UntunableCard", false };
    const tuning_opponent_source opponent;
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        givm::compile_mode::normal, setup(), std::tuple{}, character, allowed, blocked, opponent);
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, {
        .cards = { ids.get_id<givm::card_definition>(blocked.name()), ids.get_id<givm::card_definition>(allowed.name()) },
        .characters = { ids.get_id<givm::character_view>(character.name()) }
    }, { .characters = { ids.get_id<givm::character_view>(opponent.name()) } });
    givm::executor execution;
    execution.enter_entry(library);
    REQUIRE(advance(execution, library, table) == givm::execution_state::action_selection);
    const auto action = execution.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.card_count() == 2);
    CHECK(action.elemental_tuning_card_validate(table, 0));
    CHECK_FALSE(action.elemental_tuning_card_validate(table, 1));
    using result = givm::elemental_tuning_dice_validation;
    CHECK(action.elemental_tuning_dice_validate(table, givm::elemental_dice::omni) == result::omni_not_allowed);
    if(element == givm::element::none)
    {
        CHECK(action.elemental_tuning_dice_validate(table, givm::elemental_dice::cryo) == result::missing_character_element);
    }
    else
    {
        CHECK(action.elemental_tuning_dice_validate(table, givm::elemental_dice::pyro) == result::same_element);
        CHECK(action.elemental_tuning_dice_validate(table, givm::elemental_dice::hydro) == result::insufficient_dice);
        CHECK(action.elemental_tuning_dice_validate(table, givm::elemental_dice::cryo) == result::valid);
    }
    CHECK(log.quotes == 0);
    CHECK(log.modifications == 0);
}

TEST_CASE("elemental tuning converts one die and resumes both broadcasts without paying cached card costs", "[elemental_tuning][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool empowered = GENERATE(false, true);
    const bool nested = GENERATE(false, true);
    const bool quoted = GENERATE(false, true);
    tuning_log log{ .nested = nested };
    const auto character = givm::test::with_passive_skill(tuning_character_source{ &log });
    const tuning_card_source card{ &log, "SelectedTuningCard", true, empowered };
    const givm::test::named_definition_source<givm::card_definition> filler{ "TuningFiller" };
    const tuning_opponent_source opponent;
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        mode, setup(), std::tuple{}, character, card, filler, opponent);
    const auto card_definition = ids.get_id<givm::card_definition>(card.name());
    const auto filler_definition = ids.get_id<givm::card_definition>(filler.name());
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, {
        .cards = { filler_definition, filler_definition, card_definition },
        .characters = { ids.get_id<givm::character_view>(character.name()) }
    }, { .characters = { ids.get_id<givm::character_view>(opponent.name()) } });
    givm::executor execution;
    execution.enter_entry(library);
    REQUIRE(advance(execution, library, table) == givm::execution_state::action_selection);
    const auto action = execution.view_in<givm::execution_state::action_selection>();
    const auto selected_card = action.card_id(0);
    if(quoted)
    {
        CHECK(action.calculate_card_cost(library, table, 0).requirement.dice_requirement.any == 1);
        CHECK(table[selected_card].state().cost.dice_requirement.any == 2);
    }
    action.elemental_tuning(0, givm::elemental_dice::cryo);
    if(nested)
    {
        REQUIRE(advance(execution, library, table) == givm::execution_state::card_selection);
        CHECK(table[selected_card].is_valid());
        CHECK(log.modifications == 1);
        CHECK(log.completions == 0);
        execution.view_in<givm::execution_state::card_selection>().select({});
        REQUIRE(advance(execution, library, table) == givm::execution_state::card_selection);
        CHECK_FALSE(table[selected_card].is_valid());
        CHECK(log.completions == 1);
        execution.view_in<givm::execution_state::card_selection>().select({});
    }
    REQUIRE(advance(execution, library, table) == givm::execution_state::action_selection);
    CHECK(table.state().active_player == givm::player_id{ 0 });
    CHECK_FALSE(table[selected_card].is_valid());
    CHECK(table[selected_card].definition_id() == card_definition);
    CHECK(table[selected_card].state().cost.dice_requirement.any == 2);
    CHECK(log.quotes == (quoted ? 1 : 0));
    CHECK(log.modifications == 1);
    CHECK(log.completions == 1);
    CHECK(log.resource_events == 0);
    CHECK(log.removed_card_responses == 0);
    CHECK(log.result == (empowered ? givm::elemental_dice::omni : givm::elemental_dice::pyro));
    const auto& dice = table[givm::player_id{ 0 }].state().dice;
    CHECK(dice.total() == 3);
    CHECK(dice[givm::elemental_dice::cryo] == 0);
    CHECK(dice[givm::elemental_dice::pyro] == (empowered ? 1 : 2));
    CHECK(dice[givm::elemental_dice::omni] == (empowered ? 2 : 1));
    const auto next = execution.view_in<givm::execution_state::action_selection>();
    REQUIRE(next.card_count() == 1);
    CHECK(table[next.card_id(0)].definition_id() == filler_definition);
}
