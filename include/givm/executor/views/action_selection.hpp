#ifndef GIVM_EXECUTOR_VIEWS_ACTION_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_ACTION_SELECTION_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <type_traits>
#include <variant>

#include "../executor.hpp"
#include "../execution_view_error.hpp"
#include "../command_input_error.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    namespace detail
    {
        struct switch_selection
        {
            stack_count_t switch_cost_index = 0;
            dice_counts paid_dice;
        };

        struct round_end_selection {};

        struct card_selection
        {
            stack_count_t card_cost_index = 0;
            std::array<card_target_id, 2> targets;
            dice_counts paid_dice;
        };

        struct skill_selection
        {
            stack_count_t skill_cost_index = 0;
            std::array<skill_target_id, 2> targets;
            dice_counts paid_dice;
        };

        struct technique_selection
        {
            std::array<technique_target_id, 2> targets;
            dice_counts paid_dice;
        };

        struct elemental_tuning_selection
        {
            stack_count_t card_index = 0;
            elemental_dice from;
        };

        using action_selection = std::variant<
            round_end_selection, switch_selection, card_selection, skill_selection, technique_selection, elemental_tuning_selection
        >;
    }

    enum class action_target_kind : std::uint8_t
    {
        none,
        character,
        support,
        summon
    };

    struct action_target
    {
        action_target_kind kind = action_target_kind::none;
        character_id character{};
        support_id support{};
        summon_id summon{};
    };

    struct action_argument
    {
        dice_counts paid_dice;
    };
}

namespace givm::detail
{
    template<class TEvent>
    using handler_id = decltype([]<class... TDefinition>(type_list<TDefinition...>)
    {
        using views = type_list_cat<views_of_definition<TDefinition>...>;
        return []<class... TView>(type_list<TView...>)
        {
            using ids = type_list_cat<
                std::conditional_t<
                    requires { subscribed_events<TView>::template index_of<TEvent>(); },
                    type_list<decltype(std::declval<TView>().id())>,
                    type_list<>
                >...
            >;
            return typename ids::template apply<std::variant>{};
        }(views{});
    }(definition_types{}));

    using switch_handler_id = handler_id<cost_of_switch>;
    using card_cost_handler_id = handler_id<cost_of_card>;
    using skill_cost_handler_id = handler_id<cost_of_skill>;
    using technique_cost_handler_id = handler_id<cost_of_technique>;

#ifndef NDEBUG
    template<class TTable, class TId>
    inline void debug_validate_entity(const TTable& table, TId id, std::string_view command,
        std::string_view field, bool allow_removed = false)
    {
        if constexpr(requires { std::variant_size<TId>::value; })
            std::visit([&](auto value) { debug_validate_entity(table, value, command, field, allow_removed); }, id);
        else if constexpr(std::is_same_v<TId, std::monostate>)
            return;
        else
        {
            bool in_range;
            if constexpr(std::is_same_v<TId, player_id>)
                in_range = id.index < 2;
            else if constexpr(requires { id.character_id; })
            {
                debug_validate_entity(table, id.character_id, command, std::string{ field } + ".character", allow_removed);
                const auto character = table[id.character_id];
                if constexpr(std::is_same_v<TId, skill_id>)
                    in_range = id.index < character.template skills<false>().size();
                else
                    in_range = id.index < character.template attachments<false>().size();
            }
            else if constexpr(requires { id.card_id; })
            {
                debug_validate_entity(table, id.card_id, command, std::string{ field } + ".card", allow_removed);
                in_range = table.debug_entity_in_range(id);
            }
            else
            {
                debug_validate_entity(table, id.player_id, command, std::string{ field } + ".player", allow_removed);
                const auto player = table[id.player_id];
                if constexpr(std::is_same_v<TId, character_id>)
                    in_range = id.index < player.template characters<false>().size();
                else if constexpr(std::is_same_v<TId, hand_card_id>)
                    in_range = id.index < player.template hand_cards<false>().size();
                else if constexpr(std::is_same_v<TId, deck_card_id>)
                    in_range = table.debug_entity_in_range(id);
                else if constexpr(std::is_same_v<TId, support_id>)
                    in_range = id.index < player.template supports<false>().size();
                else if constexpr(std::is_same_v<TId, summon_id>)
                    in_range = id.index < player.template summons<false>().size();
                else
                    in_range = id.index < player.template combat_statuses<false>().size();
            }
            if(not in_range)
                throw command_input_error{ command, invalid_entity_argument{
                    std::string{ field }, id, invalid_entity_argument::reason::out_of_range } };
            if constexpr(not std::is_same_v<TId, player_id>)
                if(not allow_removed && not table[id].is_valid())
                    throw command_input_error{ command, invalid_entity_argument{
                        std::string{ field }, id, invalid_entity_argument::reason::removed } };
        }
    }

#endif

