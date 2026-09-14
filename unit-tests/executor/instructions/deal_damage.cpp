#include "../../executor_access.hpp"
#include <cstdint>
#include <limits>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/executor.hpp>

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

        execution_state execute(const definition_library&, detail::unrestricted_table&, detail::execution_context& context, random_fn&) const noexcept
        {
            return context.yield(execution_state::action);
        }
    };

    bool run_until_stop(const definition_library& library, executor& target, card_table& table)
    {
        zero_random random;
        return target.run(library, table, random) == execution_state::action;
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

        card_table table{};
        auto& mutable_table = detail::executor_access::unrestricted(table);
        mutable_table[player_id{ 0 }].add(definition_id, {
            .max_health = 10,
            .max_energy = 3,
            .health = player_0_health,
            .energy = 0
        });
        mutable_table[player_id{ 1 }].add(definition_id, {
            .max_health = 10,
            .max_energy = 3,
            .health = player_1_health,
            .energy = 0
        });

        executor executor;
        executor.enter_entry(library);
        zero_random random;
        const auto state = executor.run(library, table, random);
        return state == execution_state::finished
            ? executor.view_in<execution_state::finished>().result()
            : game_result::no_result;
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

    card_table table{};
    auto& mutable_table = detail::executor_access::unrestricted(table);
    const auto observer = mutable_table[player_id{ 0 }].add(observer_id, { .count = 1 });
    REQUIRE(observer.id() == observer_entity_id);
    mutable_table[player_id{ 0 }].add(definition_id, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    });
    const auto character = mutable_table[player_id{ 1 }].add(definition_id, {
        .max_health = 20, .max_energy = 3, .health = 20, .energy = 0
    });
    REQUIRE(character.id() == character_entity_id);

    executor target;
    target.enter_entry(library);

    REQUIRE(run_until_stop(library, target, table));
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

    card_table table{};
    auto& mutable_table = detail::executor_access::unrestricted(table);
    const auto observer = mutable_table[player_id{ 0 }].add(observer_id, { .count = 1 });
    REQUIRE(observer.id() == observer_entity_id);
    mutable_table[player_id{ 0 }].add(definition_id, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    });
    const auto character = mutable_table[player_id{ 1 }].add(definition_id, {
        .max_health = 10,
        .max_energy = 3,
        .health = 10,
        .energy = 0,
        .aura = element_aura::cryo
    });
    REQUIRE(character.id() == character_entity_id);

    executor target;
    target.enter_entry(library);

    REQUIRE(run_until_stop(library, target, table));
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

    card_table table{};
    auto& mutable_table = detail::executor_access::unrestricted(table);
    const auto observer = mutable_table[player_id{ 0 }].add(observer_id, { .count = 1 });
    REQUIRE(observer.id() == observer_entity_id);
    mutable_table[player_id{ 1 }].add(definition_id, {
        .max_health = 4, .max_energy = 3, .health = 4, .energy = 0
    });
    mutable_table[player_id{ 1 }].add(definition_id, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    });
    mutable_table[player_id{ 0 }].add(definition_id, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    });

    executor target;
    target.enter_entry(library);

    REQUIRE(run_until_stop(library, target, table));
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

    card_table table{};
    auto& mutable_table = detail::executor_access::unrestricted(table);
    const auto observer = mutable_table[player_id{ 0 }].add(observer_id, { .count = 1 });
    REQUIRE(observer.id() == observer_entity_id);
    mutable_table[player_id{ 0 }].add(definition_id, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    });
    const auto target_character = mutable_table[player_id{ 1 }].add(definition_id, {
        .max_health = max_value,
        .max_energy = 3,
        .health = max_value,
        .energy = 0,
        .aura = element_aura::cryo
    });
    REQUIRE(target_character.id() == target_character_id);
    mutable_table[player_id{ 1 }].add(definition_id, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    });

    executor target;
    target.enter_entry(library);

    REQUIRE(run_until_stop(library, target, table));
    CHECK(log.reaction == elemental_reaction::melt);
    CHECK(log.after_damage_value == max_value);
    CHECK(table[target_character_id].state().health == 0);
}

