#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/executor/instructions/apply_element.hpp>

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

            bool execute(card_table& table, execution_context& context, random_fn&) const
            {
                auto&& [broadcast, activation] = context.stack().top<
                    frame<
                        detail::handler_id<elemental_reaction_will_occur>[],
                        stack_count_t,
                        elemental_reaction_will_occur,
                        detail::handler_id<elemental_reaction_will_occur>,
                        stage_t
                    >,
                    frame<execution_context::return_info, stage_t>
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

        bool execute(card_table&, execution_context& context, random_fn&) const noexcept
        {
            return context.yield();
        }
    };

    bool run_until_blocked(executor& target, card_table& table)
    {
        zero_random random;
        while(target.execute_next(table, random))
        {
        }
        return table.definition_library().instruction(target.position()).is<stop_execution>();
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

    card_table table{ library };
    const auto observer = table[player_id{ 0 }].add(observer_id, { .count = 1 });
    REQUIRE(observer.id() == observer_entity_id);
    const auto character = table[player_id{ 1 }].add(definition_id, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    });
    REQUIRE(character.id() == character_entity_id);

    executor target;
    target.enter_entry(table.definition_library());

    REQUIRE(run_until_blocked(target, table));
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

    card_table table{ library };
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
    target.enter_entry(table.definition_library());

    REQUIRE(run_until_blocked(target, table));
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

    card_table table{ library };
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
    target.enter_entry(table.definition_library());

    REQUIRE(run_until_blocked(target, table));
    CHECK(log.order == std::vector{ 1, 2 });
    CHECK(log.reaction == elemental_reaction::vaporize);
    CHECK(character.state().aura == element_aura::dendro);
}