    inline cost_of_switch default_switch_cost(character_id target) noexcept
    {
        action_cost_requirement requirement;
        requirement.dice_requirement.any = 1;
        requirement.speed = action_speed::combat;
        return {
            .target = target,
            .requirement = requirement
        };
    }

    inline const cost_of_switch& calculate_switch_cost(
        const definition_library& library,
        stack_count_t cost_index,
        const table& card_table,
        frame_stack& stack
    )
    {
        auto frame = stack.top<
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >();
        GIVM_ASSERT(cost_index < get<1>(frame).size());
        auto& initial_cost = get<1>(frame)[cost_index];
        initial_cost.requirement = default_switch_cost(initial_cost.target).requirement;

        auto zero_random = []() -> std::uint32_t { return 0; };
        random_fn random{ zero_random };
        const auto handler_count = static_cast<stack_count_t>(get<0>(frame).size());
        const auto row_begin = cost_index * handler_count;
        for(stack_count_t column = 0; column < handler_count; ++column)
        {
            const auto initial_size = stack.size();
            const auto handler_id = get<0>(frame)[column];
            const auto entry = std::visit([&](auto handler) -> program_entry
            {
                const auto entity = card_table[handler];
                if(entity)
                {
                    auto event = get<1>(frame)[cost_index];
                    auto response = execution_context::make_handle_context<true>(stack, library, entity, random);
                    const auto entry = library[entity.definition_id()].template handle<cost_of_switch>(event, response, 0);
                    std::memcpy(&get<1>(frame)[cost_index], &event, sizeof(event));
                    return entry;
                }
                return {};
            }, handler_id);
            const auto index = row_begin + column;
            const auto size = stack.size() - initial_size;
            get<2>(frame)[index] = entry;
            get<3>(frame)[index] = 0;
            get<4>(frame)[index] = size;
            if(size != 0)
            {
                const auto cache = get<0>(stack.top<substack_t>());
                const auto& tail = get<0>(cache.top<unsigned char[max_alignment]>());
                get<3>(frame)[index] = static_cast<std::size_t>(tail + max_alignment - stack.data()) - size;
            }
        }
        return get<1>(frame)[cost_index];
    }

    inline const cost_of_card& calculate_card_cost(
        const definition_library& library,
        stack_count_t cost_index,
        const table& card_table,
        frame_stack& stack
    )
    {
        auto frame = stack.top<
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >();
        GIVM_ASSERT(cost_index < get<1>(frame).size());
        auto& initial_cost = get<1>(frame)[cost_index];
        const auto card = card_table[initial_cost.card];
        auto zero_random = []() -> std::uint32_t { return 0; };
        random_fn random{ zero_random };
        initial_cost.requirement = card.state().cost;

        const auto handler_count = static_cast<stack_count_t>(get<0>(frame).size());
        const auto row_begin = cost_index * handler_count;
        for(stack_count_t column = 0; column < handler_count; ++column)
        {
            const auto initial_size = stack.size();
            const auto handler_id = get<0>(frame)[column];
            const auto entry = std::visit([&](auto handler) -> program_entry
            {
                const auto entity = card_table[handler];
                if(entity)
                {
                    auto event = get<1>(frame)[cost_index];
                    auto response = execution_context::make_handle_context<true>(stack, library, entity, random);
                    const auto entry = library[entity.definition_id()].template handle<cost_of_card>(event, response, 0);
                    std::memcpy(&get<1>(frame)[cost_index], &event, sizeof(event));
                    return entry;
                }
                return {};
            }, handler_id);
            const auto index = row_begin + column;
            const auto size = stack.size() - initial_size;
            get<2>(frame)[index] = entry;
            get<3>(frame)[index] = 0;
            get<4>(frame)[index] = size;
            if(size != 0)
            {
                const auto cache = get<0>(stack.top<substack_t>());
                const auto& tail = get<0>(cache.top<unsigned char[max_alignment]>());
                get<3>(frame)[index] = static_cast<std::size_t>(tail + max_alignment - stack.data()) - size;
            }
        }
        return get<1>(frame)[cost_index];
    }

