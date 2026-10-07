#include <cstdint>
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
            static constexpr auto category = givm::definition_category::skill;
            struct definition_type { givm::normal_effect entry; };
            sequence_form form;

            constexpr std::string_view name() const noexcept { return "CompileBoundarySkill"; }

            definition_type compile(givm::definition_compile_context& context) const
            {
                const givm::set_energy energy{};
                const givm::draw_cards draw{ .position = 1 };
                const givm::draw_cards next_draw{ .position = 0 };
                const givm::add_dice dice{};
                const std::array<givm::any_command, 4> commands{ energy, draw, next_draw, dice };
                context.add_normal_effect();
                switch(form)
                {
                case sequence_form::span:
                    return { context.add_normal_effect(std::span<const givm::any_command>{ commands }) };
                case sequence_form::array:
                    return { context.add_normal_effect(commands) };
                case sequence_form::vector:
                    return { context.add_normal_effect(std::vector<givm::any_command>{ energy, draw, next_draw, dice }) };
                case sequence_form::tuple:
                    return { context.add_normal_effect(std::tuple{ energy, draw, next_draw, dice }) };
                case sequence_form::list:
                    return { context.add_normal_effect(std::list<givm::any_command>{ energy, draw, next_draw, dice }) };
                case sequence_form::subset_variant:
                {
                    using command = std::variant<givm::set_energy, givm::draw_cards, givm::add_dice>;
                    return { context.add_normal_effect(std::vector<command>{ energy, draw, next_draw, dice }) };
                }
                case sequence_form::variadic:
                    return { context.add_normal_effect(energy, draw, next_draw, dice) };
                }
                throw std::logic_error{ "invalid test command sequence form" };
            }

            static givm::normal_effect handle(const definition_type& definition,
                givm::round_started&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
            {
                const auto self = context.entity();
                givm::dice_counts dice;
                dice[givm::elemental_dice::pyro] = 3;
                return context.invoke(definition.entry,
                    givm::set_energy_input{ self.character().id(), 2 }, givm::add_dice_input{ self.player().id(), dice });
            }
        };

        struct character_source
        {
            static constexpr auto category = givm::definition_category::character;
            struct definition_type { givm::optional_definition_id<givm::definition_category::skill> skill; };

            constexpr std::string_view name() const noexcept { return "CompileBoundaryCharacter"; }
            constexpr auto skill_dependencies() const noexcept
            {
                return std::array<std::string_view, 1>{ "CompileBoundarySkill" };
            }
            definition_type compile(givm::definition_compile_context& context) const
            {
                return { context.resolve_id<givm::definition_category::skill>("CompileBoundarySkill") };
            }
            static givm::character_state query(const definition_type&, const givm::character_initial_state&)
            {
                return { .max_health = 10, .max_energy = 3, .health = 10 };
            }
            static givm::optional_definition_id<givm::definition_category::skill> query(const definition_type& definition,
                const givm::character_initial_skill& query)
            {
                return query.skill_index == 0 ? definition.skill : givm::optional_definition_id<givm::definition_category::skill>{};
            }
        };
    }

    std::expected<givm::definition_compile_result, std::vector<givm::compile_error>> make_program(
        sequence_form form, givm::compile_mode mode)
    {
        const character_source character;
        const skill_source skill{ form };
        const givm_test::reaction_source<givm::definition_category::card> card_a{ "CompileBoundaryCardA" };
        const givm_test::reaction_source<givm::definition_category::card> card_b{ "CompileBoundaryCardB" };
        auto sources = givm::make_definition_source_library(character, skill, card_a, card_b);
        if(not sources) throw std::logic_error{ givm::error_string(sources.error()) };
        if(not sources->add(givm_test::dendro_core, givm_test::catalyzing_field,
            givm_test::burning_flame, givm_test::frozen, givm_test::shield))
            throw std::logic_error{ "invalid reaction dependencies" };
        std::apply([&](const auto&... reaction) { sources->add(reaction...); }, givm_test::basic_reactions);
        const std::array<givm::any_command, 3> round{
            givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::player_0_win } };
        if(form == sequence_form::array)
        {
            const std::array<std::string_view, 1> character_names{ character.name() };
            const std::array<std::string_view, 2> card_names{ card_a.name(), card_b.name() };
            givm::definition_selection selection{};
            selection[static_cast<std::size_t>(givm::definition_category::character)] = character_names;
            selection[static_cast<std::size_t>(givm::definition_category::card)] = card_names;
            const std::array<givm::any_command, 0> initialization{};
            const std::vector<givm::any_command> round_commands(round.begin(), round.end());
            return givm::compile(*sources, givm_test::basic_sources, selection, initialization, round_commands, mode);
        }
        return givm::compile(*sources, givm_test::basic_sources, std::span<const givm::any_command>{},
            std::span<const givm::any_command>{ round }, mode);
    }
}
