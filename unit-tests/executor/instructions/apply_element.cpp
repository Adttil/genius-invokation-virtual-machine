#include "../../executor_access.hpp"
#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"

using namespace givm;

namespace
{
    struct reaction_log
    {
        std::vector<int> order;
        bool take_over = false;
        element_aura replacement_aura = element_aura::none;
        element incoming = element::none;
        element_aura reacted_aura = element_aura::none;
        elemental_reaction reaction = elemental_reaction::none;
        element_application_cause cause = element_application_cause::effect;
    };

    struct reaction_observer_source
    {
        using definition_category = support_view;

        struct definition_type
        {
            reaction_log* log;
            program_entry<elemental_reaction_will_occur> replacement_entry;
        };

        reaction_log* log;

        constexpr std::string_view name() const noexcept
        {
            return "ReactionObserver";
        }

        struct replace_reaction_aura
        {
            using context_type = elemental_reaction_will_occur;

            element_aura aura;

            execution_state execute(const definition_library&, card_table& table, detail::execution_context& context, random_fn&) const
            {
                auto&& [broadcast, activation] = context.stack().top<
                    frame<
                        detail::handler_id<elemental_reaction_will_occur>[],
                        stack_count_t,
                        elemental_reaction_will_occur,
                        detail::handler_id<elemental_reaction_will_occur>,
                        detail::stage_t
                    >,
                    frame<detail::execution_context::return_info, detail::stage_t>
                >();
                auto&& [handlers, cursor, event, current_handler, broadcast_stage] = broadcast;
                auto&& [return_info, activation_stage] = activation;
                (void)handlers;
                (void)cursor;
                (void)current_handler;
                (void)broadcast_stage;
                (void)return_info;
                (void)activation_stage;

                table[event.target].state().aura = aura;
                return context.enter_next();
            }
        };

        definition_type compile(definition_compile_context& context) const
        {
            return {
                .log = log,
                .replacement_entry = context.add_program<elemental_reaction_will_occur>(
                    std::tuple{ replace_reaction_aura{ log->replacement_aura } }
                )
            };
        }

        static program_entry<elemental_reaction_will_occur> handle(
            const definition_type& data,
            const support_view&,
            elemental_reaction_will_occur& event,
            const card_table&,
            random_fn&
        )
        {
            data.log->order.push_back(1);
            data.log->incoming = event.incoming_element;
            data.log->reacted_aura = event.reacted_aura;
            data.log->reaction = event.reaction;
            data.log->cause = event.cause;
            if(data.log->take_over)
            {
                event.already_handled = true;
                return data.replacement_entry;
            }
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
            data.log->order.push_back(2);
            return program_entry<after_elemental_reaction>::null();
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

        execution_state execute(const definition_library&, card_table&, detail::execution_context& context, random_fn&) const noexcept
        {
            return context.yield(execution_state::action);
        }
    };

    bool run_until_blocked(const definition_library& library, executor& target, card_table& table)
    {
        zero_random random;
        return target.run(library, table, random) == execution_state::action;
    }
}

TEST_CASE("apply_element stores a non-reactive aura without broadcasting a reaction", "[apply_element]")
{
    reaction_log log;
    const reaction_observer_source observer_source{ &log };
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr support_id observer_entity_id{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id character_entity_id{ .player_id = player_id{ 1 }, .index = 0 };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{
            apply_element{
                .source = observer_entity_id,
                .target = character_entity_id,
                .element = element::hydro
            }
        },
        std::tuple{ stop_execution{} },
        observer_source,
        character_source
    );
    const auto observer_id = id_map.get_id<support_view>(observer_source.name());
    const auto definition_id = id_map.get_id<character_view>(character_source.name());

    card_table table{};
    const auto observer = table[player_id{ 0 }].add(observer_id, { .count = 1 });
    REQUIRE(observer.id() == observer_entity_id);
    const auto character = table[player_id{ 1 }].add(definition_id, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    });
    REQUIRE(character.id() == character_entity_id);

    executor target;
    target.enter_entry(library);

    REQUIRE(run_until_blocked(library, target, table));
    CHECK(character.state().aura == element_aura::hydro);
    CHECK(log.order.empty());
}

TEST_CASE("apply_element broadcasts both sides of a default reaction", "[apply_element]")
{
    reaction_log log;
    const reaction_observer_source observer_source{ &log };
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr support_id observer_entity_id{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id character_entity_id{ .player_id = player_id{ 1 }, .index = 0 };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{
            apply_element{
                .source = observer_entity_id,
                .target = character_entity_id,
                .element = element::pyro,
                .cause = element_application_cause::effect
            }
        },
        std::tuple{ stop_execution{} },
        observer_source,
        character_source
    );
    const auto observer_id = id_map.get_id<support_view>(observer_source.name());
    const auto definition_id = id_map.get_id<character_view>(character_source.name());

    card_table table{};
    const auto observer = table[player_id{ 0 }].add(observer_id, { .count = 1 });
    REQUIRE(observer.id() == observer_entity_id);
    const auto character = table[player_id{ 1 }].add(definition_id, {
        .max_health = 10,
        .max_energy = 3,
        .health = 10,
        .energy = 0,
        .aura = element_aura::cryo
    });
    REQUIRE(character.id() == character_entity_id);

    executor target;
    target.enter_entry(library);

    REQUIRE(run_until_blocked(library, target, table));
    CHECK(log.order == std::vector{ 1, 2 });
    CHECK(log.incoming == element::pyro);
    CHECK(log.reacted_aura == element_aura::cryo);
    CHECK(log.reaction == elemental_reaction::melt);
    CHECK(log.cause == element_application_cause::effect);
    CHECK(character.state().aura == element_aura::none);
}

