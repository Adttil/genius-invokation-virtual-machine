#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"
#include "../test_character_source.hpp"

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
        using definition_category = character_view;

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

        definition_type compile(definition_compile_context& context) const
        {
            return {
                .log = log,
                .replacement_entry = context.add_program<elemental_reaction_will_occur>(
                    std::tuple{ set_element_aura{ .target = character_id{ player_id{ 1 }, 0 }, .aura = log->replacement_aura } }
                )
            };
        }

        static program_entry<elemental_reaction_will_occur> handle(
            const definition_type& data,
            const character_view&,
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
            const character_view&,
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

}

TEST_CASE("apply_element exposes aura changes and both reaction events", "[apply_element]")
{
    const bool observed = GENERATE(false, true);
    reaction_log log;
    element incoming = element::hydro;
    element_aura initial_aura = element_aura::none;
    element_aura expected_aura = element_aura::hydro;
    SECTION("non-reactive application") {}
    SECTION("default reaction")
    {
        initial_aura = element_aura::cryo;
        incoming = element::pyro;
        expected_aura = element_aura::none;
    }
    SECTION("response replaces the default reaction result")
    {
        initial_aura = element_aura::hydro;
        incoming = element::pyro;
        log.take_over = true;
        log.replacement_aura = element_aura::dendro;
        expected_aura = element_aura::dendro;
    }
    const reaction_observer_source observer{ &log };
    const test::initialized_character_source victim{ "Victim" };
    constexpr character_id source{ player_id{ 0 }, 0 };
    constexpr character_id affected{ player_id{ 1 }, 0 };
    const auto [library, ids] = test::compile_definitions_with_program(
        std::tuple{
            initialize_characters{ player_id{ 1 } },
            set_element_aura{ .target = affected, .aura = initial_aura },
            apply_element{ .source = source, .target = affected, .element = incoming },
            end_game{ .result = game_result::both_loss }
        }, std::tuple{}, observer, victim
    );
    card_table table;
    table.load_deck(player_id{ 0 }, { .characters = { ids.get_id<character_view>(observer.name()) } });
    table.load_deck(player_id{ 1 }, { .characters = { ids.get_id<character_view>(victim.name()) } });
    executor target;
    target.enter_entry(library);
    zero_random random;
    REQUIRE((observed ? target.step(library, table, random) : target.run(library, table, random))
        == execution_state::finished);
    CHECK(table[affected].state().aura == expected_aura);
    if(initial_aura == element_aura::none)
    {
        CHECK(log.order.empty());
    }
    else
    {
        CHECK(log.order == std::vector{ 1, 2 });
        CHECK(log.incoming == element::pyro);
        CHECK(log.reacted_aura == initial_aura);
        CHECK(log.reaction == (initial_aura == element_aura::cryo
            ? elemental_reaction::melt : elemental_reaction::vaporize));
        CHECK(log.cause == element_application_cause::effect);
    }
}
