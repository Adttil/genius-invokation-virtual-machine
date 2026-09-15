#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"
#include "../test_character_source.hpp"

namespace
{
    struct reaction_log
    {
        std::vector<int> order;
        bool take_over = false;
        givm::element_aura replacement_aura = givm::element_aura::none;
        givm::element incoming = givm::element::none;
        givm::element_aura reacted_aura = givm::element_aura::none;
        givm::elemental_reaction reaction = givm::elemental_reaction::none;
        givm::element_application_cause cause = givm::element_application_cause::effect;
    };

    struct reaction_observer_source
    {
        using definition_category = givm::character_view;

        struct definition_type
        {
            reaction_log* log;
            givm::program_entry<givm::elemental_reaction_will_occur> replacement_entry;
        };

        reaction_log* log;

        constexpr std::string_view name() const noexcept
        {
            return "ReactionObserver";
        }

        definition_type compile(givm::definition_compile_context& context) const
        {
            return {
                .log = log,
                .replacement_entry = context.add_program<givm::elemental_reaction_will_occur>(
                    std::tuple{ givm::set_element_aura{ .target = givm::character_id{ givm::player_id{ 1 }, 0 }, .aura = log->replacement_aura } }
                )
            };
        }

        static givm::program_entry<givm::elemental_reaction_will_occur> handle(
            const definition_type& data,
            const givm::character_view&,
            givm::elemental_reaction_will_occur& event,
            const givm::table&,
            givm::random_fn&
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
            return givm::program_entry<givm::elemental_reaction_will_occur>::null();
        }

        static givm::program_entry<givm::after_elemental_reaction> handle(
            const definition_type& data,
            const givm::character_view&,
            givm::after_elemental_reaction&,
            const givm::table&,
            givm::random_fn&
        )
        {
            data.log->order.push_back(2);
            return givm::program_entry<givm::after_elemental_reaction>::null();
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
    givm::element incoming = givm::element::hydro;
    givm::element_aura initial_aura = givm::element_aura::none;
    givm::element_aura expected_aura = givm::element_aura::hydro;
    SECTION("non-reactive application") {}
    SECTION("default reaction")
    {
        initial_aura = givm::element_aura::cryo;
        incoming = givm::element::pyro;
        expected_aura = givm::element_aura::none;
    }
    SECTION("response replaces the default reaction result")
    {
        initial_aura = givm::element_aura::hydro;
        incoming = givm::element::pyro;
        log.take_over = true;
        log.replacement_aura = givm::element_aura::dendro;
        expected_aura = givm::element_aura::dendro;
    }
    const reaction_observer_source observer{ &log };
    const givm::test::initialized_character_source victim{ "Victim" };
    constexpr givm::character_id source{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id affected{ givm::player_id{ 1 }, 0 };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{
            givm::initialize_characters{ givm::player_id{ 1 } },
            givm::set_element_aura{ .target = affected, .aura = initial_aura },
            givm::apply_element{ .source = source, .target = affected, .element = incoming },
            givm::end_game{ .result = givm::game_result::both_loss }
        }, std::tuple{}, observer, victim
    );
    givm::table table;
    table.load_deck(givm::player_id{ 0 }, { .characters = { ids.get_id<givm::character_view>(observer.name()) } });
    table.load_deck(givm::player_id{ 1 }, { .characters = { ids.get_id<givm::character_view>(victim.name()) } });
    givm::executor target;
    target.enter_entry(library);
    zero_random random;
    REQUIRE(target.step(library, table, random)
        == givm::execution_state::finished);
    CHECK(table[affected].state().aura == expected_aura);
    if(initial_aura == givm::element_aura::none)
    {
        CHECK(log.order.empty());
    }
    else
    {
        CHECK(log.order == std::vector{ 1, 2 });
        CHECK(log.incoming == givm::element::pyro);
        CHECK(log.reacted_aura == initial_aura);
        CHECK(log.reaction == (initial_aura == givm::element_aura::cryo
            ? givm::elemental_reaction::melt : givm::elemental_reaction::vaporize));
        CHECK(log.cause == givm::element_application_cause::effect);
    }
}
