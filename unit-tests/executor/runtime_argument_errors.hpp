#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/givm.hpp>

#include "../table/test_definition_library.hpp"

#ifndef NDEBUG
namespace givm_test::executor::runtime_argument_errors
{
    enum class scenario
    {
        invalid_character,
        removed_summon,
        duplicate_deck_card,
        insufficient_dice,
        invalid_initial_damage,
        invalid_modified_damage,
        generate_then_remove_dice
    };

    inline givm::dice_counts one_die()
    {
        givm::dice_counts result{};
        result[givm::elemental_dice::cryo] = 1;
        return result;
    }

    struct argument_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            scenario selected;
            givm::program_entry effect;
            givm::program_entry create;
            bool* observed;
        };

        scenario selected;
        bool* observed;

        std::string_view name() const { return "RuntimeArgumentSource"; }
        auto summon_dependencies() const
        {
            return std::array<std::string_view, 1>{ "RuntimeArgumentSummon" };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto summon = context.resolve_id<givm::summon_view>("RuntimeArgumentSummon");
            const auto create = context.add_program(std::tuple{
                givm::add_summon{ .definition = summon, .state = { .value = 1, .usages = 1 } }
            });
            std::vector<givm::any_command> commands;
            switch(selected)
            {
            case scenario::invalid_character: commands.emplace_back(givm::set_energy{}); break;
            case scenario::removed_summon:
                commands.emplace_back(givm::remove_summon{});
                commands.emplace_back(givm::set_summon_state{});
                break;
            case scenario::duplicate_deck_card: commands.emplace_back(givm::draw_cards{}); break;
            case scenario::insufficient_dice: commands.emplace_back(givm::remove_dice{}); break;
            case scenario::invalid_initial_damage:
            case scenario::invalid_modified_damage: commands.emplace_back(givm::deal_damage{}); break;
            case scenario::generate_then_remove_dice:
                commands.emplace_back(givm::add_dice{ .player = givm::relative_player::self, .dice = one_die() });
                commands.emplace_back(givm::remove_dice{});
                break;
            }
            return { selected, context.add_program(commands), create, observed };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .max_energy = 3, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            givm::round_started&, givm::handle_context& context)
        {
            switch(data.selected)
            {
            case scenario::invalid_character:
            {
                auto target = self.id();
                target.index += 20;
                return context.invoke(data.effect, givm::set_energy_input{ target, 1 });
            }
            case scenario::removed_summon:
            {
                auto summons = self.player().summons();
                if(summons.begin() == summons.end()) return context.invoke(data.create);
                const auto id = (*summons.begin()).id();
                const std::array ids{ id };
                const std::array changes{ givm::set_summon_state_input::change{ id, { .value = 2, .usages = 2 } } };
                return context.invoke(data.effect, givm::remove_summon_input{ ids }, givm::set_summon_state_input{ changes });
            }
            case scenario::duplicate_deck_card:
            {
                auto cards = self.player().deck_cards();
                const auto id = (*cards.begin()).id();
                const std::array ids{ id, id };
                return context.invoke(data.effect, givm::draw_cards_input{ ids });
            }
            case scenario::insufficient_dice:
            case scenario::generate_then_remove_dice:
                *data.observed = self.player().state().dice.total() == 0;
                return context.invoke(data.effect, givm::remove_dice_input{ self.player().id(), one_die() });
            case scenario::invalid_initial_damage:
            case scenario::invalid_modified_damage:
            {
                const std::array damages{ givm::damage{
                    .source = self.id(), .target = self.id(), .value = 1,
                    .multiplier_denominator = static_cast<std::uint16_t>(data.selected == scenario::invalid_initial_damage ? 0 : 1),
                    .type = givm::damage_type::physical
                } };
                return context.invoke(data.effect, givm::deal_damage_input{ damages });
            }
            }
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::damage_calculation& event, givm::handle_context&)
        {
            if(data.selected == scenario::invalid_modified_damage)
            {
                *data.observed = true;
                event.multiplier_denominator = 0;
            }
            return {};
        }
    };

    template<class TCheck>
    void with_fixture(scenario selected, TCheck check)
    {
        bool observed = false;
        const auto source = givm::test::with_passive_skill(argument_source{ selected, &observed });
        const givm::test::named_definition_source<givm::summon_view> summon{ "RuntimeArgumentSummon" };
        const givm::test::named_definition_source<givm::card_definition> card{ "RuntimeArgumentCard" };
        std::vector<givm::any_command> initialization{ givm::start_round{} };
        if(selected == scenario::removed_summon) initialization.emplace_back(givm::start_round{});
        initialization.emplace_back(givm::end_game{ givm::game_result::both_loss });
        const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
            initialization, std::tuple{}, source, summon, card);
        givm::table table{ {}, { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } } };
        load_deck(table, library, {
            .cards = { ids.get_id<givm::card_definition>(card.name()) },
            .characters = { ids.get_id<givm::character_view>(source.name()) }
        }, {});
        givm::executor execution;
        execution.start(library, table);
        auto random = []() -> std::uint32_t { return 0; };
        check(execution, library, table, random, observed);
    }

    TEST_CASE("runtime argument diagnostics identify out of range and removed entities", "[runtime-input][debug]")
    {
        for(const auto selected : { scenario::invalid_character, scenario::removed_summon })
            with_fixture(selected, [&](auto& execution, const auto& library, auto& table, auto& random, bool&)
            {
                try
                {
                    execution.step(library, table, random);
                    FAIL("invalid runtime entity did not throw");
                }
                catch(const givm::command_input_error& error)
                {
                    const auto* reason = std::get_if<givm::invalid_entity_argument>(&error.reason);
                    REQUIRE(reason);
                    if(selected == scenario::invalid_character)
                    {
                        CHECK(error.command == "set_energy");
                        CHECK(reason->cause == givm::invalid_entity_argument::reason::out_of_range);
                        REQUIRE(std::holds_alternative<givm::character_id>(reason->entity));
                        CHECK(std::get<givm::character_id>(reason->entity).index == 20);
                    }
                    else
                    {
                        CHECK(error.command == "set_summon_state");
                        CHECK(reason->cause == givm::invalid_entity_argument::reason::removed);
                        auto summons = table[givm::player_id{ 0 }].summons();
                        CHECK(summons.begin() == summons.end());
                    }
                    CHECK(std::string{ error.what() }.find(error.command) != std::string::npos);
                }
            });
    }

    TEST_CASE("runtime draw validates the entire ID batch before transferring cards", "[runtime-input][debug][draw]")
    {
        with_fixture(scenario::duplicate_deck_card, [](auto& execution, const auto& library, auto& table, auto& random, bool&)
        {
            try
            {
                execution.step(library, table, random);
                FAIL("duplicate deck card did not throw");
            }
            catch(const givm::command_input_error& error)
            {
                CHECK(error.command == "draw_cards");
                const auto* reason = std::get_if<givm::duplicate_entity_argument>(&error.reason);
                REQUIRE(reason);
                CHECK(reason->first_index == 0);
                CHECK(reason->index == 1);
                CHECK(table[givm::player_id{ 0 }].deck_card_count() == 1);
                CHECK(table[givm::player_id{ 0 }].hand_card_count() == 0);
            }
        });
    }

    TEST_CASE("runtime dice requirements use the state when the command executes", "[runtime-input][debug][dice]")
    {
        with_fixture(scenario::insufficient_dice, [](auto& execution, const auto& library, auto& table, auto& random, bool& observed)
        {
            try
            {
                execution.step(library, table, random);
                FAIL("missing dice did not throw");
            }
            catch(const givm::command_input_error& error)
            {
                CHECK(error.command == "remove_dice");
                const auto* reason = std::get_if<givm::insufficient_dice_argument>(&error.reason);
                REQUIRE(reason);
                CHECK(reason->requested[givm::elemental_dice::cryo] == 1);
                CHECK(reason->available.total() == 0);
                CHECK(observed);
                CHECK(table[givm::player_id{ 0 }].state().dice.total() == 0);
            }
        });
        with_fixture(scenario::generate_then_remove_dice, [](auto& execution, const auto& library, auto& table, auto& random, bool& observed)
        {
            CHECK(execution.step(library, table, random) == givm::execution_state::finished);
            CHECK(observed);
            CHECK(table[givm::player_id{ 0 }].state().dice.total() == 0);
        });
    }

    TEST_CASE("runtime damage validates initial and response modified denominators", "[runtime-input][debug][damage]")
    {
        for(const auto selected : { scenario::invalid_initial_damage, scenario::invalid_modified_damage })
            with_fixture(selected, [&](auto& execution, const auto& library, auto& table, auto& random, bool& observed)
            {
                try
                {
                    execution.step(library, table, random);
                    FAIL("zero damage denominator did not throw");
                }
                catch(const givm::command_input_error& error)
                {
                    CHECK(error.command == "deal_damage");
                    const auto* reason = std::get_if<givm::invalid_numeric_argument>(&error.reason);
                    REQUIRE(reason);
                    CHECK(reason->field == "multiplier_denominator");
                    CHECK(reason->value == 0);
                    CHECK(observed == (selected == scenario::invalid_modified_damage));
                    CHECK(table[givm::character_id{ givm::player_id{ 0 }, 0 }].state().health == 10);
                }
            });
    }
}
#endif