TEST_CASE("damage observation exposes the final value before elemental settlement", "[deal_damage][observation]")
{
    auto initial_aura = element_aura::none;
    SECTION("non-reactive element application") {}
    SECTION("elemental reaction") { initial_aura = element_aura::cryo; }

    damage_log log{ .effect_reduction = 1 };
    const damage_observer_source observer_source{ &log };
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr support_id source{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id damaged{ .player_id = player_id{ 1 }, .index = 0 };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{ deal_damage{
            .source = source,
            .target = damaged,
            .value = 3,
            .type = damage_type::pyro,
            .flags = damage_flag_bits::skill_damage
        } },
        std::tuple{ stop_execution{} },
        observer_source, character_source
    );
    const auto character_definition = ids.get_id<character_view>(character_source.name());
    card_table observed_table{};
    auto& mutable_observed_table = detail::executor_access::unrestricted(observed_table);
    mutable_observed_table[player_id{ 0 }].add(ids.get_id<support_view>(observer_source.name()), { .count = 1 });
    mutable_observed_table[player_id{ 0 }].add(character_definition, { .max_health = 10, .health = 10 });
    mutable_observed_table[player_id{ 1 }].add(character_definition, {
        .max_health = 10, .health = 10, .aura = initial_aura
    });
    auto normal_table = observed_table;
    zero_random random;
    executor normal;
    normal.enter_entry(library);
    REQUIRE(normal.run(library, normal_table, random) == execution_state::action);
    const auto normal_order = log.order;
    log.order.clear();

    executor observed;
    observed.enter_entry(library);
    REQUIRE(observed.step(library, observed_table, random) == execution_state::health_reduced);
    const auto health = observed.view_in<execution_state::health_reduced>();
    const auto expected_damage = initial_aura == element_aura::cryo ? 4u : 2u;
    CHECK(health.source() == damage_source_id{ source });
    CHECK(health.target() == damaged);
    CHECK(health.value() == expected_damage);
    CHECK(health.type() == damage_type::pyro);
    CHECK(health.flags().contains(damage_flag_bits::skill_damage));
    CHECK(observed_table[damaged].state().health == 10 - expected_damage);
    CHECK(observed_table[damaged].state().aura == initial_aura);
    CHECK(log.order == std::vector{ observed_event::calculation, observed_event::effect });

    auto copied_execution = observed;
    auto copied_table = observed_table;
    REQUIRE(observed.step(library, observed_table, random) == execution_state::action);
    CHECK(log.order == normal_order);
    CHECK(observed_table[damaged].state().health == normal_table[damaged].state().health);
    CHECK(observed_table[damaged].state().aura == normal_table[damaged].state().aura);
    CHECK(detail::executor_access::stack(observed).size() == detail::executor_access::stack(normal).size());

    REQUIRE(copied_execution.step(library, copied_table, random) == execution_state::action);
    CHECK(copied_table[damaged].state().health == observed_table[damaged].state().health);
    CHECK(copied_table[damaged].state().aura == observed_table[damaged].state().aura);
    CHECK(detail::executor_access::stack(copied_execution).size() == detail::executor_access::stack(observed).size());
}

TEST_CASE("earlier lethal damage supersedes an explicit terminal instruction", "[deal_damage][observation][game-result]")
{
    const bool observed = GENERATE(false, true);
    const auto initial_health = GENERATE(1u, 10u);
    CAPTURE(observed, initial_health);
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr character_id source{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id damaged{ .player_id = player_id{ 1 }, .index = 0 };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{
            deal_damage{
                .source = source, .target = damaged, .value = 4, .type = damage_type::physical
            },
            end_game{ .result = game_result::player_1_win }
        },
        std::tuple{ stop_execution{} },
        character_source
    );
    const auto definition = ids.get_id<character_view>(character_source.name());
    card_table table{};
    auto& mutable_table = detail::executor_access::unrestricted(table);
    mutable_table[player_id{ 0 }].add(definition, { .max_health = 10, .health = 10 });
    mutable_table[player_id{ 1 }].add(definition, { .max_health = 10, .health = initial_health });
    executor execution;
    execution.enter_entry(library);
    zero_random random;

    auto state = observed ? execution.step(library, table, random) : execution.run(library, table, random);
    if(observed)
    {
        REQUIRE(state == execution_state::health_reduced);
        const auto health = execution.view_in<execution_state::health_reduced>();
        CHECK(table[damaged].state().health == (initial_health == 1 ? 0 : 6));
        CHECK(health.value() == 4);
        state = execution.step(library, table, random);
    }
    REQUIRE(state == execution_state::finished);
    const auto expected_result = initial_health == 1
        ? game_result::player_0_win
        : game_result::player_1_win;
    CHECK(execution.view_in<execution_state::finished>().result() == expected_result);
}