    inline const cost_of_skill& calculate_skill_cost(
        const definition_library& library,
        stack_count_t cost_index,
        const table& card_table,
        frame_stack& stack
    )
    {
        auto frame = stack.top<
            skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >();
        GIVM_ASSERT(cost_index < get<1>(frame).size());
        auto& initial_cost = get<1>(frame)[cost_index];
        const auto skill = card_table[initial_cost.skill];
        auto zero_random = []() -> std::uint32_t { return 0; };
        random_fn random{ zero_random };
        initial_cost.requirement = library[skill.definition_id()].query(skill_initial_cost{});

        const auto handler_count = static_cast<stack_count_t>(get<0>(frame).size());
        const auto row_begin = cost_index * handler_count;
        for(stack_count_t column = 0; column < handler_count; ++column)
        {
            const auto initial_size = stack.size();
            const auto handler_id = get<0>(frame)[column];
            const auto entry = std::visit([&](auto handler) -> program_entry
            {
                const auto entity = card_table[handler];
                if(entity)
                {
                    auto event = get<1>(frame)[cost_index];
                    auto response = execution_context::make_handle_context<true>(stack, library, entity, random);
                    const auto entry = library[entity.definition_id()].template handle<cost_of_skill>(event, response, 0);
                    std::memcpy(&get<1>(frame)[cost_index], &event, sizeof(event));
                    return entry;
                }
                return {};
            }, handler_id);
            const auto index = row_begin + column;
            const auto size = stack.size() - initial_size;
            get<2>(frame)[index] = entry;
            get<3>(frame)[index] = 0;
            get<4>(frame)[index] = size;
            if(size != 0)
            {
                const auto cache = get<0>(stack.top<substack_t>());
                const auto& tail = get<0>(cache.top<unsigned char[max_alignment]>());
                get<3>(frame)[index] = static_cast<std::size_t>(tail + max_alignment - stack.data()) - size;
            }
        }
        return get<1>(frame)[cost_index];
    }

