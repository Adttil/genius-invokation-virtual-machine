#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"

namespace
{
    constexpr givm::character_id patient{ givm::player_id{ 0 }, 0 };

    struct healing_log
    {
        std::uint32_t initial_health = 4;
        std::uint32_t value = 2;
        std::uint32_t bonus = 0;
        bool dynamic = false;
        bool pause = false;
        std::vector<std::uint32_t> requested;
        std::vector<std::uint32_t> actual;
    };

    struct healing_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            healing_log* log;
            givm::program_entry heal;
            givm::program_entry pause;
        };
        healing_log* log;
        std::string_view name() const { return "HealingSource"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto command = log->dynamic ? givm::heal{} : givm::heal{
                .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::self, 0 }, .value = log->value };
            return { log, context.add_program(std::tuple{ command }), log->pause
                ? context.add_program(std::tuple{ givm::replace_cards{ givm::player_id{ 0 } } }) : givm::program_entry{} };
        }
        static givm::character_state query(const definition_type& data, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = data.log->initial_health };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::test_event&, givm::handle_context& context)
        {
            if(data.log->dynamic)
                return context.invoke(data.heal, givm::healing{ patient, patient, data.log->value });
            return context.invoke(data.heal);
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::healing& event, givm::handle_context& context)
        {
            CHECK(event.target == patient);
            CHECK(std::get<givm::character_id>(event.source) == patient);
            data.log->requested.push_back(event.value);
            event.value += data.log->bonus;
            return data.pause ? context.invoke(data.pause) : givm::program_entry{};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::healed& event, givm::handle_context& context)
        {
            CHECK(event.target == patient);
            CHECK(std::get<givm::character_id>(event.source) == patient);
            data.log->actual.push_back(event.value);
            return data.pause ? context.invoke(data.pause) : givm::program_entry{};
        }
    };

    auto compile_healing(healing_log& log, givm::compile_mode mode)
    {
        return givm::test::compile_definitions_with_program(mode,
            std::tuple{ givm::test_command{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
            givm::test::with_passive_skill(healing_source{ &log }));
    }

    void load_healing(givm::table& table, const givm::definition_library& library, const givm::issued_id_map& ids)
    {
        load_deck(table, library, { .characters = { ids.get_id<givm::character_view>("HealingSource") } }, {});
    }
}

TEST_CASE("healing modifies the request then reports actual recovery including zero", "[heal]")
{
    healing_log log;
    log.dynamic = GENERATE(false, true);
    log.initial_health = GENERATE(0u, 5u, 10u);
    log.value = GENERATE(0u, 2u, std::numeric_limits<std::uint32_t>::max() - 3);
    log.bonus = GENERATE(0u, 3u);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto [library, ids] = compile_healing(log, mode);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } } };
    load_healing(table, library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    auto random = [] { return std::uint32_t{ 0 }; };
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    const auto actual = std::min(log.value + log.bonus, 10 - log.initial_health);
    CHECK(table[patient].state().health == log.initial_health + actual);
    CHECK(log.requested == std::vector{ log.value });
    CHECK(log.actual == std::vector{ actual });
}

TEST_CASE("healing broadcasts resume and copy before and after recovery", "[heal][resume]")
{
    healing_log log{ .value = 5, .bonus = 1, .dynamic = GENERATE(false, true), .pause = true };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto [library, ids] = compile_healing(log, mode);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } } };
    load_healing(table, library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    auto random = [] { return std::uint32_t{ 0 }; };
    REQUIRE(executor.step(library, table, random) == givm::execution_state::card_selection);
    CHECK(table[patient].state().health == 4);
    CHECK(log.requested == std::vector{ 5u });
    CHECK(log.actual.empty());
    auto before_executor = executor;
    auto before_table = table;
    for(auto [running, current] : { std::pair{ &executor, &table }, std::pair{ &before_executor, &before_table } })
    {
        running->view_in<givm::execution_state::card_selection>().select({});
        REQUIRE(running->step(library, *current, random) == givm::execution_state::card_selection);
        CHECK((*current)[patient].state().health == 10);
        auto after_executor = *running;
        auto after_table = *current;
        for(auto [finishing, final] : { std::pair{ running, current }, std::pair{ &after_executor, &after_table } })
        {
            finishing->view_in<givm::execution_state::card_selection>().select({});
            REQUIRE(finishing->step(library, *final, random) == givm::execution_state::finished);
            CHECK((*final)[patient].state().health == 10);
        }
    }
    CHECK(log.requested == std::vector{ 5u });
    CHECK(log.actual == std::vector{ 6u, 6u });
}