TEST_CASE("a response can replace apply_element default reaction handling", "[apply_element]")
{
    reaction_log log{
        .take_over = true,
        .replacement_aura = element_aura::dendro
    };
    const reaction_observer_source observer_source{ &log };
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr support_id observer_entity_id{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id character_entity_id{ .player_id = player_id{ 1 }, .index = 0 };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{
            apply_element{
                .source = observer_entity_id,
                .target = character_entity_id,
                .element = element::pyro
            }
        },
        std::tuple{ stop_execution{} },
        observer_source,
        character_source
    );
    const auto observer_id = id_map.get_id<support_view>(observer_source.name());
    const auto definition_id = id_map.get_id<character_view>(character_source.name());

    card_table table{};
    const auto observer = table[player_id{ 0 }].add(observer_id, { .count = 1 });
    REQUIRE(observer.id() == observer_entity_id);
    const auto character = table[player_id{ 1 }].add(definition_id, {
        .max_health = 10,
        .max_energy = 3,
        .health = 10,
        .energy = 0,
        .aura = element_aura::hydro
    });
    REQUIRE(character.id() == character_entity_id);

    executor target;
    target.enter_entry(library);

    REQUIRE(run_until_blocked(library, target, table));
    CHECK(log.order == std::vector{ 1, 2 });
    CHECK(log.reaction == elemental_reaction::vaporize);
    CHECK(character.state().aura == element_aura::dendro);
}

TEST_CASE("step crosses aura changes without an observation stop", "[apply_element][set_element_aura][observation]")
{
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr character_id source{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id affected{ .player_id = player_id{ 1 }, .index = 0 };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{
            set_element_aura{ .target = affected, .aura = element_aura::cryo },
            set_element_aura{ .target = affected, .aura = element_aura::none },
            apply_element{ .source = source, .target = affected, .element = element::hydro }
        },
        std::tuple{ stop_execution{} },
        character_source
    );
    const auto definition = ids.get_id<character_view>(character_source.name());
    card_table table{};
    table[player_id{ 0 }].add(definition, { .max_health = 10, .health = 10 });
    table[player_id{ 1 }].add(definition, { .max_health = 10, .health = 10 });
    auto normal_table = table;
    zero_random random;
    executor normal;
    normal.enter_entry(library);
    REQUIRE(normal.run(library, normal_table, random) == execution_state::action);

    executor observed;
    observed.enter_entry(library);
    REQUIRE(observed.step(library, table, random) == execution_state::action);
    CHECK(table[affected].state().aura == element_aura::hydro);
    CHECK(table[affected].state().aura == normal_table[affected].state().aura);
    CHECK(detail::executor_access::stack(observed).size() == detail::executor_access::stack(normal).size());
}

TEST_CASE("step crosses reaction responses while preserving settlement and broadcast ordering", "[apply_element][observation]")
{
    reaction_log log;
    SECTION("default reaction") {}
    SECTION("a response replaces the default result")
    {
        log.take_over = true;
        log.replacement_aura = element_aura::dendro;
    }

    const reaction_observer_source observer_source{ &log };
    const test::named_definition_source<character_view> character_source{ "Character" };
    constexpr support_id source{ .player_id = player_id{ 0 }, .index = 0 };
    constexpr character_id affected{ .player_id = player_id{ 1 }, .index = 0 };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{ apply_element{ .source = source, .target = affected, .element = element::pyro } },
        std::tuple{ stop_execution{} },
        observer_source, character_source
    );
    card_table table{};
    table[player_id{ 0 }].add(ids.get_id<support_view>(observer_source.name()), { .count = 1 });
    table[player_id{ 1 }].add(ids.get_id<character_view>(character_source.name()), {
        .max_health = 10, .health = 10, .aura = element_aura::hydro
    });
    auto normal_table = table;
    zero_random random;
    executor normal;
    normal.enter_entry(library);
    REQUIRE(normal.run(library, normal_table, random) == execution_state::action);
    const auto normal_order = log.order;
    log.order.clear();

    executor observed;
    observed.enter_entry(library);
    REQUIRE(observed.step(library, table, random) == execution_state::action);
    CHECK(log.order == normal_order);
    CHECK(log.order == std::vector{ 1, 2 });
    CHECK(log.incoming == element::pyro);
    CHECK(log.reacted_aura == element_aura::hydro);
    CHECK(log.reaction == elemental_reaction::vaporize);
    CHECK(log.cause == element_application_cause::effect);
    CHECK(table[affected].state().aura == (log.take_over ? element_aura::dendro : element_aura::none));
    CHECK(table[affected].state().aura == normal_table[affected].state().aura);
    CHECK(detail::executor_access::stack(observed).size() == detail::executor_access::stack(normal).size());
}
