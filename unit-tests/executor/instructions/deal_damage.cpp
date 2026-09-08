#include <cstdint>
#include <limits>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/executor/instructions/deal_damage.hpp>

#include "../../table/test_definition_library.hpp"

using namespace givm;

namespace
{
    enum class observed_event
    {
        calculation,
        effect,
        reaction_will_occur,
        after_reaction,
        after_damage
    };

    struct damage_log
    {
        std::vector<observed_event> order;
        std::uint32_t calculation_bonus = 0;
        std::uint16_t multiplier_numerator = 1;
        std::uint16_t multiplier_denominator = 1;
        std::uint32_t effect_reduction = 0;
        std::uint32_t after_damage_value = 0;
        elemental_reaction reaction = elemental_reaction::none;
        element_application_cause reaction_cause = element_application_cause::effect;
    };

    struct damage_observer_source
    {
        using definition_category = support_view;

        struct definition_type
        {
            damage_log* log;
        };

        damage_log* log;

        constexpr std::string_view name() const noexcept
        {
            return "DamageObserver";
        }

        constexpr definition_type compile(definition_compile_context&) const noexcept
        {
            return { log };
        }

        static program_entry<damage_calculation> handle(
            const definition_type& data,
            const support_view&,
            damage_calculation& event,
            const card_table&,
            random_fn&
        )
        {
            data.log->order.push_back(observed_event::calculation);
            event.value += data.log->calculation_bonus;
            event.multiplier_numerator = data.log->multiplier_numerator;
            event.multiplier_denominator = data.log->multiplier_denominator;
            return program_entry<damage_calculation>::null();
        }

        static program_entry<damage_effect> handle(
            const definition_type& data,
            const support_view&,
            damage_effect& event,
            const card_table&,
            random_fn&
        )
        {
            data.log->order.push_back(observed_event::effect);
            event.value = event.value < data.log->effect_reduction
                ? 0
                : event.value - data.log->effect_reduction;
            return program_entry<damage_effect>::null();
        }

        static program_entry<elemental_reaction_will_occur> handle(
            const definition_type& data,
            const support_view&,
            elemental_reaction_will_occur& event,
            const card_table&,
            random_fn&
        )
        {
            data.log->order.push_back(observed_event::reaction_will_occur);
            data.log->reaction = event.reaction;
            data.log->reaction_cause = event.cause;
            return program_entry<elemental_reaction_will_occur>::null();
        }

        static program_entry<after_elemental_reaction> handle(
            const definition_type& data,
            const support_view&,
            after_elemental_reaction&,
            const card_table&,
            random_fn&
        )
        {
            data.log->order.push_back(observed_event::after_reaction);
            return program_entry<after_elemental_reaction>::null();
        }

        static program_entry<after_damage> handle(
            const definition_type& data,
            const support_view&,
            after_damage& event,
            const card_table&,
            random_fn&
        )
        {
            data.log->order.push_back(observed_event::after_damage);
            data.log->after_damage_value = event.value;
            return program_entry<after_damage>::null();
        }
    };

    struct zero_random
    {
        std::uint32_t operator()() const noexcept
        {
            return 0;
        }
    };

    struct stop_execution
    {
        using context_type = void;

        bool execute(card_table&, execution_context& context, random_fn&) const noexcept
        {
            return context.yield();
        }
    };

    bool run_until_stop(executor& target, card_table& table)
    {
        zero_random random;
        while(target.execute_next(table, random))
        {
        }
        return target.status() == game_result::no_result
            && table.definition_library().instruction(target.position()).is<stop_execution>();
    }

