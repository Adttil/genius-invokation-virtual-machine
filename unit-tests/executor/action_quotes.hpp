#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../table/test_definition_library.hpp"
#include "test_character_source.hpp"

namespace givm_test::executor::action_quotes
{
namespace
{
    constexpr givm::player_id player{ 0 };
    constexpr givm::character_id original{ player, 0 };
    constexpr givm::character_id next{ player, 1 };
    using targets_t = std::array<givm::card_target_id, 2>;

    givm::dice_counts dice(std::uint8_t fire, std::uint8_t water, std::uint8_t omni)
    {
        givm::dice_counts result;
        result[givm::elemental_dice::pyro] = fire;
        result[givm::elemental_dice::hydro] = water;
        result[givm::elemental_dice::omni] = omni;
        return result;
    }

    struct payment_log
    {
        std::vector<targets_t> quoted;
        std::vector<std::string> order;
        std::vector<givm::dice_counts> removed;
        std::vector<std::pair<std::uint32_t, std::uint32_t>> energy;
        targets_t effect_targets{};
        givm::dice_counts effect_dice;
        std::uint32_t effect_energy = 0;
    };

    struct card_source
    {
        static constexpr auto category = givm::definition_category::card;
        using definition_type = payment_log*;
        payment_log* log;
        std::string_view name() const { return "QuotedCard"; }
        definition_type compile(givm::definition_compile_context&) const { return log; }
        static givm::target_validation query(definition_type, const givm::card_target_validation&)
        {
            return givm::target_validation::valid_complete_or_continue;
        }
        static givm::normal_effect handle(definition_type log, givm::this_card_play& event,
            givm::handle_context<givm::hand_card_view>& context, std::uint32_t = 0)
        {
            log->order.push_back("effect");
            log->effect_targets = event.targets;
            log->effect_dice = context.table()[player].state().dice;
            log->effect_energy = context.table()[original].state().energy;
            return {};
        }
    };

