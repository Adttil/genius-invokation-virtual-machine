#include <cstdint>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/executor.hpp>

#include "../../test_source_library.hpp"
#include "../test_character_source.hpp"

namespace givm_test::executor_instructions::remove_dice
{
namespace
{
    struct removed_dice_log
    {
        bool dynamic;
        givm::dice_counts first;
        givm::dice_counts nested;
        givm::dice_counts last;
        std::vector<givm::player_id> players;
        std::vector<givm::dice_counts> removals;
        givm::dice_counts expected[2];
    };

    struct removed_dice_source
    {
        using definition_category = givm::support_view;
        struct definition_type
        {
            removed_dice_log* log;
            givm::program_entry effect;
            givm::program_entry nested;
        };
        removed_dice_log* log;

        constexpr std::string_view name() const { return "RemovedDice"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto zero = log->dynamic ? givm::remove_dice{}
                : givm::remove_dice{ .player = givm::relative_player::self };
            const auto first = log->dynamic ? givm::remove_dice{}
                : givm::remove_dice{ .player = givm::relative_player::opponent, .dice = log->first };
            const auto last = log->dynamic ? givm::remove_dice{}
                : givm::remove_dice{ .player = givm::relative_player::self, .dice = log->last };
            const auto nested = log->dynamic ? givm::remove_dice{}
                : givm::remove_dice{ .player = givm::relative_player::self, .dice = log->nested };
            return { log,
                context.add_program(std::tuple{ zero, first, zero, last, zero }),
                context.add_program(std::tuple{ givm::replace_cards{ givm::player_id{ 1 } }, nested }) };
        }
        static givm::program_entry handle(const definition_type& data,
            givm::round_started&, givm::handle_context<givm::support_view>& context, std::uint32_t = 0)
        {
            if(data.log->dynamic)
                return context.invoke(data.effect,
                    givm::remove_dice_input{ givm::player_id{ 1 }, {} },
                    givm::remove_dice_input{ givm::player_id{ 0 }, data.log->first },
                    givm::remove_dice_input{ givm::player_id{ 0 }, {} },
                    givm::remove_dice_input{ givm::player_id{ 1 }, data.log->last },
                    givm::remove_dice_input{ givm::player_id{ 1 }, {} });
            return context.invoke(data.effect);
        }
        static givm::program_entry handle(const definition_type& data,
            givm::dice_removed& event, givm::handle_context<givm::support_view>& context, std::uint32_t = 0)
        {
            const auto index = data.log->players.size();
            REQUIRE(index < 3);
            const auto player = givm::player_id{ index == 0 ? 0u : 1u };
            const auto& expected = index == 0 ? data.log->first : index == 1 ? data.log->nested : data.log->last;
            CHECK(event.player == player);
            CHECK(event.dice == expected);
            data.log->expected[index == 0 ? 0 : 1] -= expected;
            CHECK(context.table()[givm::player_id{ 0 }].state().dice == data.log->expected[0]);
            CHECK(context.table()[givm::player_id{ 1 }].state().dice == data.log->expected[1]);
            data.log->players.push_back(event.player);
            data.log->removals.push_back(event.dice);
            if(index == 0)
            {
                if(data.log->dynamic)
                    return context.invoke(data.nested,
                        givm::remove_dice_input{ givm::player_id{ 1 }, data.log->nested });
                return context.invoke(data.nested);
            }
            return {};
        }
    };

    struct collection_log
    {
        std::vector<givm::elemental_dice> selected;
    };

    struct collector_source
    {
        using definition_category = givm::support_view;
        struct definition_type
        {
            collection_log* log;
            givm::program_entry collect;
        };
        collection_log* log;

        constexpr std::string_view name() const { return "DiceCollector"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_program(std::tuple{ givm::remove_dice{}, givm::set_support_state{} }) };
        }
        static givm::program_entry handle(const definition_type& data,
            givm::round_ended&, givm::handle_context<givm::support_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity();
            const auto player = self.player().id();
            const auto& available = context.table()[player].state().dice;
            for(std::uint8_t index = 0; index < 8; ++index)
            {
                const auto dice = static_cast<givm::elemental_dice>(index);
                if(available[dice] == 0) continue;
                givm::dice_counts removed;
                removed[dice] = 1;
                data.log->selected.push_back(dice);
                auto state = self.state();
                ++state.count;
                return context.invoke(data.collect,
                    givm::remove_dice_input{ player, removed }, givm::set_support_state_input{ self.id(), state });
            }
            return {};
        }
    };
}