    inline const cost_of_technique& calculate_technique_cost(
        const definition_library& library,
        const table& card_table,
        frame_stack& stack
    )
    {
        auto frame = stack.top<
            technique_cost_handler_id[], cost_of_technique[], program_entry[], std::size_t[], std::size_t[],
            skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >();
        GIVM_ASSERT(0 < get<1>(frame).size());
        auto& initial_cost = get<1>(frame)[0];
        const auto technique = card_table[initial_cost.technique];
        auto zero_random = []() -> std::uint32_t { return 0; };
        random_fn random{ zero_random };
        initial_cost.requirement = library[technique.definition_id()].query(technique_initial_cost{});

        const auto handler_count = static_cast<stack_count_t>(get<0>(frame).size());
        for(stack_count_t column = 0; column < handler_count; ++column)
        {
            const auto initial_size = stack.size();
            const auto handler_id = get<0>(frame)[column];
            const auto entry = std::visit([&](auto handler) -> program_entry
            {
                const auto entity = card_table[handler];
                if(entity)
                {
                    auto event = get<1>(frame)[0];
                    auto response = execution_context::make_handle_context<true>(stack, library, entity, random);
                    const auto entry = library[entity.definition_id()].template handle<cost_of_technique>(event, response, 0);
                    std::memcpy(&get<1>(frame)[0], &event, sizeof(event));
                    return entry;
                }
                return {};
            }, handler_id);
            const auto index = column;
            const auto size = stack.size() - initial_size;
            get<2>(frame)[index] = entry;
            get<3>(frame)[index] = 0;
            get<4>(frame)[index] = size;
            if(size != 0)
            {
                const auto cache = get<0>(stack.top<substack_t>());
                const auto& tail = get<0>(cache.top<unsigned char[max_alignment]>());
                get<3>(frame)[index] = static_cast<std::size_t>(tail + max_alignment - stack.data()) - size;
            }
        }
        return get<1>(frame)[0];
    }

}

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
        invalid_element,
        omni_not_allowed,
        missing_character_element,
        same_element,
        insufficient_dice
    };

    enum class action_unavailable : std::uint8_t
    {
        controlled,
        missing_technique,
        elemental_tuning_forbidden
    };

    struct action_cost_cache_error
    {
        enum class reason : std::uint8_t { not_calculated, already_calculated, incomplete_calculation };
        std::string action;
        std::size_t index;
        reason cause;
    };

    struct action_target_validation_error
    {
        std::size_t checked_count;
        std::size_t target_count;
        target_validation result;
    };

    inline std::string error_string(action_unavailable reason)
    {
        switch(reason)
        {
        case action_unavailable::controlled: return "The active character is controlled.";
        case action_unavailable::missing_technique: return "The active character has no usable technique.";
        case action_unavailable::elemental_tuning_forbidden: return "The selected card cannot be used for elemental tuning.";
        }
        return "Unknown action availability error.";
    }

    inline std::string error_string(const action_cost_cache_error& error)
    {
        const auto description = error.cause == action_cost_cache_error::reason::not_calculated
            ? "has not been calculated" : error.cause == action_cost_cache_error::reason::already_calculated
                ? "has already been calculated" : "did not finish calculating";
        return error.action + " cost at index " + std::to_string(error.index) + " " + description + ".";
    }

    inline std::string error_string(const action_target_validation_error& error)
    {
        const auto description = error.result == target_validation::invalid ? "is invalid"
            : error.result == target_validation::valid_complete ? "is already complete and does not permit another target"
            : "is incomplete";
        return "Target selection with " + std::to_string(error.checked_count) + " of "
            + std::to_string(error.target_count) + " targets " + description + ".";
    }

    inline std::string error_string(switch_payment_validation result)
    {
        switch(result)
        {
        case switch_payment_validation::valid: return "Payment is valid.";
        case switch_payment_validation::requirement_mismatch: return "Selected dice do not match the cost requirement.";
        case switch_payment_validation::insufficient_dice: return "The player does not have the selected dice.";
        case switch_payment_validation::energy_tag_mismatch: return "The active character's energy type does not match the cost.";
        case switch_payment_validation::insufficient_energy: return "The active character has insufficient energy.";
        }
        return "Unknown payment validation result.";
    }

    inline std::string error_string(card_payment_validation result)
    {
        switch(result)
        {
        case card_payment_validation::valid: return "Payment is valid.";
        case card_payment_validation::requirement_mismatch: return "Selected dice do not match the cost requirement.";
        case card_payment_validation::insufficient_dice: return "The player does not have the selected dice.";
        case card_payment_validation::energy_tag_mismatch: return "The active character's energy type does not match the cost.";
        case card_payment_validation::insufficient_energy: return "The active character has insufficient energy.";
        }
        return "Unknown payment validation result.";
    }

    inline std::string error_string(skill_payment_validation result)
    {
        switch(result)
        {
        case skill_payment_validation::valid: return "Payment is valid.";
        case skill_payment_validation::requirement_mismatch: return "Selected dice do not match the cost requirement.";
        case skill_payment_validation::insufficient_dice: return "The player does not have the selected dice.";
        case skill_payment_validation::energy_tag_mismatch: return "The active character's energy type does not match the cost.";
        case skill_payment_validation::insufficient_energy: return "The active character has insufficient energy.";
        }
        return "Unknown payment validation result.";
    }

    inline std::string error_string(technique_payment_validation result)
    {
        switch(result)
        {
        case technique_payment_validation::valid: return "Payment is valid.";
        case technique_payment_validation::requirement_mismatch: return "Selected dice do not match the cost requirement.";
        case technique_payment_validation::insufficient_dice: return "The player does not have the selected dice.";
        case technique_payment_validation::energy_tag_mismatch: return "The active character's energy type does not match the cost.";
        case technique_payment_validation::insufficient_energy: return "The active character has insufficient energy.";
        }
        return "Unknown payment validation result.";
    }

    inline std::string error_string(elemental_tuning_dice_validation result)
    {
        switch(result)
        {
        case elemental_tuning_dice_validation::valid: return "The selected die is valid for elemental tuning.";
        case elemental_tuning_dice_validation::invalid_element: return "The die element is invalid.";
        case elemental_tuning_dice_validation::omni_not_allowed: return "An omni die cannot be used for elemental tuning.";
        case elemental_tuning_dice_validation::missing_character_element: return "The active character has no element.";
        case elemental_tuning_dice_validation::same_element: return "The selected die already has the active character's element.";
        case elemental_tuning_dice_validation::insufficient_dice: return "The player does not have the selected die.";
        }
        return "Unknown elemental tuning validation result.";
    }

    template<>
    class execution_view<execution_state::action_selection>
    {
    public:
        bool is_controlled(const definition_library& library, const table& card_table) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif

            const auto active = *card_table[card_table.state().active_player].state().active_character;
            return library.is_controlled(card_table[active]);
        }

        constexpr std::size_t switch_target_count() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif

            return get<0>(std::as_const(executor_->context_.stack()).top<
                cost_of_switch[],
                program_entry[], std::size_t[], std::size_t[],
                stack_count_t,
                detail::action_selection, substack_t
            >()).size();
        }

        constexpr const cost_of_switch& switch_cost(std::size_t target_index) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
            validate_switch_index(target_index);
            validate_cost_cache("switch", target_index, switch_cache_offset() + target_index);
#endif
            return switch_cost_record(target_index);
        }

        constexpr character_id switch_target(std::size_t target_index) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
            validate_switch_index(target_index);
