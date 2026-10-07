#include <cstdint>
#include <givm/definition_source.hpp>

#include "interface_layer_sources.hpp"

#include <array>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace givm_test::interface_layers
{
    namespace
    {
        struct skill_source
        {
            static constexpr auto category = givm::definition_category::skill;
            struct definition_type { givm::normal_effect entry; };

            constexpr std::string_view name() const noexcept { return "LayeredSkill"; }

            definition_type compile(givm::definition_compile_context& context) const
            {
                return { context.add_normal_effect(givm::set_energy{}, givm::add_dice{}) };
            }

            static givm::action_cost_requirement query(const definition_type&, const givm::skill_initial_cost&)
            {
                return { .energy = 1 };
            }

            static givm::target_validation query(const definition_type&, const givm::skill_target_validation& query)
            {
                const auto cost = query.library[query.skill.definition_id()].query(givm::skill_initial_cost{});
                return query.target_count == 0 && cost.energy == 1
                    && query.table[query.skill.character().id()].state().energy == 2
                    ? givm::target_validation::valid_complete : givm::target_validation::invalid;
            }

            static givm::normal_effect handle(const definition_type& definition,
                givm::round_started&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
            {
                const auto self = context.entity();
                const auto energy = context.table()[self.character().id()].state().health / 5;
                givm::dice_counts dice;
                dice[givm::elemental_dice::cryo] = 2;
                return context.invoke(definition.entry,
                    givm::set_energy_input{ self.character().id(), energy }, givm::add_dice_input{ self.player().id(), dice });
            }
        };

        struct character_source
        {
            static constexpr auto category = givm::definition_category::character;
            struct definition_type { givm::optional_definition_id<givm::definition_category::skill> skill; };

            constexpr std::string_view name() const noexcept { return "LayeredCharacter"; }
            constexpr auto skill_dependencies() const noexcept
            {
                return std::array<std::string_view, 1>{ "LayeredSkill" };
            }

            definition_type compile(givm::definition_compile_context& context) const
            {
                return { context.resolve_id<givm::definition_category::skill>("LayeredSkill") };
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

        constexpr skill_source skill;
        constexpr character_source character;
    }

    givm::definition_source_library make_layered_sources()
    {
        auto result = givm::make_definition_source_library(character, skill);
        if(not result) throw std::logic_error{ givm::error_string(result.error()) };
        return std::move(*result);
    }
}