    struct payment_source
    {
        static constexpr auto category = givm::definition_category::character;
        struct definition_type
        {
            payment_log* log;
            bool target_prices;
            bool terminal;
            std::uint8_t extra_fire;
            givm::preview_effect fee;
            givm::normal_effect consume;
            givm::preview_effect mark;
            givm::preview_effect end;
        };
        payment_log* log;
        bool target_prices = false;
        bool terminal = false;
        std::uint8_t extra_fire = 0;
        std::string_view name() const { return "QuotedPayment"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, target_prices, terminal, extra_fire,
                context.add_preview_effect(givm::replace_cards{ player }, givm::discard_hand_card{},
                    givm::set_active_character{}, givm::set_energy{}, givm::add_dice{}, givm::return_response{}),
                context.add_normal_effect(givm::remove_dice{}),
                context.add_preview_effect(givm::set_energy{}),
                context.add_preview_effect(givm::replace_cards{ player }, givm::end_game{ givm::game_result::both_loss }) };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .max_energy = 3, .health = 10, .energy = 3 };
        }
        static givm::preview_effect handle(const definition_type& data, givm::cost_of_card& event,
            givm::handle_context<givm::skill_view, givm::event_category::preview>& context)
        {
            if(context.entity().character().id() != original) return {};
            data.log->quoted.push_back(event.targets);
            if(data.target_prices)
            {
                const auto target = event.targets[0].template get<givm::entity_category::character>();
                event.requirement.dice_requirement.any = static_cast<std::uint8_t>(target.index() + 1);
                return context.invoke(data.mark,
                    givm::set_energy_input{ original, static_cast<std::uint32_t>(target.index() + 1) });
            }
            event.requirement.dice_requirement.any = 3;
            event.requirement.energy = 3;
            if(data.terminal) return context.invoke(data.end);
            const auto card = [&]
            {
                for(auto candidate : context.entity().player().hand_cards())
                    if(candidate.id() != event.card) return candidate.id();
                return givm::hand_card_id{};
            }();
            const std::array discarded{ card };
            return context.invoke(data.fee,
                givm::discard_hand_card_input{ discarded }, givm::set_active_character_input{ next },
                givm::set_energy_input{ original, 1 }, givm::add_dice_input{ player, dice(data.extra_fire, 0, 0) },
                givm::return_response_input{ 97 });
        }
        static givm::normal_effect handle(const definition_type& data, givm::hand_card_discarded&,
            givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            if(context.entity().character().id() != original) return {};
            data.log->order.push_back("discard-chain");
            return context.invoke(data.consume, givm::remove_dice_input{ player, dice(1, 1, 0) });
        }
        static givm::normal_effect handle(const definition_type& data, givm::dice_removed& event,
            givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            if(context.entity().character().id() != original) return {};
            data.log->order.push_back("dice");
            data.log->removed.push_back(event.dice);
            return {};
        }
        static givm::normal_effect handle(const definition_type& data, givm::energy_changed& event,
            givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            if(context.entity().character().id() == original)
                data.log->energy.emplace_back(event.previous, event.current);
            return {};
        }
        static givm::normal_effect handle(const definition_type& data, givm::card_played&,
            givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            if(context.entity().character().id() == original) data.log->order.push_back("played");
            return {};
        }
    };

    struct zero_random
    {
        std::size_t calls = 0;
        std::uint32_t operator()() { ++calls; return 0; }
    };

    givm::execution_state advance(givm::executor& execution, const givm::definition_library& library,
        givm::table& table, zero_random& random, givm::execution_state state)
    {
        for(;;)
        {
            if(state == givm::execution_state::active_character_changed)
                state = execution.view_in<givm::execution_state::active_character_changed>().resume(library, table, random);
            else if(state == givm::execution_state::action_started)
                state = execution.view_in<givm::execution_state::action_started>().resume(library, table, random);
            else return state;
        }
    }

    givm::table make_table(const givm::definition_library& library, const givm::issued_id_map& ids, givm::dice_counts initial)
    {
        givm::table result{ { .self_player = player }, { .dice = initial, .active_character = original },
            { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
        const auto owner = ids.get_id<givm::definition_category::character>("QuotedPayment");
        load_deck(result, library, { .cards = { ids.get_id<givm::definition_category::card>("Token"), ids.get_id<givm::definition_category::card>("QuotedCard") },
            .characters = { owner, owner } }, { .characters = { owner } });
        return result;
    }
}

TEST_CASE("target quotations retain distinct costs and inputs in copied action windows", "[action-quotes][payment]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    payment_log log;
    const auto owner = givm::test::with_passive_skill(payment_source{ &log, true });
    const givm::test::named_definition_source<givm::definition_category::card> token{ "Token" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::draw_cards{ .position = 0, .count = 2 }, givm::settle{}, givm::begin_action{} },
        std::tuple{}, owner, card_source{ &log }, token);
    auto table = make_table(library, ids, dice(0, 0, 3));
    givm::executor execution;
    zero_random random;
    REQUIRE(advance(execution, library, table, random, execution.start(library, table).resume(library, table, random))
        == givm::execution_state::action_selection);
    const auto action = execution.view_in<givm::execution_state::action_selection>();
    auto unquoted_copy = execution;
    const targets_t first_targets{ original, {} }, second_targets{ next, original };
    const auto first_quote = action.calculate_card_cost(library, table, 0, first_targets);
    const auto second_quote = action.calculate_card_cost(library, table, 0, second_targets);
    CHECK(first_quote != second_quote);
    CHECK(action.card_cost(first_quote).requirement.dice_requirement.any == 1);
    CHECK(action.card_cost(second_quote).requirement.dice_requirement.any == 2);
    CHECK(log.quoted == std::vector<targets_t>{ first_targets, second_targets });
    CHECK(log.order.empty());
    CHECK(random.calls == 0);
    CHECK(table[original].state().energy == 3);
    for(const bool second : { false, true })
    {
        auto branch = execution;
        auto branch_table = table;
        log.order.clear();
        const auto quote = second ? second_quote : first_quote;
        REQUIRE(advance(branch, library, branch_table, random,
            branch.view_in<givm::execution_state::action_selection>().play_card_with_cached_cost(
                library, branch_table, random, quote, dice(0, 0, second ? 2 : 1))) == givm::execution_state::action_selection);
        CHECK(log.effect_targets == (second ? second_targets : first_targets));
        CHECK(log.effect_energy == (second ? 2 : 1));
        CHECK(log.effect_dice == dice(0, 0, second ? 1 : 2));
        CHECK(log.order == std::vector<std::string>{ "effect", "dice", "played" });
        CHECK(log.quoted.size() == 2);
#ifndef NDEBUG
        REQUIRE_THROWS_AS(branch.view_in<givm::execution_state::action_selection>().card_cost(quote),
            givm::view_input_error<givm::action_cost_cache_error>);
#endif
    }
    CHECK(table[original].state().energy == 3);
    CHECK(table[player].state().dice == dice(0, 0, 3));
#ifndef NDEBUG
    REQUIRE_THROWS_AS(action.calculate_card_cost(library, table, 0, std::span<const givm::card_target_id>{ first_targets }.first(1)),
        givm::view_input_error<givm::action_cost_cache_error>);
    const auto other_action = unquoted_copy.view_in<givm::execution_state::action_selection>();
    REQUIRE_THROWS_AS(other_action.card_cost(first_quote), givm::view_input_error<givm::action_cost_cache_error>);
    const auto other_quote = other_action.calculate_card_cost(library, table, 0, second_targets);
    REQUIRE_THROWS_AS(other_action.card_cost(first_quote), givm::view_input_error<givm::action_cost_cache_error>);
    REQUIRE_THROWS_AS(action.card_cost(other_quote), givm::view_input_error<givm::action_cost_cache_error>);
#endif
}

