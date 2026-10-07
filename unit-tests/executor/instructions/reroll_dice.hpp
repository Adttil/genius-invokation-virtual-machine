#include <concepts>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/executor.hpp>

#include "../../test_source_library.hpp"
#include "../test_character_source.hpp"

namespace givm_test::executor_instructions::reroll_dice
{
namespace
{
    struct reroll_log
    {
        bool dynamic = false;
        givm::relative_player target = givm::relative_player::self;
        std::uint32_t count = 2;
        std::uint32_t dice_notifications = 0;
        std::vector<int> order;
    };

    struct reroll_source
    {
        static constexpr auto category = givm::definition_category::support;
        struct definition_type
        {
            reroll_log* log;
            givm::normal_effect effect;
        };
        reroll_log* log;

        constexpr std::string_view name() const { return "RerollObserver"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto command = log->dynamic ? givm::reroll_dice{}
                : givm::reroll_dice{ .player = log->target, .reroll_count = log->count };
            return { log, context.add_normal_effect(std::tuple{ command, givm::set_support_state{} }) };
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::round_started&, givm::handle_context<givm::support_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity();
            const auto player = data.log->target == givm::relative_player::self
                ? self.player().id() : givm::player_id{ 1 - self.player().id().index() };
            const givm::set_support_state_input state{ self.id(), { .count = 7 } };
            if(data.log->dynamic)
                return context.invoke(data.effect, givm::reroll_dice_input{ player, data.log->count }, state);
            return context.invoke(data.effect, state);
        }
        template<class TEvent>
            requires(std::same_as<TEvent, givm::dice_added> || std::same_as<TEvent, givm::dice_removed>
                || std::same_as<TEvent, givm::dice_converted> || std::same_as<TEvent, givm::dice_roll_preparation>)
        static givm::normal_effect handle(const definition_type& data,
            TEvent&, givm::handle_context<givm::support_view>&, std::uint32_t = 0)
        {
            ++data.log->dice_notifications;
            return {};
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::card_played&, givm::handle_context<givm::support_view>&, std::uint32_t = 0)
        {
            data.log->order.push_back(3);
            return {};
        }
    };

    struct reroll_card_source
    {
        static constexpr auto category = givm::definition_category::card;
        struct definition_type
        {
            reroll_log* log;
            givm::normal_effect effect;
        };
        reroll_log* log;
        constexpr std::string_view name() const { return "RerollCard"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_normal_effect(std::tuple{ givm::reroll_dice{} }) };
        }
        static givm::card_state query(const definition_type&, const givm::card_initial_state&)
        {
            return { .cost = { .speed = givm::action_speed::fast } };
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::this_card_play&, givm::handle_context<givm::hand_card_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity();
            data.log->order.push_back(0);
            return context.invoke(data.effect, givm::reroll_dice_input{ self.player().id(), 2 });
        }
    };

    struct reroll_random
    {
        std::uint32_t calls = 0;
        std::uint32_t operator()()
        {
            ++calls;
            return 0u | (1u << 3) | (2u << 6) | (3u << 9) | (4u << 12)
                | (5u << 15) | (6u << 18) | (7u << 21) | (0u << 24) | (1u << 27);
        }
    };

    givm::dice_counts initial_dice()
    {
        givm::dice_counts result;
        result[givm::elemental_dice::pyro] = 3;
        result[givm::elemental_dice::hydro] = 2;
        result[givm::elemental_dice::omni] = 1;
        return result;
    }

    template<class TRandom>
    givm::execution_state advance(givm_test::executor_driver& executor, const givm::definition_library& library,
        givm::table& table, TRandom& random)
    {
        for(;;)
        {
            const auto state = executor.advance(library, table, random);
            if(state == givm::execution_state::dice_reroll_selection
                || state == givm::execution_state::action_selection || state == givm::execution_state::finished)
                return state;
        }
    }
}