#endif

            return switch_cost_record(target_index).target;
        }

        const cost_of_switch& calculate_switch_cost(
            const definition_library& library, const table& card_table,
            std::size_t target_index
        ) const
        {
#ifndef NDEBUG
            validate_view();
            validate_switch_index(target_index);
            begin_cost_calculation("switch", target_index, switch_cache_offset() + target_index);
#endif

            const auto& result = detail::calculate_switch_cost(
                library, target_index, card_table, executor_->context_.stack()
            );
#ifndef NDEBUG
            debug_cost_states()[switch_cache_offset() + target_index] = 2;
#endif
            return result;
        }

        constexpr switch_payment_validation switch_payment_validate(
            const table& card_table, std::size_t target_index, const dice_counts& paid_dice
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif

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

        template<class TRandom>
        execution_state switch_active_character_with_cached_cost(
            const definition_library& library, table& card_table, TRandom& random_source,
            std::size_t target_index, const dice_counts& paid_dice) const
        {
#ifndef NDEBUG
            validate_view();
            const auto payment = switch_payment_validate(card_table, target_index, paid_dice);
            if(payment != switch_payment_validation::valid)
                throw view_input_error{ "switch_active_character_with_cached_cost", payment };
#endif

            // Assign the complete variant through its trivial assignment operator.
            get<0>(executor_->context_.stack().top<detail::action_selection, substack_t>()) = detail::action_selection{
                detail::switch_selection{ .switch_cost_index = target_index, .paid_dice = paid_dice }
            };
            return executor_->advance(library, card_table, random_source);
        }

        template<class TRandom>
        execution_state switch_active_character(
            const definition_library& library, table& card_table, TRandom& random_source,
            std::size_t target_index, const dice_counts& paid_dice
        ) const
        {
#ifndef NDEBUG
            validate_view();
#endif
            calculate_switch_cost(library, card_table, target_index);
            return switch_active_character_with_cached_cost(library, card_table, random_source, target_index, paid_dice);
        }

        constexpr std::size_t card_count() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif

            return get<0>(std::as_const(executor_->context_.stack()).top<
                cost_of_card[], program_entry[], std::size_t[], std::size_t[],
                detail::switch_handler_id[],
                cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
                stack_count_t, detail::action_selection, substack_t
            >()).size();
        }

        constexpr const cost_of_card& card_cost(std::size_t card_index) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
            validate_card_index(card_index);
            validate_cost_cache("card", card_index, card_cache_offset() + card_index);
#endif
            return card_cost_record(card_index);
        }

        constexpr hand_card_id card_id(std::size_t card_index) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
            validate_card_index(card_index);
#endif

            return card_cost_record(card_index).card;
        }

        const cost_of_card& calculate_card_cost(
            const definition_library& library, const table& card_table, std::size_t card_index
        ) const
        {
#ifndef NDEBUG
            validate_view();
            validate_card_index(card_index);
            begin_cost_calculation("card", card_index, card_cache_offset() + card_index);
#endif

            const auto& result = detail::calculate_card_cost(library, card_index, card_table, executor_->context_.stack());
#ifndef NDEBUG
            debug_cost_states()[card_cache_offset() + card_index] = 2;
#endif
            return result;
        }

        constexpr card_payment_validation card_payment_validate(
            const table& card_table, std::size_t card_index, const dice_counts& paid_dice
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif

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
#ifndef NDEBUG
            validate_view();
#endif

            const auto id = card_id(card_index);
            const auto entity = card_table[id];
            const auto definition = library[entity.definition_id()];
            std::array<card_target_id, 2> selected_targets{};
            const auto target_count = std::min(targets.size(), selected_targets.size());
            for(std::size_t index = 0; index < target_count; ++index)
            {
                selected_targets[index] = targets[index];
            }
#ifndef NDEBUG
            for(std::size_t index = 0; index < target_count; ++index)
                detail::debug_validate_entity(card_table, selected_targets[index], "card_targets_validate", "targets");
#endif
            return definition.query(card_target_validation{
                .card = entity, .table = card_table, .library = library,
                .targets = selected_targets, .target_count = target_count
            });
        }

        template<class TRandom>
        execution_state play_card_with_cached_cost(
            const definition_library& library, table& card_table, TRandom& random_source,
            std::size_t card_index, const dice_counts& paid_dice, std::span<const card_target_id> targets = {}
        ) const
        {
#ifndef NDEBUG
            validate_view();
            const auto payment = card_payment_validate(card_table, card_index, paid_dice);
            if(payment != card_payment_validation::valid)
                throw view_input_error{ "play_card_with_cached_cost", payment };
            validate_targets("play_card_with_cached_cost", targets, [&](auto prefix)
            {
                return card_targets_validate(library, card_table, card_index, prefix);
            });
#endif

            std::array<card_target_id, 2> selected_targets{};
            const auto target_count = std::min(targets.size(), selected_targets.size());
            for(std::size_t index = 0; index < target_count; ++index)
            {
                selected_targets[index] = targets[index];
            }
            get<0>(executor_->context_.stack().top<detail::action_selection, substack_t>()) = detail::action_selection{
                detail::card_selection{
                    .card_cost_index = card_index, .targets = selected_targets, .paid_dice = paid_dice
                }
            };
            return executor_->advance(library, card_table, random_source);
        }

        template<class TRandom>
        execution_state play_card(
            const definition_library& library, table& card_table, TRandom& random_source,
            std::size_t card_index, const dice_counts& paid_dice, std::span<const card_target_id> targets = {}
        ) const
        {
#ifndef NDEBUG
            validate_view();
#endif
            calculate_card_cost(library, card_table, card_index);
            return play_card_with_cached_cost(library, card_table, random_source, card_index, paid_dice, targets);
        }

        constexpr bool elemental_tuning_card_validate(const table& card_table, std::size_t card_index) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif

            return card_table[card_id(card_index)].state().elemental_tuning_allowed;
        }

        constexpr elemental_tuning_dice_validation elemental_tuning_dice_validate(
            const table& card_table, elemental_dice from
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif

#ifndef NDEBUG
            if(std::to_underlying(from) > std::to_underlying(elemental_dice::omni))
                return elemental_tuning_dice_validation::invalid_element;
#endif
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

        template<class TRandom>
        execution_state elemental_tuning(
            const definition_library& library, table& card_table, TRandom& random_source,
            std::size_t card_index, elemental_dice from
        ) const
        {
#ifndef NDEBUG
            validate_view();
            if(not elemental_tuning_card_validate(card_table, card_index))
                throw view_input_error{ "elemental_tuning", action_unavailable::elemental_tuning_forbidden };
            const auto result = elemental_tuning_dice_validate(card_table, from);
            if(result != elemental_tuning_dice_validation::valid)
                throw view_input_error{ "elemental_tuning", result };
#endif

            get<0>(executor_->context_.stack().top<detail::action_selection, substack_t>()) = detail::action_selection{
                detail::elemental_tuning_selection{ .card_index = card_index, .from = from }
            };
            return executor_->advance(library, card_table, random_source);
        }

        constexpr std::size_t skill_count() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif

            return get<0>(std::as_const(executor_->context_.stack()).top<
                cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
                detail::card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
                detail::switch_handler_id[],
                cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
                stack_count_t, detail::action_selection, substack_t
            >()).size();
        }

        constexpr const cost_of_skill& skill_cost(std::size_t skill_index) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
            validate_skill_index(skill_index);
            validate_cost_cache("skill", skill_index, skill_cache_offset() + skill_index);
