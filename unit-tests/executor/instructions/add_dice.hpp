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

namespace givm_test::executor_instructions::add_dice
{
namespace
{
    struct added_dice_log
    {
        bool dynamic;
        givm::dice_counts first;
        givm::dice_counts nested;
        givm::dice_counts last;
        std::vector<givm::player_id> players;
        std::vector<givm::dice_counts> additions;
        givm::dice_counts expected[2];
    };

    struct added_dice_source
    {
        using definition_category = givm::support_view;
        struct definition_type
        {
            added_dice_log* log;
            givm::normal_effect effect;
            givm::normal_effect nested;
        };
        added_dice_log* log;

        constexpr std::string_view name() const { return "AddedDice"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto zero = log->dynamic ? givm::add_dice{}
                : givm::add_dice{ .player = givm::relative_player::self };
            const auto first = log->dynamic ? givm::add_dice{}
                : givm::add_dice{ .player = givm::relative_player::opponent, .dice = log->first };
            const auto last = log->dynamic ? givm::add_dice{}
                : givm::add_dice{ .player = givm::relative_player::self, .dice = log->last };
            const auto nested = log->dynamic ? givm::add_dice{}
                : givm::add_dice{ .player = givm::relative_player::self, .dice = log->nested };
            return { log,
                context.add_normal_effect(std::tuple{ zero, first, givm::settle{}, zero, last, zero }),
                context.add_normal_effect(std::tuple{ givm::replace_cards{ givm::player_id{ 1 } }, nested }) };
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::round_started&, givm::handle_context<givm::support_view>& context, std::uint32_t = 0)
        {
            if(data.log->dynamic)
                return context.invoke(data.effect,
                    givm::add_dice_input{ givm::player_id{ 1 }, {} },
                    givm::add_dice_input{ givm::player_id{ 0 }, data.log->first },
                    givm::add_dice_input{ givm::player_id{ 0 }, {} },
                    givm::add_dice_input{ givm::player_id{ 1 }, data.log->last },
                    givm::add_dice_input{ givm::player_id{ 1 }, {} });
            return context.invoke(data.effect);
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::dice_added& event, givm::handle_context<givm::support_view>& context, std::uint32_t = 0)
        {
            const auto index = data.log->players.size();
            REQUIRE(index < 3);
            const auto player = givm::player_id{ index == 0 ? 0u : 1u };
            const auto& expected = index == 0 ? data.log->first : index == 1 ? data.log->nested : data.log->last;
            CHECK(event.player == player);
            CHECK(event.dice == expected);
            data.log->expected[index == 0 ? 0 : 1] += expected;
            CHECK(context.table()[givm::player_id{ 0 }].state().dice == data.log->expected[0]);
            CHECK(context.table()[givm::player_id{ 1 }].state().dice == data.log->expected[1]);
            data.log->players.push_back(event.player);
            data.log->additions.push_back(event.dice);
            if(index == 0)
            {
                if(data.log->dynamic)
                    return context.invoke(data.nested,
                        givm::add_dice_input{ givm::player_id{ 1 }, data.log->nested });
                return context.invoke(data.nested);
            }
            return {};
        }
    };
}

TEST_CASE("adding dice updates all types before notifying and resumes nested effects", "[add_dice]")
{
    added_dice_log log{ GENERATE(false, true) };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    for(std::uint8_t index = 0; index < 8; ++index)
        log.first[static_cast<givm::elemental_dice>(index)] = index + 1;
    log.nested[givm::elemental_dice::pyro] = 2;
    log.last[givm::elemental_dice::omni] = 3;
    log.expected[0][givm::elemental_dice::geo] = 4;
    log.expected[1][givm::elemental_dice::cryo] = 2;
    const added_dice_source source{ &log };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(source));
    const std::array support_names{ source.name() };
    const givm::test::initialization_skill_source initialization{
        [](givm::definition_compile_context& context)
        {
            return std::tuple{ givm::add_support{ .player = givm::relative_player::self,
                .definition = context.resolve_id<givm::support_view>("AddedDice") } };
        }, {}, support_names };
    const givm::test::initialization_character_source character;
    REQUIRE(sources.add(initialization, character));
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources,
        std::tuple{ givm::start_battle{}, givm::settle{}, givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, mode));
    givm::table table{ { .round_number = 1, .self_player = givm::player_id{ 1 } },
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
    CHECK(log.additions == std::vector<givm::dice_counts>{ log.first, log.nested, log.last });
    CHECK(table[givm::player_id{ 0 }].state().dice == log.expected[0]);
    CHECK(table[givm::player_id{ 1 }].state().dice == log.expected[1]);
}
}