TEST_CASE("single-player rerolls preserve partial choices and prefetched randomness across copies", "[reroll_dice]")
{
    reroll_log log{ .dynamic = GENERATE(false, true),
        .target = GENERATE(givm::relative_player::self, givm::relative_player::opponent) };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const reroll_source source{ &log };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(source));
    const std::array support_names{ source.name() };
    const givm::test::initialization_skill_source initialization{
        [](givm::definition_compile_context& context)
        {
            return std::tuple{ givm::add_support{ .player = givm::relative_player::self,
                .definition = context.resolve_id<givm::definition_category::support>("RerollObserver") } };
        }, {}, support_names };
    const givm::test::initialization_character_source character;
    REQUIRE(sources.add(initialization, character));
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources,
        std::tuple{ givm::start_battle{}, givm::settle{}, givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, mode));
    givm::table table{ { .round_number = 1, .active_player = givm::player_id{ 0 }, .self_player = givm::player_id{ 1 } },
        { .dice = initial_dice() }, { .dice = initial_dice() } };
    load_deck(table, library, {}, { .characters = { ids.get_id<givm::definition_category::character>(character.name()) } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    reroll_random random;
    REQUIRE(advance(executor, library, table, random) == givm::execution_state::dice_reroll_selection);
    const auto player = givm::player_id{ log.target == givm::relative_player::self ? 1u : 0u };
    const auto other = givm::player_id{ 1 - player.index() };
    const auto prefetched_calls = random.calls;
    CHECK(prefetched_calls > 0);
    for(int copy = 0; copy < 2; ++copy)
    {
        auto branch = executor;
        auto branch_table = table;
        auto no_random = [] { FAIL("rerolling a copied selection must use the existing random results"); return 0u; };
        const auto first = branch.view_in<givm::execution_state::dice_reroll_selection>();
        CHECK(first.player() == player);
        givm::dice_counts invalid;
        invalid[givm::elemental_dice::cryo] = 1;
        givm::dice_counts selection;
        selection[givm::elemental_dice::pyro] = 2;
        selection[givm::elemental_dice::omni] = 1;
        CHECK_FALSE(first.selection_validate(branch_table, invalid));
        CHECK(first.selection_validate(branch_table, selection));
        CHECK(first.selection_validate(branch_table, {}));
        CHECK(branch_table[player].state().dice == initial_dice());
        branch.submitted(first.select(library, branch_table, no_random, selection));
        REQUIRE(advance(branch, library, branch_table, no_random) == givm::execution_state::dice_reroll_selection);
        givm::dice_counts expected;
        expected[givm::elemental_dice::cryo] = 1;
        expected[givm::elemental_dice::hydro] = 3;
        expected[givm::elemental_dice::pyro] = 2;
        CHECK(branch_table[player].state().dice == expected);
        CHECK(branch_table[other].state().dice == initial_dice());
        const auto second = branch.view_in<givm::execution_state::dice_reroll_selection>();
        selection = {};
        selection[givm::elemental_dice::hydro] = 2;
        REQUIRE(second.selection_validate(branch_table, selection));
        branch.submitted(second.select(library, branch_table, no_random, selection));
        REQUIRE(advance(branch, library, branch_table, no_random) == givm::execution_state::finished);
        expected[givm::elemental_dice::hydro] -= 2;
        expected[givm::elemental_dice::electro] = 1;
        expected[givm::elemental_dice::geo] = 1;
        CHECK(branch_table[player].state().dice == expected);
        CHECK(branch_table[other].state().dice == initial_dice());
        for(const auto support : branch_table[givm::player_id{ 1 }].supports()) CHECK(support.state().count == 7);
    }
    CHECK(random.calls == prefetched_calls);
    CHECK(log.dice_notifications == 0);
    CHECK(table[player].state().dice == initial_dice());
}

TEST_CASE("single-player rerolls skip empty pools and zero counts and can stop without rerolling", "[reroll_dice]")
{
    const auto scenario = GENERATE(0, 1, 2);
    reroll_log log{ .dynamic = GENERATE(false, true), .count = scenario == 0 ? 0u : 2u };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const reroll_source source{ &log };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(source));
    const std::array support_names{ source.name() };
    const givm::test::initialization_skill_source initialization{
        [](givm::definition_compile_context& context)
        {
            return std::tuple{ givm::add_support{ .player = givm::relative_player::self,
                .definition = context.resolve_id<givm::definition_category::support>("RerollObserver") } };
        }, {}, support_names };
    const givm::test::initialization_character_source character;
    REQUIRE(sources.add(initialization, character));
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources,
        std::tuple{ givm::start_battle{}, givm::settle{}, givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, mode));
    const auto initial = scenario == 1 ? givm::dice_counts{} : initial_dice();
    givm::table table{ { .round_number = 1, .self_player = givm::player_id{ 1 } }, { .dice = initial_dice() }, { .dice = initial } };
    load_deck(table, library, {}, { .characters = { ids.get_id<givm::definition_category::character>(character.name()) } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    reroll_random random;
    if(scenario == 2)
    {
        REQUIRE(advance(executor, library, table, random) == givm::execution_state::dice_reroll_selection);
        executor.submitted(executor.view_in<givm::execution_state::dice_reroll_selection>().select(library, table, random, {}));
    }
    REQUIRE(advance(executor, library, table, random) == givm::execution_state::finished);
    if(scenario != 2) CHECK(random.calls == 0);
    CHECK(table[givm::player_id{ 0 }].state().dice == initial_dice());
    CHECK(table[givm::player_id{ 1 }].state().dice == initial);
    CHECK(log.dice_notifications == 0);
    for(const auto support : table[givm::player_id{ 1 }].supports()) CHECK(support.state().count == 7);
}

TEST_CASE("a played card finishes both rerolls before the card-played notification", "[reroll_dice][play_card]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    reroll_log log;
    const reroll_source observer{ &log };
    const reroll_card_source card{ &log };
    const givm::test::initialized_character_source character;
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(observer));
    REQUIRE(sources.add(card));
    REQUIRE(sources.add(character));
    const std::array card_names{ card.name() };
    const std::array support_names{ observer.name() };
    const givm::test::initialization_skill_source initialization{
        [](givm::definition_compile_context& context)
        {
            return std::tuple{
                givm::add_support{ .player = givm::relative_player::self,
                    .definition = context.resolve_id<givm::definition_category::support>("RerollObserver") },
                givm::create_hand_card{ .player = givm::relative_player::self,
                    .definition = context.resolve_id<givm::definition_category::card>("RerollCard") } };
        }, card_names, support_names };
    const givm::test::initialization_character_source driver;
    REQUIRE(sources.add(initialization, driver));
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources,
        std::tuple{ givm::start_battle{}, givm::settle{}, givm::begin_action{} }, std::tuple{}, mode));
    givm::table table{ { .round_number = 1, .self_player = givm::player_id{ 0 } },
        { .dice = initial_dice(), .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .dice = initial_dice(), .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    const givm::linked_deck deck{ .characters = { ids.get_id<givm::definition_category::character>(character.name()) } };
    load_deck(table, library, { .characters = { ids.get_id<givm::definition_category::character>(driver.name()) } }, deck);
    givm_test::executor_driver executor;
    executor.start(library, table);
    reroll_random random;
    REQUIRE(advance(executor, library, table, random) == givm::execution_state::action_selection);
    const auto action = executor.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.card_count() == 1);
    const auto quote_1 = action.calculate_card_cost(library, table, 0);
    executor.submitted(action.play_card_with_cached_cost(library, table, random, quote_1, {}));
    for(int index = 1; index <= 2; ++index)
    {
        REQUIRE(advance(executor, library, table, random) == givm::execution_state::dice_reroll_selection);
        CHECK(log.order.size() == static_cast<std::size_t>(index));
        log.order.push_back(index);
        const auto view = executor.view_in<givm::execution_state::dice_reroll_selection>();
        CHECK(view.player() == givm::player_id{ 0 });
        givm::dice_counts selected;
        selected[givm::elemental_dice::pyro] = 1;
        REQUIRE(view.selection_validate(table, selected));
        executor.submitted(view.select(library, table, random, selected));
    }
    REQUIRE(advance(executor, library, table, random) == givm::execution_state::action_selection);
    CHECK(log.order == std::vector<int>{ 0, 1, 2, 3 });
    CHECK(log.dice_notifications == 0);
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 0);
    CHECK(table[givm::player_id{ 1 }].state().dice == initial_dice());
    CHECK(table.state().active_player == givm::player_id{ 0 });
}
}