TEST_CASE("damage observation retains overkill damage after health reaches zero", "[deal_damage][observation]")
{
    damage_log log;
    const damage_observer_source observer_source{ &log };
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr support_id source{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id damaged{ .player_id = player_id{ 1 }, .index = 0 };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{ deal_damage{
            .source = source, .target = damaged, .value = 999, .type = damage_type::physical
        } },
        std::tuple{ stop_execution{} },
        observer_source, character_source
    );
    const auto definition = ids.get_id<character_view>(character_source.name());
    card_table table{};
    auto& mutable_table = detail::executor_access::unrestricted(table);
    mutable_table[player_id{ 0 }].add(ids.get_id<support_view>(observer_source.name()), { .count = 1 });
    mutable_table[player_id{ 0 }].add(definition, { .max_health = 10, .health = 10 });
    mutable_table[player_id{ 1 }].add(definition, { .max_health = 10, .health = 1 });
    executor execution;
    execution.enter_entry(library);
    zero_random random;

    REQUIRE(execution.step(library, table, random) == execution_state::health_reduced);
    const auto damage = execution.view_in<execution_state::health_reduced>();
    CHECK(damage.target() == damaged);
    CHECK(damage.value() == 999);
    CHECK(table[damaged].state().health == 0);
    CHECK(log.order == std::vector{ observed_event::calculation, observed_event::effect });

    REQUIRE(execution.step(library, table, random) == execution_state::finished);
    CHECK(log.after_damage_value == 999);
    CHECK(log.order == std::vector{
        observed_event::calculation, observed_event::effect, observed_event::after_damage
    });
    CHECK(execution.view_in<execution_state::finished>().result() == game_result::player_0_win);
}

TEST_CASE("zero damage skips health observation while preserving element and after-damage work", "[deal_damage][observation]")
{
    auto type = damage_type::physical;
    std::uint32_t value = 0;
    damage_log log;
    SECTION("physical damage") {}
    SECTION("elemental damage still applies its element") { type = damage_type::pyro; }
    SECTION("a damage effect reduces the final damage to zero")
    {
        value = 3;
        log.effect_reduction = 3;
    }

    const damage_observer_source observer_source{ &log };
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr support_id source{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id damaged{ .player_id = player_id{ 1 }, .index = 0 };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{ deal_damage{ .source = source, .target = damaged, .value = value, .type = type } },
        std::tuple{ stop_execution{} },
        observer_source, character_source
    );
    const auto definition = ids.get_id<character_view>(character_source.name());
    card_table table{};
    auto& mutable_table = detail::executor_access::unrestricted(table);
    mutable_table[player_id{ 0 }].add(ids.get_id<support_view>(observer_source.name()), { .count = 1 });
    mutable_table[player_id{ 0 }].add(definition, { .max_health = 10, .health = 10 });
    mutable_table[player_id{ 1 }].add(definition, { .max_health = 10, .health = 10 });
    executor execution;
    execution.enter_entry(library);
    zero_random random;

    REQUIRE(execution.step(library, table, random) == execution_state::action);
    CHECK(table[damaged].state().health == 10);
    CHECK(table[damaged].state().aura == (type == damage_type::pyro ? element_aura::pyro : element_aura::none));
    CHECK(log.order == std::vector{
        observed_event::calculation, observed_event::effect, observed_event::after_damage
    });
    CHECK(log.after_damage_value == 0);
}
