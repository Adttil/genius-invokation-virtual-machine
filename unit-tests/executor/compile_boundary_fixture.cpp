#include "compile_boundary_fixture.hpp"
#include "../test_source_library.hpp"

#include <array>
#include <cstddef>
#include <list>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

namespace givm_test::executor::compile_boundary
{
    namespace
    {
        struct skill_source
        {
            using definition_category = givm::skill_view;
            struct definition_type { givm::program_entry entry; };
            sequence_form form;

            constexpr std::string_view name() const noexcept { return "CompileBoundarySkill"; }

            definition_type compile(givm::definition_compile_context& context) const
            {
                // The fixed command borrows local data only until add_program returns.
                const std::array<std::size_t, 2> positions{ 1, 0 };
                const givm::set_energy energy{};
                const givm::draw_cards draw{ .positions = positions };
                const givm::add_dice dice{};
                const std::array<givm::any_command, 3> commands{ energy, draw, dice };
                context.add_program();
                switch(form)
                {
                case sequence_form::span:
                    return { context.add_program(std::span<const givm::any_command>{ commands }) };
                case sequence_form::array:
                    return { context.add_program(commands) };
                case sequence_form::vector:
                    return { context.add_program(std::vector<givm::any_command>{ energy, draw, dice }) };
                case sequence_form::tuple:
                    return { context.add_program(std::tuple{ energy, draw, dice }) };
                case sequence_form::list:
                    return { context.add_program(std::list<givm::any_command>{ energy, draw, dice }) };
                case sequence_form::subset_variant:
                {
                    using command = std::variant<givm::set_energy, givm::draw_cards, givm::add_dice>;
                    return { context.add_program(std::vector<command>{ energy, draw, dice }) };
                }
                case sequence_form::variadic:
                    return { context.add_program(energy, draw, dice) };
                }
                throw std::logic_error{ "invalid test command sequence form" };
            }

            static givm::program_entry handle(const definition_type& definition, const givm::skill_view& self,
                givm::round_started&, givm::handle_context& context)
            {
                givm::dice_counts dice;
                dice[givm::elemental_dice::pyro] = 3;
                return context.invoke(definition.entry,
                    givm::set_energy_input{ self.character().id(), 2 }, givm::add_dice_input{ self.player().id(), dice });
            }
        };

        struct character_source
        {
            using definition_category = givm::character_view;
            struct definition_type { givm::definition_id<givm::skill_view> skill; };

            constexpr std::string_view name() const noexcept { return "CompileBoundaryCharacter"; }
            constexpr auto skill_dependencies() const noexcept
            {
                return std::array<std::string_view, 1>{ "CompileBoundarySkill" };
            }
            definition_type compile(givm::definition_compile_context& context) const
            {
                return { context.resolve_id<givm::skill_view>("CompileBoundarySkill") };
            }
            static givm::character_state query(const definition_type&, const givm::character_initial_state&)
            {
                return { .max_health = 10, .max_energy = 3, .health = 10 };
            }
            static givm::definition_id<givm::skill_view> query(const definition_type& definition,
                const givm::character_initial_skill& query)
            {
                return query.skill_index == 0 ? definition.skill : givm::definition_id<givm::skill_view>{};
            }
        };
    }

    std::expected<givm::definition_compile_result, std::vector<givm::compile_error>> make_program(
        sequence_form form, givm::compile_mode mode)
    {
        const character_source character;
        const skill_source skill{ form };
        const givm_test::reaction_source<givm::card_definition> card_a{ "CompileBoundaryCardA" };
        const givm_test::reaction_source<givm::card_definition> card_b{ "CompileBoundaryCardB" };
        auto sources = givm::make_definition_source_library(character, skill, card_a, card_b);
        if(not sources) throw std::logic_error{ givm::error_string(sources.error()) };
        const std::array<givm::any_command, 2> round{
            givm::start_round{}, givm::end_game{ givm::game_result::player_0_win } };
        if(form == sequence_form::array)
        {
            const std::array<std::string_view, 1> character_names{ character.name() };
            const std::array<std::string_view, 2> card_names{ card_a.name(), card_b.name() };
            givm::definition_selection selection{};
            selection[givm::definition_types::index_of<givm::character_view>()] = character_names;
            selection[givm::definition_types::index_of<givm::card_definition>()] = card_names;
            const std::array<givm::any_command, 0> initialization{};
            const std::vector<givm::any_command> round_commands(round.begin(), round.end());
            return givm::compile(*sources, givm_test::basic_sources, selection, initialization, round_commands, mode);
        }
        return givm::compile(*sources, givm_test::basic_sources, std::span<const givm::any_command>{},
            std::span<const givm::any_command>{ round }, mode);
    }
}