    game_result result_after_damage(
        std::uint32_t player_0_health,
        std::uint32_t player_1_health,
        player_id damaged_player
    )
    {
        const test::named_definition_source<character_view> character_source{ "Character" };
        const character_id target{ .player_id = damaged_player, .index = 0 };
        const character_id source{
            .player_id = other_player(damaged_player),
            .index = 0
        };
        const auto [library, id_map] = test::compile_definitions_with_program(
            std::tuple{
                deal_damage{
                    .source = source,
                    .target = target,
                    .value = 1,
                    .type = damage_type::physical
                }
            },
            std::tuple{ stop_execution{} },
            character_source
        );
        const auto definition_id = id_map.get_id<character_view>(
            character_source.name()
        );

        card_table table{ library };
        table[player_id{ 0 }].add(definition_id, {
            .max_health = 10,
            .max_energy = 3,
            .health = player_0_health,
            .energy = 0
        });
        table[player_id{ 1 }].add(definition_id, {
            .max_health = 10,
            .max_energy = 3,
            .health = player_1_health,
            .energy = 0
        });

        executor executor;
        executor.enter_entry(table.definition_library());
        zero_random random;
        while(executor.execute_next(table, random))
        {
        }
        return executor.status();
    }
}

TEST_CASE("deal_damage determines every terminal team result", "[deal_damage][game-result]")
{
    CHECK(result_after_damage(10, 10, player_id{ 1 }) == game_result::no_result);
    CHECK(result_after_damage(1, 10, player_id{ 0 }) == game_result::player_1_win);
    CHECK(result_after_damage(10, 1, player_id{ 1 }) == game_result::player_0_win);
    CHECK(result_after_damage(0, 1, player_id{ 1 }) == game_result::both_loss);
}

TEST_CASE("deal_damage exposes calculation and effect mutation before health loss", "[deal_damage]")
{
    damage_log log{
        .calculation_bonus = 1,
        .multiplier_numerator = 3,
        .multiplier_denominator = 2,
        .effect_reduction = 3
    };
    const damage_observer_source observer_source{ &log };
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr support_id observer_entity_id{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id character_entity_id{ .player_id = player_id{ 1 }, .index = 0 };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{
            deal_damage{
                .source = observer_entity_id,
                .target = character_entity_id,
                .value = 4,
                .type = damage_type::physical
            }
        },
        std::tuple{ stop_execution{} },
        observer_source,
        character_source
    );
    const auto observer_id = id_map.get_id<support_view>(observer_source.name());
    const auto definition_id = id_map.get_id<character_view>(character_source.name());

    card_table table{ library };
    const auto observer = table[player_id{ 0 }].add(observer_id, { .count = 1 });
    REQUIRE(observer.id() == observer_entity_id);
    table[player_id{ 0 }].add(definition_id, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    });
    const auto character = table[player_id{ 1 }].add(definition_id, {
        .max_health = 20, .max_energy = 3, .health = 20, .energy = 0
    });
    REQUIRE(character.id() == character_entity_id);

    executor target;
    target.enter_entry(table.definition_library());

    REQUIRE(run_until_stop(target, table));
    CHECK(log.order == std::vector{
        observed_event::calculation,
        observed_event::effect,
        observed_event::after_damage
    });
    CHECK(log.after_damage_value == 4);
    CHECK(character.state().health == 16);
    CHECK(character.state().aura == element_aura::none);
}