TEST_CASE("removing dice updates all types before notifying and resumes nested effects", "[remove_dice]")
{
    removed_dice_log log{ GENERATE(false, true) };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    for(std::uint8_t index = 0; index < 8; ++index)
        log.first[static_cast<givm::elemental_dice>(index)] = index + 1;
    log.nested[givm::elemental_dice::pyro] = 2;
    log.last[givm::elemental_dice::omni] = 3;
    log.expected[0] = log.first;
    log.expected[0][givm::elemental_dice::geo] += 4;
    log.expected[1] = log.nested;
    log.expected[1] += log.last;
    log.expected[1][givm::elemental_dice::cryo] = 2;
    const removed_dice_source source{ &log };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(source));
    const std::array support_names{ source.name() };
    const givm::test::initialization_skill_source initialization{
        [](givm::definition_compile_context& context)
        {
            return std::tuple{ givm::add_support{ .player = givm::relative_player::self,
                .definition = context.resolve_id<givm::support_view>("RemovedDice") } };
        }, {}, support_names };
    const givm::test::initialization_character_source character;
    REQUIRE(sources.add(initialization, character));
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources,
        std::tuple{ givm::start_battle{}, givm::start_round{}, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, mode));
    givm::table table{ { .round_number = 1, .active_player = givm::player_id{ 0 }, .self_player = givm::player_id{ 1 } },
        { .dice = log.expected[0] }, { .dice = log.expected[1] } };
    load_deck(table, library, {}, { .characters = { ids.get_id<givm::character_view>(character.name()) } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    auto random = [] { return std::uint32_t{ 0 }; };
    std::size_t pauses = 0;
    for(;;)
    {
        const auto state = executor.advance(library, table, random);
        if(state == givm::execution_state::finished) break;
        if(state != givm::execution_state::card_selection) continue;
        ++pauses;
        REQUIRE(log.players.size() == 1);
        CHECK(table[givm::player_id{ 0 }].state().dice == log.expected[0]);
        CHECK(table[givm::player_id{ 1 }].state().dice == log.expected[1]);
        auto copied_table = table;
        auto copied_executor = executor;
        table = std::move(copied_table);
        executor = std::move(copied_executor);
        const auto view = executor.view_in<givm::execution_state::card_selection>();
        REQUIRE(view.selection_validate(table, {}));
        executor.submitted(view.select(library, table, random, {}));
    }
    CHECK(pauses == 1);
    CHECK(log.players == std::vector<givm::player_id>{ givm::player_id{ 0 }, givm::player_id{ 1 }, givm::player_id{ 1 } });
    CHECK(log.removals == std::vector<givm::dice_counts>{ log.first, log.nested, log.last });
    CHECK(table[givm::player_id{ 0 }].state().dice == log.expected[0]);
    CHECK(table[givm::player_id{ 1 }].state().dice == log.expected[1]);
    CHECK(log.expected[0].total() == 4);
    CHECK(log.expected[1].total() == 2);
}

TEST_CASE("successive end-round collectors choose dice from the updated pool", "[remove_dice]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    collection_log log;
    const collector_source source{ &log };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(source));
    const std::array support_names{ source.name() };
    const givm::test::initialization_skill_source initialization{
        [](givm::definition_compile_context& context)
        {
            const givm::add_support add{ .player = givm::relative_player::self,
                .definition = context.resolve_id<givm::support_view>("DiceCollector"), .state = {} };
            return std::tuple{ add, add };
        }, {}, support_names };
    const givm::test::initialization_character_source character;
    REQUIRE(sources.add(initialization, character));
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources,
        std::tuple{ givm::start_battle{}, givm::end_round{}, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, mode));
    givm::dice_counts initial;
    initial[givm::elemental_dice::hydro] = 1;
    initial[givm::elemental_dice::pyro] = 1;
    initial[givm::elemental_dice::geo] = 2;
    givm::table table{ { .round_number = 1, .self_player = givm::player_id{ 0 } }, { .dice = initial }, {} };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(character.name()) } }, {});
    givm_test::executor_driver executor;
    executor.start(library, table);
    auto random = [] { return std::uint32_t{ 0 }; };
    while(executor.advance(library, table, random) != givm::execution_state::finished) {}
    CHECK(log.selected == std::vector<givm::elemental_dice>{ givm::elemental_dice::hydro, givm::elemental_dice::pyro });
    givm::dice_counts remaining;
    remaining[givm::elemental_dice::geo] = 2;
    CHECK(table[givm::player_id{ 0 }].state().dice == remaining);
    for(const auto support : table[givm::player_id{ 0 }].supports())
        CHECK(support.state().count == 1);
}
}
