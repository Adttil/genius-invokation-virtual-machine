#ifndef GIVM_EXECUTOR_VIEWS_ACTION_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_ACTION_SELECTION_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>
#include <utility>

#include "../executor.hpp"
#include "../commands/begin_action.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    enum class switch_payment_validation : std::uint8_t
    {
        valid,
        requirement_mismatch,
        insufficient_dice,
        energy_tag_mismatch,
        insufficient_energy
    };

    enum class card_payment_validation : std::uint8_t
    {
        valid,
        requirement_mismatch,
        insufficient_dice,
        energy_tag_mismatch,
        insufficient_energy
    };

    enum class skill_payment_validation : std::uint8_t
    {
        valid,
        requirement_mismatch,
        insufficient_dice,
        energy_tag_mismatch,
        insufficient_energy
    };

    enum class technique_payment_validation : std::uint8_t
    {
        valid,
        requirement_mismatch,
        insufficient_dice,
        energy_tag_mismatch,
        insufficient_energy
    };

    enum class elemental_tuning_dice_validation : std::uint8_t
    {
        valid,
        omni_not_allowed,
        missing_character_element,
        same_element,
        insufficient_dice
    };

    template<>
    class execution_view<execution_state::action_selection>
    {
    public:
        bool is_controlled(const definition_library& library, const table& card_table) const noexcept
        {
            const auto active = *card_table[card_table.state().active_player].state().active_character;
            return library.is_controlled(card_table[active]);
        }

        constexpr std::size_t switch_target_count() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<
                cost_of_switch[],
                program_entry[], std::size_t[], std::size_t[],
                stack_count_t,
                detail::action_selection, substack_t
            >()).size();
        }

        constexpr const cost_of_switch& switch_cost(std::size_t target_index) const noexcept
        {
            return get<0>(std::as_const(*stack_).top<
                cost_of_switch[],
                program_entry[], std::size_t[], std::size_t[],
                stack_count_t,
                detail::action_selection, substack_t
            >())[target_index];
        }

        constexpr character_id switch_target(std::size_t target_index) const noexcept
        {
            return switch_cost(target_index).target;
        }

        const cost_of_switch& calculate_switch_cost(
            const definition_library& library, const table& card_table,
            std::size_t target_index
        ) const
        {
            return detail::calculate_switch_cost(
                library, target_index, card_table, *stack_
            );
        }

        constexpr switch_payment_validation switch_payment_validate(
            const table& card_table, std::size_t target_index, const dice_counts& paid_dice
        ) const noexcept
        {
            const auto& cost = switch_cost(target_index);
            if(not payment_matches(cost.requirement.dice_requirement, paid_dice))
            {
                return switch_payment_validation::requirement_mismatch;
            }
            const auto player = card_table.state().active_player;
            if(not card_table[player].state().dice.contains(paid_dice))
            {
                return switch_payment_validation::insufficient_dice;
            }
            const auto active = *card_table[player].state().active_character;
            const auto& state = card_table[active].state();
            if(cost.requirement.energy != 0 && state.energy_tag != cost.requirement.energy_tag)
            {
                return switch_payment_validation::energy_tag_mismatch;
            }
            if(state.energy < cost.requirement.energy)
            {
                return switch_payment_validation::insufficient_energy;
            }
            return switch_payment_validation::valid;
        }

        constexpr void switch_active_character(std::size_t target_index, const dice_counts& paid_dice) const noexcept
        {
            // Assign the complete variant through its trivial assignment operator.
            get<0>(stack_->top<detail::action_selection, substack_t>()) = detail::action_selection{
                detail::switch_selection{ .switch_cost_index = target_index, .paid_dice = paid_dice }
            };
        }

        void switch_active_character(
            const definition_library& library, const table& card_table,
            std::size_t target_index, const dice_counts& paid_dice
        ) const
        {
            detail::calculate_switch_cost(library, target_index, card_table, *stack_);
            // Assign the complete variant through its trivial assignment operator.
            get<0>(stack_->top<detail::action_selection, substack_t>()) = detail::action_selection{
                detail::switch_selection{ .switch_cost_index = target_index, .paid_dice = paid_dice }
            };
        }

        constexpr std::size_t card_count() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<
                cost_of_card[], program_entry[], std::size_t[], std::size_t[],
                detail::switch_handler_id[],
                cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
                stack_count_t, detail::action_selection, substack_t
            >()).size();
        }

        constexpr const cost_of_card& card_cost(std::size_t card_index) const noexcept
        {
            return get<0>(std::as_const(*stack_).top<
                cost_of_card[], program_entry[], std::size_t[], std::size_t[],
                detail::switch_handler_id[],
                cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
                stack_count_t, detail::action_selection, substack_t
            >())[card_index];
        }

        constexpr hand_card_id card_id(std::size_t card_index) const noexcept
        {
            return card_cost(card_index).card;
        }

        const cost_of_card& calculate_card_cost(
            const definition_library& library, const table& card_table, std::size_t card_index
        ) const
        {
            return detail::calculate_card_cost(library, card_index, card_table, *stack_);
        }

        constexpr card_payment_validation card_payment_validate(
            const table& card_table, std::size_t card_index, const dice_counts& paid_dice
        ) const noexcept
        {
            const auto& cost = card_cost(card_index);
            if(not payment_matches(cost.requirement.dice_requirement, paid_dice))
            {
                return card_payment_validation::requirement_mismatch;
            }
            if(not card_table[cost.card.player_id].state().dice.contains(paid_dice))
            {
                return card_payment_validation::insufficient_dice;
            }
            const auto active = *card_table[cost.card.player_id].state().active_character;
            const auto& state = card_table[active].state();
            if(cost.requirement.energy != 0 && state.energy_tag != cost.requirement.energy_tag)
            {
                return card_payment_validation::energy_tag_mismatch;
            }
            if(state.energy < cost.requirement.energy)
            {
                return card_payment_validation::insufficient_energy;
            }
            return card_payment_validation::valid;
        }

        target_validation card_targets_validate(
            const definition_library& library, const table& card_table,
            std::size_t card_index, std::span<const card_target_id> targets = {}
        ) const
        {
            const auto id = card_id(card_index);
            const auto entity = card_table[id];
            const auto definition = library[entity.definition_id()];
            std::array<card_target_id, 2> selected_targets{};
            const auto target_count = std::min(targets.size(), selected_targets.size());
            for(std::size_t index = 0; index < target_count; ++index)
            {
                selected_targets[index] = targets[index];
            }
            return definition.query(card_target_validation{
                .card = entity, .table = card_table, .library = library,
                .targets = selected_targets, .target_count = target_count
            });
        }

        constexpr void play_card(
            std::size_t card_index, const dice_counts& paid_dice, std::span<const card_target_id> targets = {}
        ) const noexcept
        {
            std::array<card_target_id, 2> selected_targets{};
            const auto target_count = std::min(targets.size(), selected_targets.size());
            for(std::size_t index = 0; index < target_count; ++index)
            {
                selected_targets[index] = targets[index];
            }
            get<0>(stack_->top<detail::action_selection, substack_t>()) = detail::action_selection{
                detail::card_selection{
                    .card_cost_index = card_index, .targets = selected_targets, .paid_dice = paid_dice
                }
            };
        }

        void play_card(
            const definition_library& library, const table& card_table,
            std::size_t card_index, const dice_counts& paid_dice, std::span<const card_target_id> targets = {}
        ) const
        {
            detail::calculate_card_cost(library, card_index, card_table, *stack_);
            play_card(card_index, paid_dice, targets);
        }

        constexpr bool elemental_tuning_card_validate(const table& card_table, std::size_t card_index) const noexcept
        {
            return card_table[card_id(card_index)].state().elemental_tuning_allowed;
        }

        constexpr elemental_tuning_dice_validation elemental_tuning_dice_validate(
            const table& card_table, elemental_dice from
        ) const noexcept
        {
            if(from == elemental_dice::omni)
            {
                return elemental_tuning_dice_validation::omni_not_allowed;
            }
            const auto& player = card_table[card_table.state().active_player].state();
            const auto active_element = card_table[*player.active_character].state().element;
            if(active_element == element::none)
            {
                return elemental_tuning_dice_validation::missing_character_element;
            }
            if(from == static_cast<elemental_dice>(active_element))
            {
                return elemental_tuning_dice_validation::same_element;
            }
            if(player.dice[from] == 0)
            {
                return elemental_tuning_dice_validation::insufficient_dice;
            }
            return elemental_tuning_dice_validation::valid;
        }

        constexpr void elemental_tuning(std::size_t card_index, elemental_dice from) const noexcept
        {
            get<0>(stack_->top<detail::action_selection, substack_t>()) = detail::action_selection{
                detail::elemental_tuning_selection{ .card_index = card_index, .from = from }
            };
        }

        constexpr std::size_t skill_count() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<
                cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
                detail::card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
                detail::switch_handler_id[],
                cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
                stack_count_t, detail::action_selection, substack_t
            >()).size();
        }

        constexpr const cost_of_skill& skill_cost(std::size_t skill_index) const noexcept
        {
            return get<0>(std::as_const(*stack_).top<
                cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
                detail::card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
                detail::switch_handler_id[],
                cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
                stack_count_t, detail::action_selection, substack_t
            >())[skill_index];
        }

        constexpr givm::skill_id skill_id(std::size_t skill_index) const noexcept
        {
            return skill_cost(skill_index).skill;
        }

        const cost_of_skill& calculate_skill_cost(
            const definition_library& library, const table& card_table, std::size_t skill_index
        ) const
        {
            return detail::calculate_skill_cost(library, skill_index, card_table, *stack_);
        }

        constexpr skill_payment_validation skill_payment_validate(
            const table& card_table, std::size_t skill_index, const dice_counts& paid_dice
        ) const noexcept
        {
            const auto& cost = skill_cost(skill_index);
            if(not payment_matches(cost.requirement.dice_requirement, paid_dice))
            {
                return skill_payment_validation::requirement_mismatch;
            }
            if(not card_table[cost.skill.character_id.player_id].state().dice.contains(paid_dice))
            {
                return skill_payment_validation::insufficient_dice;
            }
            const auto active = *card_table[cost.skill.character_id.player_id].state().active_character;
            const auto& state = card_table[active].state();
            if(cost.requirement.energy != 0 && state.energy_tag != cost.requirement.energy_tag)
            {
                return skill_payment_validation::energy_tag_mismatch;
            }
            if(state.energy < cost.requirement.energy)
            {
                return skill_payment_validation::insufficient_energy;
            }
            return skill_payment_validation::valid;
        }

        target_validation skill_targets_validate(
            const definition_library& library, const table& card_table,
            std::size_t skill_index, std::span<const skill_target_id> targets = {}
        ) const
        {
            const auto id = skill_id(skill_index);
            const auto entity = card_table[id];
            const auto definition = library[entity.definition_id()];
            std::array<skill_target_id, 2> selected_targets{};
            const auto target_count = std::min(targets.size(), selected_targets.size());
            for(std::size_t index = 0; index < target_count; ++index)
            {
                selected_targets[index] = targets[index];
            }
            return definition.query(skill_target_validation{
                .skill = entity, .table = card_table, .library = library,
                .targets = selected_targets, .target_count = target_count
            });
        }

        constexpr void use_skill(
            std::size_t skill_index, const dice_counts& paid_dice, std::span<const skill_target_id> targets = {}
        ) const noexcept
        {
            std::array<skill_target_id, 2> selected_targets{};
            const auto target_count = std::min(targets.size(), selected_targets.size());
            for(std::size_t index = 0; index < target_count; ++index)
            {
                selected_targets[index] = targets[index];
            }
            get<0>(stack_->top<detail::action_selection, substack_t>()) = detail::action_selection{
                detail::skill_selection{
                    .skill_cost_index = skill_index, .targets = selected_targets, .paid_dice = paid_dice
                }
            };
        }

        void use_skill(
            const definition_library& library, const table& card_table,
            std::size_t skill_index, const dice_counts& paid_dice, std::span<const skill_target_id> targets = {}
        ) const
        {
            detail::calculate_skill_cost(library, skill_index, card_table, *stack_);
            use_skill(skill_index, paid_dice, targets);
        }

        constexpr bool has_technique() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<
                cost_of_technique[], program_entry[], std::size_t[], std::size_t[],
                detail::skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
                detail::card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
                detail::switch_handler_id[],
                cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
                stack_count_t, detail::action_selection, substack_t
            >()).size() != 0;
        }

        constexpr const cost_of_technique& technique_cost() const noexcept
        {
            return get<0>(std::as_const(*stack_).top<
                cost_of_technique[], program_entry[], std::size_t[], std::size_t[],
                detail::skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
                detail::card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
                detail::switch_handler_id[],
                cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
                stack_count_t, detail::action_selection, substack_t
            >())[0];
        }

        constexpr givm::attachment_id technique_id() const noexcept
        {
            return technique_cost().technique;
        }

        const cost_of_technique& calculate_technique_cost(
            const definition_library& library, const table& card_table
        ) const
        {
            return detail::calculate_technique_cost(library, card_table, *stack_);
        }

        constexpr technique_payment_validation technique_payment_validate(
            const table& card_table, const dice_counts& paid_dice
        ) const noexcept
        {
            const auto& cost = technique_cost();
            if(not payment_matches(cost.requirement.dice_requirement, paid_dice))
            {
                return technique_payment_validation::requirement_mismatch;
            }
            if(not card_table[cost.technique.character_id.player_id].state().dice.contains(paid_dice))
            {
                return technique_payment_validation::insufficient_dice;
            }
            const auto active = *card_table[cost.technique.character_id.player_id].state().active_character;
            const auto& state = card_table[active].state();
            if(cost.requirement.energy != 0 && state.energy_tag != cost.requirement.energy_tag)
            {
                return technique_payment_validation::energy_tag_mismatch;
            }
            if(state.energy < cost.requirement.energy)
            {
                return technique_payment_validation::insufficient_energy;
            }
            return technique_payment_validation::valid;
        }

        target_validation technique_targets_validate(
            const definition_library& library, const table& card_table,
            std::span<const technique_target_id> targets = {}
        ) const
        {
            const auto id = technique_id();
            const auto entity = card_table[id];
            const auto definition = library[entity.definition_id()];
            std::array<technique_target_id, 2> selected_targets{};
            const auto target_count = std::min(targets.size(), selected_targets.size());
            for(std::size_t index = 0; index < target_count; ++index)
            {
                selected_targets[index] = targets[index];
            }
            return definition.query(technique_target_validation{
                .technique = entity, .table = card_table, .library = library,
                .targets = selected_targets, .target_count = target_count
            });
        }

        constexpr void use_technique(
            const dice_counts& paid_dice, std::span<const technique_target_id> targets = {}
        ) const noexcept
        {
            std::array<technique_target_id, 2> selected_targets{};
            const auto target_count = std::min(targets.size(), selected_targets.size());
            for(std::size_t index = 0; index < target_count; ++index)
            {
                selected_targets[index] = targets[index];
            }
            get<0>(stack_->top<detail::action_selection, substack_t>()) = detail::action_selection{
                detail::technique_selection{
                    .targets = selected_targets, .paid_dice = paid_dice
                }
            };
        }

        void use_technique(
            const definition_library& library, const table& card_table,
            const dice_counts& paid_dice, std::span<const technique_target_id> targets = {}
        ) const
        {
            detail::calculate_technique_cost(library, card_table, *stack_);
            use_technique(paid_dice, targets);
        }

        constexpr void declare_round_end() const noexcept
        {
            get<0>(stack_->top<detail::action_selection, substack_t>()).emplace<detail::round_end_selection>();
        }

    private:
        friend class executor;
        constexpr explicit execution_view(frame_stack& stack) noexcept : stack_{ &stack } {}
        frame_stack* stack_;

        static constexpr bool payment_matches(
            const elemental_dice_requirement& requirement, const dice_counts& paid
        ) noexcept
        {
            const std::uint32_t required_total =
                requirement.fixed.total() + requirement.same + requirement.any;
            if(paid.total() != required_total)
            {
                return false;
            }

            std::uint32_t required_omni = requirement.fixed[elemental_dice::omni];
            std::uint32_t largest_remaining_group = 0;
            for(std::uint8_t index = 1; index < 8; ++index)
            {
                const auto dice = static_cast<elemental_dice>(index);
                const std::uint32_t fixed = requirement.fixed[dice];
                const std::uint32_t count = paid[dice];
                if(count < fixed)
                {
                    required_omni += fixed - count;
                }
                else
                {
                    const auto remaining = count - fixed;
                    if(remaining > largest_remaining_group)
                    {
                        largest_remaining_group = remaining;
                    }
                }
            }

            const std::uint32_t omni = paid[elemental_dice::omni];
            return omni >= required_omni
                && largest_remaining_group + (omni - required_omni) >= requirement.same;
        }
    };
}

#include "../../macro_undef.hpp"

#endif