#endif
            return skill_cost_record(skill_index);
        }

        constexpr givm::skill_id skill_id(std::size_t skill_index) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
            validate_skill_index(skill_index);
#endif

            return skill_cost_record(skill_index).skill;
        }

        const cost_of_skill& calculate_skill_cost(
            const definition_library& library, const table& card_table, std::size_t skill_index
        ) const
        {
#ifndef NDEBUG
            validate_view();
            validate_skill_index(skill_index);
            begin_cost_calculation("skill", skill_index, skill_cache_offset() + skill_index);
#endif

            const auto& result = detail::calculate_skill_cost(library, skill_index, card_table, executor_->context_.stack());
#ifndef NDEBUG
            debug_cost_states()[skill_cache_offset() + skill_index] = 2;
#endif
            return result;
        }

        constexpr skill_payment_validation skill_payment_validate(
            const table& card_table, std::size_t skill_index, const dice_counts& paid_dice
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif

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
#ifndef NDEBUG
            validate_view();
#endif

            const auto id = skill_id(skill_index);
            const auto entity = card_table[id];
            const auto definition = library[entity.definition_id()];
            std::array<skill_target_id, 2> selected_targets{};
            const auto target_count = std::min(targets.size(), selected_targets.size());
            for(std::size_t index = 0; index < target_count; ++index)
            {
                selected_targets[index] = targets[index];
            }
#ifndef NDEBUG
            for(std::size_t index = 0; index < target_count; ++index)
                detail::debug_validate_entity(card_table, selected_targets[index], "skill_targets_validate", "targets");
#endif
            return definition.query(skill_target_validation{
                .skill = entity, .table = card_table, .library = library,
                .targets = selected_targets, .target_count = target_count
            });
        }

        template<class TRandom>
        execution_state use_skill_with_cached_cost(
            const definition_library& library, table& card_table, TRandom& random_source,
            std::size_t skill_index, const dice_counts& paid_dice, std::span<const skill_target_id> targets = {}
        ) const
        {
#ifndef NDEBUG
            validate_view();
            const auto payment = skill_payment_validate(card_table, skill_index, paid_dice);
            if(payment != skill_payment_validation::valid)
                throw view_input_error{ "use_skill_with_cached_cost", payment };
            if(is_controlled(library, card_table))
                throw view_input_error{ "use_skill_with_cached_cost", action_unavailable::controlled };
            validate_targets("use_skill_with_cached_cost", targets, [&](auto prefix)
            {
                return skill_targets_validate(library, card_table, skill_index, prefix);
            });
#endif

            std::array<skill_target_id, 2> selected_targets{};
            const auto target_count = std::min(targets.size(), selected_targets.size());
            for(std::size_t index = 0; index < target_count; ++index)
            {
                selected_targets[index] = targets[index];
            }
            get<0>(executor_->context_.stack().top<detail::action_selection, substack_t>()) = detail::action_selection{
                detail::skill_selection{
                    .skill_cost_index = skill_index, .targets = selected_targets, .paid_dice = paid_dice
                }
            };
            return executor_->advance(library, card_table, random_source);
        }

        template<class TRandom>
        execution_state use_skill(
            const definition_library& library, table& card_table, TRandom& random_source,
            std::size_t skill_index, const dice_counts& paid_dice, std::span<const skill_target_id> targets = {}
        ) const
        {
#ifndef NDEBUG
            validate_view();
#endif
            calculate_skill_cost(library, card_table, skill_index);
            return use_skill_with_cached_cost(library, card_table, random_source, skill_index, paid_dice, targets);
        }

        constexpr bool has_technique() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif

            return get<0>(std::as_const(executor_->context_.stack()).top<
                cost_of_technique[], program_entry[], std::size_t[], std::size_t[],
                detail::skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
                detail::card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
                detail::switch_handler_id[],
                cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
                stack_count_t, detail::action_selection, substack_t
            >()).size() != 0;
        }

        constexpr const cost_of_technique& technique_cost() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
            validate_technique_index(0);
            validate_cost_cache("technique", 0, technique_cache_offset() + 0);
