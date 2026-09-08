#include <cstdint>
#include <string_view>
#include <tuple>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>

#include <givm/executor/instructions/absorb_damage_by_count.hpp>
#include <givm/executor/instructions/deal_damage.hpp>

#include "../../table/test_definition_library.hpp"

using namespace givm;

namespace
{
    struct count_shield_source
    {
        using definition_category = combat_status_view;

        struct definition_type
        {
            std::uint32_t maximum_count;
            program_entry<damage_effect> absorption_entry;
        };

        std::string_view source_name;
        std::uint32_t maximum_count;

        constexpr std::string_view name() const noexcept
        {
            return source_name;
        }

        definition_type compile(definition_compile_context& context) const
        {
            return {
                .maximum_count = maximum_count,
                .absorption_entry = context.add_program<damage_effect>(
                    std::tuple{ absorb_damage_by_count{ .maximum_count = maximum_count } }
                )
            };
        }

        static program_entry<damage_effect> handle(
            const definition_type& data,
            const combat_status_view& self,
            damage_effect& event,
            const card_table&,
            random_fn&
        )
        {
            if(
                event.value == 0
                || event.target.player_id != self.player().id()
                || event.flags.contains(damage_flag_bits::ignore_shield)
                || self.state().count == 0
            )
            {
                return program_entry<damage_effect>::null();
            }
            return data.absorption_entry;
        }
    };

    struct zero_random
    {
        std::uint32_t operator()() const noexcept
        {
            return 0;
        }
    };

    struct pause_once
    {
        using context_type = void;

        bool execute(card_table&, execution_context& context, random_fn&) const
        {
            if(context.current_stage() == stage_t{})
            {
                ++context.current_stage();
                return context.yield();
            }
            return context.enter_next();
        }
    };

    void run_until_blocked(executor& execution, card_table& table)
    {
        zero_random random;
        while(execution.execute_next(table, random))
        {
        }
    }
}

TEST_CASE(
    "fixed count shields reuse one response program for each current handler",
    "[absorb_damage_by_count][fixed-program]"
)
{
    STATIC_REQUIRE(instruction_compatible_with<absorb_damage_by_count, damage_effect>);
    STATIC_REQUIRE(not instruction_compatible_with<absorb_damage_by_count, after_damage>);

    const count_shield_source shield_source{ "CountShield", 2 };
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr character_id attacker_id{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id damaged_character_id{ .player_id = player_id{ 1 }, .index = 0 };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{
            deal_damage{
                .source = attacker_id,
                .target = damaged_character_id,
                .value = 7,
                .type = damage_type::physical,
                .flags = {}
            },
            pause_once{},
            deal_damage{
                .source = attacker_id,
                .target = damaged_character_id,
                .value = 2,
                .type = damage_type::physical,
                .flags = {}
            },
            pause_once{}
        },
        std::tuple{},
        shield_source,
        character_source
    );
    const auto shield_definition = id_map.get_id<combat_status_view>(shield_source.name());
    const auto character_definition = id_map.get_id<character_view>(character_source.name());

    card_table table{ library };
    const auto attacker = table[player_id{ 0 }].add(character_definition, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    }).id();
    REQUIRE(attacker == attacker_id);
    const auto other_side_shield = table[player_id{ 0 }].add(
        shield_definition,
        { .count = 7 }
    ).id();
    const auto damaged_character = table[player_id{ 1 }].add(character_definition, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    }).id();
    REQUIRE(damaged_character == damaged_character_id);
    const auto first_shield = table[player_id{ 1 }].add(
        shield_definition,
        { .count = 3 }
    ).id();
    const auto second_shield = table[player_id{ 1 }].add(
        shield_definition,
        { .count = 1 }
    ).id();

    executor execution;
    execution.enter_entry(table.definition_library());
    run_until_blocked(execution, table);

    CHECK(table[other_side_shield].state().count == 7);
    CHECK(table[first_shield].state().count == 1);
    CHECK(table[second_shield].state().count == 0);
    CHECK(table[damaged_character].state().health == 6);

    run_until_blocked(execution, table);

    CHECK(table[first_shield].state().count == 0);
    CHECK(table[second_shield].state().count == 0);
    CHECK(table[damaged_character].state().health == 5);
}

TEST_CASE(
    "damage fully absorbed by a second fixed count shield leaves later shields untouched",
    "[absorb_damage_by_count][fixed-program]"
)
{
    const count_shield_source shield_source{ "CountShield", 10 };
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr character_id attacker_id{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id damaged_character_id{ .player_id = player_id{ 1 }, .index = 0 };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{
            deal_damage{
                .source = attacker_id,
                .target = damaged_character_id,
                .value = 3,
                .type = damage_type::physical,
                .flags = {}
            },
            pause_once{}
        },
        std::tuple{},
        shield_source,
        character_source
    );
    const auto shield_definition = id_map.get_id<combat_status_view>(shield_source.name());
    const auto character_definition = id_map.get_id<character_view>(character_source.name());

    card_table table{ library };
    const auto attacker = table[player_id{ 0 }].add(character_definition, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    }).id();
    REQUIRE(attacker == attacker_id);
    const auto damaged_character = table[player_id{ 1 }].add(character_definition, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    }).id();
    REQUIRE(damaged_character == damaged_character_id);
    const auto first_shield = table[player_id{ 1 }].add(
        shield_definition,
        { .count = 2 }
    ).id();
    const auto second_shield = table[player_id{ 1 }].add(
        shield_definition,
        { .count = 3 }
    ).id();
    const auto third_shield = table[player_id{ 1 }].add(
        shield_definition,
        { .count = 4 }
    ).id();

    executor execution;
    execution.enter_entry(table.definition_library());
    run_until_blocked(execution, table);

    CHECK(table[first_shield].state().count == 0);
    CHECK(table[second_shield].state().count == 2);
    CHECK(table[third_shield].state().count == 4);
    CHECK(table[damaged_character].state().health == 10);
}

TEST_CASE(
    "a fixed count shield leaves ignored damage and its count unchanged",
    "[absorb_damage_by_count][fixed-program]"
)
{
    const count_shield_source shield_source{ "CountShield", 2 };
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr character_id attacker_id{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id damaged_character_id{ .player_id = player_id{ 1 }, .index = 0 };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{
            deal_damage{
                .source = attacker_id,
                .target = damaged_character_id,
                .value = 2,
                .type = damage_type::physical,
                .flags = damage_flags{ damage_flag_bits::ignore_shield }
            },
            pause_once{}
        },
        std::tuple{},
        shield_source,
        character_source
    );
    const auto shield_definition = id_map.get_id<combat_status_view>(shield_source.name());
    const auto character_definition = id_map.get_id<character_view>(character_source.name());

    card_table table{ library };
    const auto attacker = table[player_id{ 0 }].add(character_definition, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    }).id();
    REQUIRE(attacker == attacker_id);
    const auto damaged_character = table[player_id{ 1 }].add(character_definition, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    }).id();
    REQUIRE(damaged_character == damaged_character_id);
    const auto shield = table[player_id{ 1 }].add(
        shield_definition,
        { .count = 3 }
    ).id();

    executor execution;
    execution.enter_entry(table.definition_library());
    run_until_blocked(execution, table);

    CHECK(table[shield].state().count == 3);
    CHECK(table[damaged_character].state().health == 8);
}