TEST_CASE("elemental deal_damage applies its reaction between effect and after_damage", "[deal_damage]")
{
    damage_log log;
    const damage_observer_source observer_source{ &log };
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr support_id observer_entity_id{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id character_entity_id{ .player_id = player_id{ 1 }, .index = 0 };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{
            deal_damage{
                .source = observer_entity_id,
                .target = character_entity_id,
                .value = 3,
                .type = damage_type::pyro
            }
        },
        std::tuple{ stop_execution{} },
        observer_source,
        character_source
    );
    const auto observer_id = id_map.get_id<support_view>(observer_source.name());
    const auto definition_id = id_map.get_id<character_view>(character_source.name());

    card_table table{ library };
    const auto observer = table[player_id{ 0 }].add(observer_id, { .count = 1 });
    REQUIRE(observer.id() == observer_entity_id);
    table[player_id{ 0 }].add(definition_id, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    });
    const auto character = table[player_id{ 1 }].add(definition_id, {
        .max_health = 10,
        .max_energy = 3,
        .health = 10,
        .energy = 0,
        .aura = element_aura::cryo
    });
    REQUIRE(character.id() == character_entity_id);

    executor target;
    target.enter_entry(table.definition_library());

    REQUIRE(run_until_stop(target, table));
    CHECK(log.order == std::vector{
        observed_event::calculation,
        observed_event::effect,
        observed_event::reaction_will_occur,
        observed_event::after_reaction,
        observed_event::after_damage
    });
    CHECK(log.reaction == elemental_reaction::melt);
    CHECK(log.reaction_cause == element_application_cause::damage);
    CHECK(log.after_damage_value == 5);
    CHECK(character.state().health == 5);
    CHECK(character.state().aura == element_aura::none);
}

TEST_CASE("deal_damage clamps lethal health loss without ending a living team", "[deal_damage]")
{
    damage_log log;
    const damage_observer_source observer_source{ &log };
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr support_id observer_entity_id{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id target_character_id{ .player_id = player_id{ 1 }, .index = 0 };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{
            deal_damage{
                .source = observer_entity_id,
                .target = target_character_id,
                .value = 100,
                .type = damage_type::physical
            }
        },
        std::tuple{ stop_execution{} },
        observer_source,
        character_source
    );
    const auto observer_id = id_map.get_id<support_view>(observer_source.name());
    const auto definition_id = id_map.get_id<character_view>(character_source.name());

    card_table table{ library };
    const auto observer = table[player_id{ 0 }].add(observer_id, { .count = 1 });
    REQUIRE(observer.id() == observer_entity_id);
    table[player_id{ 1 }].add(definition_id, {
        .max_health = 4, .max_energy = 3, .health = 4, .energy = 0
    });
    table[player_id{ 1 }].add(definition_id, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    });
    table[player_id{ 0 }].add(definition_id, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    });

    executor target;
    target.enter_entry(table.definition_library());

    REQUIRE(run_until_stop(target, table));
    CHECK(table[target_character_id].state().health == 0);
}

TEST_CASE("deal_damage saturates reaction bonus and multiplier", "[deal_damage]")
{
    constexpr auto max_value = std::numeric_limits<std::uint32_t>::max();
    damage_log log{
        .multiplier_numerator = 2,
        .multiplier_denominator = 1
    };
    const damage_observer_source observer_source{ &log };
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr support_id observer_entity_id{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id target_character_id{ .player_id = player_id{ 1 }, .index = 0 };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{
            deal_damage{
                .source = observer_entity_id,
                .target = target_character_id,
                .value = max_value - 1,
                .type = damage_type::pyro
            }
        },
        std::tuple{ stop_execution{} },
        observer_source,
        character_source
    );
    const auto observer_id = id_map.get_id<support_view>(observer_source.name());
    const auto definition_id = id_map.get_id<character_view>(character_source.name());

    card_table table{ library };
    const auto observer = table[player_id{ 0 }].add(observer_id, { .count = 1 });
    REQUIRE(observer.id() == observer_entity_id);
    table[player_id{ 0 }].add(definition_id, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    });
    const auto target_character = table[player_id{ 1 }].add(definition_id, {
        .max_health = max_value,
        .max_energy = 3,
        .health = max_value,
        .energy = 0,
        .aura = element_aura::cryo
    });
    REQUIRE(target_character.id() == target_character_id);
    table[player_id{ 1 }].add(definition_id, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    });

    executor target;
    target.enter_entry(table.definition_library());

    REQUIRE(run_until_stop(target, table));
    CHECK(log.reaction == elemental_reaction::melt);
    CHECK(log.after_damage_value == max_value);
    CHECK(table[target_character_id].state().health == 0);
}