#endif
            return technique_cost_record();
        }

        constexpr givm::attachment_id technique_id() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
            validate_technique_index(0);
#endif

            return technique_cost_record().technique;
        }

        const cost_of_technique& calculate_technique_cost(
            const definition_library& library, const table& card_table
        ) const
        {
#ifndef NDEBUG
            validate_view();
            validate_technique_index(0);
            begin_cost_calculation("technique", 0, technique_cache_offset() + 0);
#endif

            const auto& result = detail::calculate_technique_cost(library, card_table, executor_->context_.stack());
#ifndef NDEBUG
            debug_cost_states()[technique_cache_offset() + 0] = 2;
#endif
            return result;
        }

        constexpr technique_payment_validation technique_payment_validate(
            const table& card_table, const dice_counts& paid_dice
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif

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
#ifndef NDEBUG
            validate_view();
#endif

            const auto id = technique_id();
            const auto entity = card_table[id];
            const auto definition = library[entity.definition_id()];
            std::array<technique_target_id, 2> selected_targets{};
            const auto target_count = std::min(targets.size(), selected_targets.size());
            for(std::size_t index = 0; index < target_count; ++index)
            {
                selected_targets[index] = targets[index];
            }
#ifndef NDEBUG
            for(std::size_t index = 0; index < target_count; ++index)
                detail::debug_validate_entity(card_table, selected_targets[index], "technique_targets_validate", "targets");
#endif
            return definition.query(technique_target_validation{
                .technique = entity, .table = card_table, .library = library,
                .targets = selected_targets, .target_count = target_count
            });
        }

        template<class TRandom>
        execution_state use_technique_with_cached_cost(
            const definition_library& library, table& card_table, TRandom& random_source,
            const dice_counts& paid_dice, std::span<const technique_target_id> targets = {}
        ) const
        {
#ifndef NDEBUG
            validate_view();
            const auto payment = technique_payment_validate(card_table, paid_dice);
            if(payment != technique_payment_validation::valid)
                throw view_input_error{ "use_technique_with_cached_cost", payment };
            if(is_controlled(library, card_table))
                throw view_input_error{ "use_technique_with_cached_cost", action_unavailable::controlled };
            validate_targets("use_technique_with_cached_cost", targets, [&](auto prefix)
            {
                return technique_targets_validate(library, card_table, prefix);
            });
#endif

            std::array<technique_target_id, 2> selected_targets{};
            const auto target_count = std::min(targets.size(), selected_targets.size());
            for(std::size_t index = 0; index < target_count; ++index)
            {
                selected_targets[index] = targets[index];
            }
            get<0>(executor_->context_.stack().top<detail::action_selection, substack_t>()) = detail::action_selection{
                detail::technique_selection{
                    .targets = selected_targets, .paid_dice = paid_dice
                }
            };
            return executor_->advance(library, card_table, random_source);
        }

        template<class TRandom>
        execution_state use_technique(
            const definition_library& library, table& card_table, TRandom& random_source,
            const dice_counts& paid_dice, std::span<const technique_target_id> targets = {}
        ) const
        {
#ifndef NDEBUG
            validate_view();
#endif
            calculate_technique_cost(library, card_table);
            return use_technique_with_cached_cost(library, card_table, random_source, paid_dice, targets);
        }

        template<class TRandom>
        execution_state declare_round_end(
            const definition_library& library, table& card_table, TRandom& random_source
        ) const
        {
#ifndef NDEBUG
            validate_view();
#endif

            get<0>(executor_->context_.stack().top<detail::action_selection, substack_t>()).emplace<detail::round_end_selection>();
            return executor_->advance(library, card_table, random_source);
        }

    private:
        friend class executor;
        constexpr explicit execution_view(executor& owner
#ifndef NDEBUG
            , std::size_t version
#endif
        ) noexcept : executor_{ &owner }
