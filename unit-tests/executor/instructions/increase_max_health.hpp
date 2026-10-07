#include <algorithm>
#include <cstdint>
#include <limits>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"

namespace givm_test::executor_instructions::increase_max_health
{
namespace
{
    constexpr givm::character_id target{ givm::player_id{ 0 }, 0 };
    struct increase_log
    {
        givm::character_state initial{ .max_health = 10, .health = 10 };
        std::uint32_t value = 3;
        bool dynamic = false;
        bool pause = false;
        std::size_t calculation_count = 0;
        std::vector<std::uint32_t> actual;
    };

    struct increase_source
    {
        static constexpr auto category = givm::definition_category::character;
        struct definition_type
        {
            increase_log* log;
            givm::normal_effect increase;
            givm::normal_effect pause;
        };
        increase_log* log;
        std::string_view name() const { return "HealthIncreaseSource"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto command = log->dynamic ? givm::increase_max_health{} : givm::increase_max_health{
                .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::self, 0 }, .value = log->value };
            return { log, context.add_normal_effect(std::tuple{ command }), log->pause
                ? context.add_normal_effect(std::tuple{ givm::replace_cards{ givm::player_id{ 0 } } }) : givm::normal_effect{} };
        }
        static givm::character_state query(const definition_type& data, const givm::character_initial_state&)
        {
            return data.log->initial;
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::round_started&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            if(data.log->dynamic)
                return context.invoke(data.increase, givm::increase_max_health_input{ target, target, data.log->value });
            return context.invoke(data.increase);
        }
        static givm::immediate_effect handle(const definition_type& data,
            givm::healing& event, givm::handle_context<givm::skill_view, givm::event_category::immediate>&, std::uint32_t = 0)
        {
            ++data.log->calculation_count;
            event.value = 0;
            return {};
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::healed& event, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            CHECK(event.target == target);
            CHECK(event.source.template get<givm::entity_category::character>() == target);
            const auto state = context.table()[target].state();
            CHECK(state.max_health == data.log->initial.max_health + event.value);
            CHECK(state.health == data.log->initial.health + event.value);
            data.log->actual.push_back(event.value);
            return data.pause ? context.invoke(data.pause) : givm::normal_effect{};
        }
    };
}

TEST_CASE("increasing maximum health restores the same amount without healing calculation", "[increase_max_health]")
{
    increase_log log;
    log.dynamic = GENERATE(false, true);
    log.pause = GENERATE(false, true);
    log.initial.max_health = GENERATE(10u, std::numeric_limits<std::uint32_t>::max() - 2);
    const auto deficit = GENERATE(0u, 5u);
    log.initial.health = log.initial.max_health - deficit;
    log.value = GENERATE(0u, 3u);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
        givm::test::with_passive_skill(increase_source{ &log }));
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::definition_category::character>("HealthIncreaseSource") } }, {});
    givm_test::executor_driver executor;
    executor.start(library, table);
    auto random = [] { return std::uint32_t{ 0 }; };
    const auto result = executor.advance(library, table, random);
    REQUIRE(result == (log.pause ? givm::execution_state::card_selection : givm::execution_state::finished));
    const auto increase = std::min(log.value, std::numeric_limits<std::uint32_t>::max() - log.initial.max_health);
    CHECK(table[target].state().max_health == log.initial.max_health + increase);
    CHECK(table[target].state().health == log.initial.health + increase);
    if(log.pause)
    {
        auto copy_executor = executor;
        auto copy_table = table;
        for(auto [running, current] : { std::pair{ &executor, &table }, std::pair{ &copy_executor, &copy_table } })
        {
            running->submitted(running->view_in<givm::execution_state::card_selection>().select(library, *current, random, {}));
            REQUIRE(running->advance(library, *current, random) == givm::execution_state::finished);
            CHECK((*current)[target].state().max_health == log.initial.max_health + increase);
            CHECK((*current)[target].state().health == log.initial.health + increase);
        }
    }
    CHECK(log.calculation_count == 0);
    CHECK(log.actual == std::vector{ increase });
}
}
