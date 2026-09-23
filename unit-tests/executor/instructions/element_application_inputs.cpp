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
    struct element_input_log
    {
        std::vector<int> order;
    };

    struct element_input_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            element_input_log* log;
            bool dynamic;
            givm::program_entry application;
            givm::program_entry during_reaction;
            givm::program_entry after_reaction;
        };

        element_input_log* log;
        bool dynamic;
        std::string_view name() const { return "ElementInputSource"; }

        definition_type compile(givm::definition_compile_context& context) const
        {
            constexpr givm::relative_character_target source{
                givm::relative_player::self, 0 };
            constexpr givm::relative_character_target target{
                givm::relative_player::opponent, 0 };
            const auto application = dynamic
                ? context.add_program(std::tuple{ givm::apply_element{}, givm::apply_element{}, givm::apply_element{} })
                : context.add_program(std::tuple{
                    givm::apply_element{ .source = source, .target = target, .element = givm::element::pyro },
                    givm::apply_element{ .source = source, .target = target, .element = givm::element::none },
                    givm::apply_element{ .source = source, .target = target, .element = givm::element::cryo }
                });
            return { log, dynamic, application,
                context.add_program(std::tuple{ givm::apply_element{}, givm::apply_element{}, givm::replace_cards{ givm::player_id{ 0 } } }),
                context.add_program(std::tuple{ givm::apply_element{} }) };
        }

        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }

        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            givm::test_event&, givm::handle_context& context)
        {
            data.log->order.push_back(1);
            if(not data.dynamic) return context.invoke(data.application);
            const auto target = (*context.table()[givm::player_id{ 1 }].characters().begin()).id();
            return context.invoke(data.application,
                givm::element_application{ .source = self.id(), .target = target, .element = givm::element::pyro },
                givm::element_application{ .source = self.id(), .target = target, .element = givm::element::none },
                givm::element_application{ .source = self.id(), .target = target, .element = givm::element::cryo });
        }

        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            givm::elemental_reaction_will_occur& event, givm::handle_context& context)
        {
            data.log->order.push_back(2);
            CHECK(event.reaction == givm::elemental_reaction::vaporize);
            CHECK(event.reacted_aura == givm::element_aura::hydro);
            CHECK(context.table()[event.target].state().aura == givm::element_aura::hydro);
            return context.invoke(data.during_reaction,
                givm::element_application{ .source = self.id(), .target = event.target, .element = givm::element::none },
                givm::element_application{ .source = self.id(), .target = event.target, .element = givm::element::dendro });
        }

        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            givm::after_elemental_reaction& event, givm::handle_context& context)
        {
            data.log->order.push_back(3);
            CHECK(event.reaction == givm::elemental_reaction::vaporize);
            CHECK(event.reacted_aura == givm::element_aura::hydro);
            CHECK(context.table()[event.target].state().aura == givm::element_aura::none);
            return context.invoke(data.after_reaction,
                givm::element_application{ .source = self.id(), .target = event.target, .element = givm::element::electro });
        }
    };

    struct zero_random
    {
        std::uint32_t operator()() const { return 0; }
    };
}

TEST_CASE("element application inputs resume nested responses with the same result as fixed commands", "[apply_element][inputs]")
{
    const bool observed = GENERATE(false, true);
    const bool dynamic = GENERATE(false, true);
    element_input_log log;
    const auto source = givm::test::with_passive_skill(element_input_source{ &log, dynamic });
    const givm::test::initialized_character_source target{ "ElementInputTarget",
        { .max_health = 10, .health = 10, .aura = givm::element_aura::hydro } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{
            givm::test_command{},
            givm::end_game{ givm::game_result::both_loss }
        }, std::tuple{}, source, target);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library,
        { .characters = { ids.get_id<givm::character_view>(source.name()) } },
        { .characters = { ids.get_id<givm::character_view>(target.name()) } });
    const auto target_id = (*table[givm::player_id{ 1 }].characters().begin()).id();
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    auto state = executor.step(library, table, random);
    REQUIRE(state == givm::execution_state::card_selection);
    CHECK(log.order == std::vector{ 1, 2 });
    CHECK(table[target_id].state().aura == givm::element_aura::dendro);
    CHECK(table[target_id].state().health == 10);
    executor.view_in<givm::execution_state::card_selection>().select({});
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
    CHECK(log.order == std::vector{ 1, 2, 3 });
    CHECK(table[target_id].state().aura == givm::element_aura::cryo);
    CHECK(table[target_id].state().health == 10);
}