#ifndef NDEBUG
            , version_{ version }
#endif
        {}
        executor* executor_;
#ifndef NDEBUG
        std::size_t version_;
#endif


        constexpr const cost_of_switch& switch_cost_record(std::size_t target_index) const noexcept
        {
            return get<0>(std::as_const(executor_->context_.stack()).top<
                cost_of_switch[],
                program_entry[], std::size_t[], std::size_t[],
                stack_count_t,
                detail::action_selection, substack_t
            >())[target_index];
        }

        constexpr const cost_of_card& card_cost_record(std::size_t card_index) const noexcept
        {
            return get<0>(std::as_const(executor_->context_.stack()).top<
                cost_of_card[], program_entry[], std::size_t[], std::size_t[],
                detail::switch_handler_id[],
                cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
                stack_count_t, detail::action_selection, substack_t
            >())[card_index];
        }

        constexpr const cost_of_skill& skill_cost_record(std::size_t skill_index) const noexcept
        {
            return get<0>(std::as_const(executor_->context_.stack()).top<
                cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
                detail::card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
                detail::switch_handler_id[],
                cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
                stack_count_t, detail::action_selection, substack_t
            >())[skill_index];
        }

        constexpr const cost_of_technique& technique_cost_record() const noexcept
        {
            return get<0>(std::as_const(executor_->context_.stack()).top<
                cost_of_technique[], program_entry[], std::size_t[], std::size_t[],
                detail::skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
                detail::card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
                detail::switch_handler_id[],
                cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
                stack_count_t, detail::action_selection, substack_t
            >())[0];
        }

#ifndef NDEBUG
        void validate_view() const
        {
            executor_->template validate_view<execution_state::action_selection>(version_);
        }

        std::span<std::uint8_t> debug_cost_states() const
        {
            return get<0>(executor_->context_.stack().top<
                std::uint8_t[],
                detail::technique_cost_handler_id[], cost_of_technique[], program_entry[], std::size_t[], std::size_t[],
                detail::skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
                detail::card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
                detail::switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
                stack_count_t, detail::action_selection, substack_t
            >());
        }

        std::size_t switch_cache_offset() const { return 0; }
        std::size_t card_cache_offset() const { return switch_target_count(); }
        std::size_t skill_cache_offset() const { return card_cache_offset() + card_count(); }
        std::size_t technique_cache_offset() const { return skill_cache_offset() + skill_count(); }

        void validate_switch_index(std::size_t index) const
        {
            const auto count = switch_target_count();
            if(index >= count) throw view_input_error{ "switch_target", view_index_out_of_range{ "target_index", index, count } };
        }

        void validate_card_index(std::size_t index) const
        {
            const auto count = card_count();
            if(index >= count) throw view_input_error{ "card_id", view_index_out_of_range{ "card_index", index, count } };
        }

        void validate_skill_index(std::size_t index) const
        {
            const auto count = skill_count();
            if(index >= count) throw view_input_error{ "skill_id", view_index_out_of_range{ "skill_index", index, count } };
        }

        void validate_technique_index(std::size_t) const
        {
            if(not has_technique()) throw view_input_error{ "technique_id", action_unavailable::missing_technique };
        }

        void validate_cost_cache(std::string_view action, std::size_t index, std::size_t offset) const
        {
            const auto state = debug_cost_states()[offset];
            if(state != 2)
                throw view_input_error{ "cached_cost", action_cost_cache_error{
                    std::string{ action }, index, state == 0 ? action_cost_cache_error::reason::not_calculated
                        : action_cost_cache_error::reason::incomplete_calculation } };
        }

        void begin_cost_calculation(std::string_view action, std::size_t index, std::size_t offset) const
        {
            auto& state = debug_cost_states()[offset];
            if(state != 0)
                throw view_input_error{ "calculate_cost", action_cost_cache_error{
                    std::string{ action }, index, state == 2 ? action_cost_cache_error::reason::already_calculated
                        : action_cost_cache_error::reason::incomplete_calculation } };
            state = 1;
        }

        template<class TTarget, class TValidate>
        static void validate_targets(std::string_view operation, std::span<const TTarget> targets, TValidate&& validate)
        {
            const auto count = std::min(targets.size(), 2uz);
            for(std::size_t selected = count == 0 ? 0 : 1; selected <= count; ++selected)
            {
                const auto result = validate(targets.first(selected));
                if(result == target_validation::invalid
                    || (selected < count && result == target_validation::valid_complete)
                    || (selected == count && result != target_validation::valid_complete
                        && result != target_validation::valid_complete_or_continue))
                {
                    throw view_input_error{ operation, action_target_validation_error{ selected, count, result } };
                }
            }
        }
#endif

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
            for(std::uint8_t index = 0; index < std::to_underlying(elemental_dice::omni); ++index)
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
