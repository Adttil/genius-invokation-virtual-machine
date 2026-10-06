#include <algorithm>
#include <array>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../table/test_definition_library.hpp"
#include "test_character_source.hpp"

namespace givm_test::executor::program_input_ownership
{
namespace
{
    constexpr givm::character_id actor{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id opponent{ givm::player_id{ 1 }, 0 };
    constexpr std::size_t target_count = 257;
    enum class preparation { fixed, dynamic, concatenated, bad_concatenation, bad_fixed };

    givm::defer_program_input make_owned_deferred(givm::program_entry relay, givm::program_entry leaf)
    {
        std::vector<givm::character_id> targets(target_count, actor);
        auto invocation = givm::defer_invoke(leaf, givm::modify_energy_input{ targets, 1 });
        // Packing must already own the range, before the eventual invoke or compilation.
        std::ranges::fill(targets, opponent);
        return givm::defer_invoke(relay, std::move(invocation));
    }

    struct input_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            preparation mode;
            givm::program_entry main;
            givm::program_entry relay;
            givm::program_entry leaf;
        };
        preparation mode;
        std::string_view name() const { return "OwnedProgramInputs"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            using namespace givm;
            const auto leaf = context.add_program(modify_energy{}, return_response{ .index = 17 });
            const auto relay = context.add_program(defer_program{}, return_response{ .index = 19 });
            if(mode == preparation::fixed || mode == preparation::bad_fixed)
            {
                const auto invocation = mode == preparation::bad_fixed
                    ? defer_invoke(relay, defer_invoke(leaf, set_energy_input{ actor, 7 }))
                    : make_owned_deferred(relay, leaf);
                const auto main = context.add_program(
                    set_energy{ .target = { relative_player::self, 0 }, .value = 2 },
                    defer_program{ invocation }, replace_cards{ .player = actor.player_id },
                    modify_energy{ .target = { relative_player::self, 0 }, .delta = 3 });
                return { mode, main, relay, leaf };
            }
            return { mode, context.add_program(set_energy{}, defer_program{},
                replace_cards{ .player = actor.player_id }, modify_energy{}), relay, leaf };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .max_energy = 10000, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data, givm::round_started&,
            givm::handle_context<givm::skill_view>& context, std::uint32_t)
        {
            using namespace givm;
            if(data.mode == preparation::fixed) return context.invoke(data.main);
            const auto deferred = make_owned_deferred(data.relay, data.leaf);
            const set_energy_input first{ actor, 2 };
            if(data.mode == preparation::dynamic)
                return context.invoke(data.main, first, deferred,
                    modify_energy_input{ std::array{ actor }, 3 });

            // Every chunk and its original span storage expires before invoke.
            const auto inputs = [&]
            {
                std::vector<givm::program_inputs> chunks;
                chunks.push_back(pack_inputs());
                chunks.push_back(pack_inputs(first, deferred));
                chunks.push_back(givm::program_inputs{});
                if(data.mode == preparation::bad_concatenation)
                    chunks.push_back(pack_inputs(set_energy_input{ actor, 3 }));
                else
                    chunks.push_back(pack_inputs(modify_energy_input{ std::array{ actor }, 3 }));
                chunks.push_back(pack_inputs());
                return concat_inputs(chunks);
            }();
            return context.invoke(data.main, inputs);
        }
    };

    auto compile_inputs(preparation mode)
    {
        // Definition sources and all fixed input objects are destroyed on return.
        const auto source = givm::test::with_passive_skill(input_source{ mode });
        const givm::test::initialized_character_source plain{ "OwnedInputOpponent",
            { .max_health = 10, .max_energy = 10000, .health = 10 } };
        return givm::test::compile_definitions_with_program(givm::compile_mode::normal,
            std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, source, plain);
    }

    givm::table make_table(const givm::definition_library& library, const givm::issued_id_map& ids)
    {
        givm::table table{ { .self_player = actor.player_id },
            { .active_character = actor }, { .active_character = opponent } };
        load_deck(table, library, { .characters = { ids.get_id<givm::character_view>("OwnedProgramInputs") } },
            { .characters = { ids.get_id<givm::character_view>("OwnedInputOpponent") } });
        return table;
    }
}

TEST_CASE("prepared program inputs own nested ranges through compilation invocation pauses and copies", "[program-input][settlement][ownership]")
{
    const auto mode = GENERATE(preparation::fixed, preparation::dynamic, preparation::concatenated);
    const auto [compiled_library, ids] = compile_inputs(mode);
    const auto library = compiled_library;
    auto table = make_table(library, ids);
    givm::executor execution;
    REQUIRE(execution.start(library, table).resume(library, table, givm_test::zero_random) == givm::execution_state::card_selection);
    CHECK(table[actor].state().energy == 2);
    CHECK(table[opponent].state().energy == 0);
    auto copied = execution;
    auto copied_table = table;
    const auto finish = [&](givm::executor& current, givm::table& current_table)
    {
        REQUIRE(current.view_in<givm::execution_state::card_selection>().select(library, current_table,
            givm_test::zero_random, {}) == givm::execution_state::finished);
        CHECK(current_table[actor].state().energy == target_count + 5);
        CHECK(current_table[opponent].state().energy == 0);
    };
    finish(execution, table);
    finish(copied, copied_table);
}

#ifndef NDEBUG
TEST_CASE("concatenated inputs report the logical position of a later mismatched chunk", "[program-input][debug][ownership]")
{
    const auto [library, ids] = compile_inputs(preparation::bad_concatenation);
    auto table = make_table(library, ids);
    givm::executor execution;
    try
    {
        execution.start(library, table).resume(library, table, givm_test::zero_random);
        FAIL("a mismatched later chunk must be rejected before executing the program");
    }
    catch(const givm::program_input_error& exception)
    {
        const auto* reason = std::get_if<givm::program_input_type_mismatch>(&exception.reason);
        REQUIRE(reason);
        CHECK(reason->input_index == 2);
        CHECK(reason->command_index == 3);
        CHECK(reason->expected == "modify_energy_input");
        CHECK(reason->actual == "set_energy_input");
    }
    CHECK(table[actor].state().energy == 0);
    CHECK(table[opponent].state().energy == 0);
}
TEST_CASE("fixed deferred payload type mismatches are compile diagnostics", "[program-input][debug][ownership]")
{
    const auto source = givm::test::with_passive_skill(input_source{ preparation::bad_fixed });
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(source, source.passive));
    const auto result = givm::compile(sources, givm_test::basic_sources,
        std::tuple{ givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, givm::compile_mode::normal);
    REQUIRE_FALSE(result);
    bool found = false;
    for(const auto& diagnostic : result.error())
    {
        const auto* fixed = std::get_if<givm::fixed_program_input_error>(&diagnostic.reason);
        if(not fixed) continue;
        found = true;
        const auto* mismatch = std::get_if<givm::program_input_type_mismatch>(&fixed->reason);
        REQUIRE(mismatch);
        CHECK(mismatch->input_index == 0);
        CHECK(mismatch->command_index == 0);
        CHECK(mismatch->expected == "modify_energy_input");
        CHECK(mismatch->actual == "set_energy_input");
    }
    CHECK(found);
}
#endif
}