TEST_CASE("payment settles discard chains before saturating selected colors and the original character energy", "[action-quotes][payment][onpay]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto extra = GENERATE(std::uint8_t{ 0 }, std::uint8_t{ 2 });
    payment_log log;
    const auto owner = givm::test::with_passive_skill(payment_source{ &log, false, false, extra });
    const givm::test::named_definition_source<givm::definition_category::card> token{ "Token" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::draw_cards{ .position = 0, .count = 2 }, givm::settle{}, givm::begin_action{} },
        std::tuple{}, owner, card_source{ &log }, token);
    auto table = make_table(library, ids, dice(2, 1, 1));
    givm::executor execution;
    zero_random random;
    REQUIRE(advance(execution, library, table, random, execution.start(library, table).resume(library, table, random))
        == givm::execution_state::action_selection);
    const auto action = execution.view_in<givm::execution_state::action_selection>();
    const auto quote = action.calculate_card_cost(library, table, 0);
    REQUIRE(action.card_payment_validate(table, quote, dice(2, 1, 0)) == givm::card_payment_validation::valid);
    REQUIRE(advance(execution, library, table, random,
        action.play_card_with_cached_cost(library, table, random, quote, dice(2, 1, 0))) == givm::execution_state::card_selection);
    CHECK(table[player].hand_card_count() == 1);
    CHECK(table[player].state().dice == dice(2, 1, 1));
    auto copy = execution;
    auto copy_table = table;
    for(bool copied : { false, true })
    {
        auto& current = copied ? copy : execution;
        auto& current_table = copied ? copy_table : table;
        log.order.clear(); log.removed.clear(); log.energy.clear();
        REQUIRE(advance(current, library, current_table, random,
            current.view_in<givm::execution_state::card_selection>().select(library, current_table, random, {}))
            == givm::execution_state::action_selection);
        CHECK(log.order == std::vector<std::string>{ "discard-chain", "dice", "effect", "dice", "played" });
        CHECK(log.removed == std::vector<givm::dice_counts>{ dice(1, 1, 0), dice(extra ? 2 : 1, 0, 0) });
        CHECK(log.effect_dice == dice(extra ? 1 : 0, 0, 1));
        CHECK(log.effect_energy == 0);
        CHECK(log.energy == std::vector<std::pair<std::uint32_t, std::uint32_t>>{ { 3, 1 }, { 1, 0 } });
        CHECK(current_table[player].state().active_character == next);
        CHECK(current_table[next].state().energy == 3);
        CHECK(log.quoted.size() == 1);
    }
}

TEST_CASE("a terminal cached fee stops before payment and the selected card effect", "[action-quotes][payment][terminal]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    payment_log log;
    const auto owner = givm::test::with_passive_skill(payment_source{ &log, false, true });
    const givm::test::named_definition_source<givm::definition_category::card> token{ "Token" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::draw_cards{ .position = 0, .count = 2 }, givm::settle{}, givm::begin_action{} },
        std::tuple{}, owner, card_source{ &log }, token);
    auto table = make_table(library, ids, dice(2, 1, 1));
    givm::executor execution;
    zero_random random;
    REQUIRE(advance(execution, library, table, random, execution.start(library, table).resume(library, table, random))
        == givm::execution_state::action_selection);
    REQUIRE(advance(execution, library, table, random,
        execution.view_in<givm::execution_state::action_selection>().play_card(library, table, random, 0, dice(2, 1, 0)))
        == givm::execution_state::card_selection);
    REQUIRE(execution.view_in<givm::execution_state::card_selection>().select(library, table, random, {})
        == givm::execution_state::finished);
    CHECK(table[player].state().dice == dice(2, 1, 1));
    CHECK(table[original].state().energy == 3);
    CHECK(log.order.empty());
}
}
