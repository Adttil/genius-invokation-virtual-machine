#ifndef GIVM_EXECUTOR_VIEWS_ACTION_SELECTION_HPP
#define GIVM_EXECUTOR_VIEWS_ACTION_SELECTION_HPP

#include <algorithm>
#include <array>
#ifndef NDEBUG
#include <atomic>
#endif
#include <cstddef>
#include <cstdint>
#include <memory>
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
    template<class TCost>
    class action_cost_id
    {
    public:
        friend bool operator==(action_cost_id, action_cost_id) = default;

    private:
        friend class execution_view<execution_state::action_selection>;
        explicit action_cost_id(std::size_t offset
#ifndef NDEBUG
            , std::size_t window, std::size_t quote
#endif
        ) noexcept : offset_{ offset }
#ifndef NDEBUG
            , window_{ window }, quote_{ quote }
#endif
        {}

        std::size_t offset_;
#ifndef NDEBUG
        std::size_t window_;
        std::size_t quote_;
#endif
    };

    using switch_cost_id = action_cost_id<cost_of_switch>;
    using card_cost_id = action_cost_id<cost_of_card>;
    using skill_cost_id = action_cost_id<cost_of_skill>;
    using technique_cost_id = action_cost_id<cost_of_technique>;

    namespace detail
    {
        template<class TCost>
        struct paid_action_selection
        {
            std::size_t cost_offset;
            dice_counts paid_dice;
            character_id energy_payer;
        };

        using switch_selection = paid_action_selection<cost_of_switch>;
        using card_selection = paid_action_selection<cost_of_card>;
        using skill_selection = paid_action_selection<cost_of_skill>;
        using technique_selection = paid_action_selection<cost_of_technique>;
        struct round_end_selection {};

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
    using handler_id = decltype([]<std::size_t... I>(std::index_sequence<I...>)
    {
        using views = type_list_cat<detail::definition_views<detail::definition_categories[I]>...>;
        return []<class... TView>(type_list<TView...>)
        {
            using ids = type_list_cat<
                std::conditional_t<
                    requires { subscribed_events<std::remove_cvref_t<TView>::category>::template index_of<TEvent>(); },
                    type_list<decltype(std::declval<TView>().id())>,
                    type_list<>
                >...
            >;
            return []<class... TId>(type_list<TId...>)
            { return variant_entity_id<TId::category...>{}; }(ids{});
        }(views{});
    }(std::make_index_sequence<detail::definition_categories.size()>{}));

    using switch_handler_id = handler_id<cost_of_switch>;
    using card_cost_handler_id = handler_id<cost_of_card>;
    using skill_cost_handler_id = handler_id<cost_of_skill>;
    using technique_cost_handler_id = handler_id<cost_of_technique>;

#ifndef NDEBUG
    template<class TTable, class TId>
    inline void debug_validate_entity(const TTable& table, TId id, std::string_view command,
        std::string_view field, bool allow_removed = false)
    {
        if constexpr(requires { id.visit([](auto) {}); })
            id.visit([&](auto value) { debug_validate_entity(table, value, command, field, allow_removed); });
        else if constexpr(std::is_same_v<TId, std::nullptr_t>)
            return;
        else
        {
            bool in_range;
            if constexpr(std::is_same_v<TId, player_id>)
                in_range = id.index() < 2;
            else if constexpr(requires { id.character_id(); })
            {
                debug_validate_entity(table, id.character_id(), command, std::string{ field } + ".character", allow_removed);
                const auto character = table[id.character_id()];
                if constexpr(std::is_same_v<TId, skill_id>)
                    in_range = id.index() < character.template skills<false>().size();
                else
                    in_range = id.index() < character.template attachments<false>().size();
            }
            else if constexpr(requires { id.hand_card_id(); })
            {
                debug_validate_entity(table, id.hand_card_id(), command, std::string{ field } + ".card", allow_removed);
                in_range = table.debug_entity_in_range(id);
            }
            else if constexpr(requires { id.deck_card_id(); })
            {
                debug_validate_entity(table, id.deck_card_id(), command, std::string{ field } + ".card", allow_removed);
                in_range = table.debug_entity_in_range(id);
            }
            else
            {
                debug_validate_entity(table, id.player_id(), command, std::string{ field } + ".player", allow_removed);
                const auto player = table[id.player_id()];
                if constexpr(std::is_same_v<TId, character_id>)
                    in_range = id.index() < player.template characters<false>().size();
                else if constexpr(std::is_same_v<TId, hand_card_id>)
                    in_range = id.index() < player.template hand_cards<false>().size();
                else if constexpr(std::is_same_v<TId, deck_card_id>)
                    in_range = table.debug_entity_in_range(id);
                else if constexpr(std::is_same_v<TId, support_id>)
                    in_range = id.index() < player.template supports<false>().size();
                else if constexpr(std::is_same_v<TId, summon_id>)
                    in_range = id.index() < player.template summons<false>().size();
                else
                    in_range = id.index() < player.template combat_statuses<false>().size();
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

    using cost_event_types = type_list<cost_of_switch, cost_of_card, cost_of_skill, cost_of_technique>;

    struct action_skill_candidate
    {
        skill_id skill;
        skill_flags flags;
    };

    inline constexpr auto action_window_frame = frame<
        switch_handler_id[], card_cost_handler_id[], skill_cost_handler_id[], technique_cost_handler_id[],
        character_id[], hand_card_id[], action_skill_candidate[], attachment_id[],
#ifndef NDEBUG
        std::size_t, std::size_t,
#endif
        stack_count_t, action_selection, substack_t
    >;

    struct cached_cost_program
    {
        preview_effect entry;
        player_id self_player;
        std::size_t input_offset;
        std::size_t input_size;
    };

    struct cached_cost
    {
        std::size_t event_offset;
        std::size_t programs_offset;
        stack_count_t program_count;
#ifndef NDEBUG
        std::size_t previous;
        std::size_t type;
        std::size_t identity;
        bool complete = false;
#endif
    };

#ifndef NDEBUG
    inline std::atomic<std::size_t> next_action_cache_identity{ 1 };
#endif

    inline const cached_cost& cost_cache(const frame_stack& stack, std::size_t offset) noexcept
    {
        return *reinterpret_cast<const cached_cost*>(stack.data() + offset);
    }

    template<class TCost>
    inline const TCost& cost_event(const frame_stack& stack, std::size_t offset) noexcept
    {
        return *reinterpret_cast<const TCost*>(stack.data() + cost_cache(stack, offset).event_offset);
    }

    template<class TTarget>
    constexpr auto select_action_targets(std::span<const TTarget> targets) noexcept
    {
        std::array<TTarget, 2> result{};
        for(std::size_t index = 0; index < std::min(targets.size(), result.size()); ++index)
        {
            if((not targets[index])) break;
            result[index] = targets[index];
        }
        return result;
    }

    template<class TTarget>
    constexpr std::span<const TTarget> action_targets(const std::array<TTarget, 2>& targets) noexcept
    {
        const std::size_t size = (not targets[0]) ? 0
            : (not targets[1]) ? 1 : 2;
        return std::span<const TTarget>{ targets }.first(size);
    }

    template<class TCost>
    inline std::size_t calculate_cost(const definition_library& library, const table& card_table,
        frame_stack& stack, TCost event)
    {
        constexpr auto type = cost_event_types::index_of<TCost>();
        const auto handler_count = get<type>(get<0>(stack.top<action_window_frame>())).size();
        auto cache = get<0>(stack.top<substack_t>());
        auto [programs, stored_event, record] = cache.push(dynamic_array<cached_cost_program>(handler_count), event,
            cached_cost{});
        const auto offset = static_cast<std::size_t>(reinterpret_cast<unsigned char*>(&record) - stack.data());
        record.event_offset = reinterpret_cast<unsigned char*>(&stored_event) - stack.data();
        record.programs_offset = handler_count == 0 ? 0
            : reinterpret_cast<unsigned char*>(programs.data()) - stack.data();
        record.program_count = handler_count;
#ifndef NDEBUG
        auto window = get<0>(stack.top<action_window_frame>());
        record.previous = get<8>(window);
        record.type = type;
        record.identity = next_action_cache_identity.fetch_add(1, std::memory_order_relaxed);
        get<8>(window) = offset;
#endif
        const auto programs_offset = record.programs_offset;
        const auto event_offset = record.event_offset;
        for(stack_count_t index = 0; index < handler_count; ++index)
        {
            const auto handler = get<type>(get<0>(stack.top<action_window_frame>()))[index];
            const auto initial_size = stack.size();
            player_id self{};
            const auto entry = handler.visit([&](auto id) -> preview_effect
            {
                const auto entity = card_table[id];
                self = entity.player().id();
                if(not entity) return {};
                auto response = execution_context::make_preview_context(stack, library, entity);
                return library[entity.definition_id()].template handle<TCost>(event, response);
            });
            const auto size = stack.size() - initial_size;
            std::size_t input_offset = 0;
            if(size != 0)
            {
                const auto current_cache = get<0>(stack.top<substack_t>());
                const auto& tail = get<0>(current_cache.top<unsigned char[max_alignment]>());
                input_offset = static_cast<std::size_t>(tail + max_alignment - stack.data()) - size;
            }
            auto* current_programs = reinterpret_cast<cached_cost_program*>(stack.data() + programs_offset);
            std::construct_at(current_programs + index, cached_cost_program{ entry, self, input_offset, size });
        }
        reinterpret_cast<TCost*>(stack.data() + event_offset)->requirement = event.requirement;
#ifndef NDEBUG
        reinterpret_cast<cached_cost*>(stack.data() + offset)->complete = true;
#endif
        return offset;
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
        enum class reason : std::uint8_t { expired_window, already_calculated, incomplete_calculation };
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
        const auto description = error.cause == action_cost_cache_error::reason::expired_window
            ? "belongs to another action window" : error.cause == action_cost_cache_error::reason::already_calculated
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
        constexpr auto window() const noexcept
        {
            return get<0>(std::as_const(executor_->context_.stack()).top<detail::action_window_frame>());
        }

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
            return get<4>(window()).size();
        }

        constexpr character_id switch_target(std::size_t index) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
            validate_switch_index(index);
#endif
            return get<4>(window())[index];
        }

        constexpr std::size_t card_count() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif
            return get<5>(window()).size();
        }

        constexpr hand_card_id card_id(std::size_t index) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
            validate_card_index(index);
#endif
            return get<5>(window())[index];
        }

        constexpr std::size_t skill_count() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif
            return get<6>(window()).size();
        }

        constexpr givm::skill_id skill_id(std::size_t index) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
            validate_skill_index(index);
#endif
            return get<6>(window())[index].skill;
        }

        constexpr bool has_technique() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif
            return not get<7>(window()).empty();
        }

        constexpr givm::attachment_id technique_id() const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
            validate_technique_index(0);
#endif
            return get<7>(window())[0];
        }

        switch_cost_id calculate_switch_cost(const definition_library& library, const table& card_table,
            std::size_t target_index) const
        {
#ifndef NDEBUG
            validate_view();
            validate_switch_index(target_index);
#endif
            auto event = detail::default_switch_cost(switch_target(target_index));
#ifndef NDEBUG
            begin_cost_calculation("switch", target_index, event);
#endif
            const auto offset = detail::calculate_cost(library, card_table, executor_->context_.stack(), event);
            return switch_cost_id{ offset
#ifndef NDEBUG
                , get<9>(window()), detail::cost_cache(executor_->context_.stack(), offset).identity
#endif
            };
        }

        const cost_of_switch& switch_cost(switch_cost_id id) const noexcept(detail::view_checks_disabled)
        {
            return cached_cost("switch", id);
        }

        constexpr switch_payment_validation switch_payment_validate(
            const table& card_table, switch_cost_id id, const dice_counts& paid_dice
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif

            const auto& cost = switch_cost(id);
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
        execution_state switch_active_character_with_cached_cost(const definition_library& library, table& card_table,
            TRandom& random_source, switch_cost_id quote, const dice_counts& paid_dice) const
        {
#ifndef NDEBUG
            validate_view();
            const auto payment = switch_payment_validate(card_table, quote, paid_dice);
            if(payment != switch_payment_validation::valid)
                throw view_input_error{ "switch_active_character_with_cached_cost", payment };
#endif
            const auto payer = *card_table[card_table.state().active_player].state().active_character;
            get<0>(executor_->context_.stack().top<detail::action_selection, substack_t>()) = detail::action_selection{
                detail::switch_selection{ .cost_offset = quote.offset_, .paid_dice = paid_dice, .energy_payer = payer }
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
            const auto quote = calculate_switch_cost(library, card_table, target_index);
            return switch_active_character_with_cached_cost(library, card_table, random_source, quote, paid_dice);
        }

        card_cost_id calculate_card_cost(const definition_library& library, const table& card_table,
            std::size_t card_index, std::span<const card_target_id> targets = {}) const
        {
#ifndef NDEBUG
            validate_view();
            validate_card_index(card_index);
            validate_targets("calculate_card_cost", targets, [&](auto prefix)
            {
                return card_targets_validate(library, card_table, card_index, prefix);
            });
#endif
            const auto id = card_id(card_index);
            const cost_of_card event{ .card = id,
                .targets = detail::select_action_targets(targets), .requirement = card_table[id].state().cost };
#ifndef NDEBUG
            begin_cost_calculation("card", card_index, event);
#endif
            const auto offset = detail::calculate_cost(library, card_table, executor_->context_.stack(), event);
            return card_cost_id{ offset
#ifndef NDEBUG
                , get<9>(window()), detail::cost_cache(executor_->context_.stack(), offset).identity
#endif
            };
        }

        const cost_of_card& card_cost(card_cost_id id) const noexcept(detail::view_checks_disabled)
        {
            return cached_cost("card", id);
        }

        constexpr card_payment_validation card_payment_validate(
            const table& card_table, card_cost_id id, const dice_counts& paid_dice
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif

            const auto& cost = card_cost(id);
            if(not payment_matches(cost.requirement.dice_requirement, paid_dice))
            {
                return card_payment_validation::requirement_mismatch;
            }
            if(not card_table[cost.card.player_id()].state().dice.contains(paid_dice))
            {
                return card_payment_validation::insufficient_dice;
            }
            const auto active = *card_table[cost.card.player_id()].state().active_character;
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
            const auto target_count = detail::action_targets(detail::select_action_targets(targets)).size();
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
        execution_state play_card_with_cached_cost(const definition_library& library, table& card_table,
            TRandom& random_source, card_cost_id quote, const dice_counts& paid_dice) const
        {
#ifndef NDEBUG
            validate_view();
            const auto payment = card_payment_validate(card_table, quote, paid_dice);
            if(payment != card_payment_validation::valid)
                throw view_input_error{ "play_card_with_cached_cost", payment };
#endif
            const auto payer = *card_table[card_table.state().active_player].state().active_character;
            get<0>(executor_->context_.stack().top<detail::action_selection, substack_t>()) = detail::action_selection{
                detail::card_selection{ .cost_offset = quote.offset_, .paid_dice = paid_dice, .energy_payer = payer }
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
            const auto quote = calculate_card_cost(library, card_table, card_index, targets);
            return play_card_with_cached_cost(library, card_table, random_source, quote, paid_dice);
        }

        skill_cost_id calculate_skill_cost(const definition_library& library, const table& card_table,
            std::size_t skill_index, std::span<const skill_target_id> targets = {}) const
        {
#ifndef NDEBUG
            validate_view();
            validate_skill_index(skill_index);
            validate_targets("calculate_skill_cost", targets, [&](auto prefix)
            {
                return skill_targets_validate(library, card_table, skill_index, prefix);
            });
#endif
            const auto id = skill_id(skill_index);
            const cost_of_skill event{ .skill = id, .flags = get<6>(window())[skill_index].flags,
                .targets = detail::select_action_targets(targets), .requirement = library[card_table[id].definition_id()].query(skill_initial_cost{}) };
#ifndef NDEBUG
            begin_cost_calculation("skill", skill_index, event);
#endif
            const auto offset = detail::calculate_cost(library, card_table, executor_->context_.stack(), event);
            return skill_cost_id{ offset
#ifndef NDEBUG
                , get<9>(window()), detail::cost_cache(executor_->context_.stack(), offset).identity
#endif
            };
        }

        const cost_of_skill& skill_cost(skill_cost_id id) const noexcept(detail::view_checks_disabled)
        {
            return cached_cost("skill", id);
        }

        constexpr skill_payment_validation skill_payment_validate(
            const table& card_table, skill_cost_id id, const dice_counts& paid_dice
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif

            const auto& cost = skill_cost(id);
            if(not payment_matches(cost.requirement.dice_requirement, paid_dice))
            {
                return skill_payment_validation::requirement_mismatch;
            }
            if(not card_table[cost.skill.character_id().player_id()].state().dice.contains(paid_dice))
            {
                return skill_payment_validation::insufficient_dice;
            }
            const auto active = *card_table[cost.skill.character_id().player_id()].state().active_character;
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
            const auto target_count = detail::action_targets(detail::select_action_targets(targets)).size();
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
        execution_state use_skill_with_cached_cost(const definition_library& library, table& card_table,
            TRandom& random_source, skill_cost_id quote, const dice_counts& paid_dice) const
        {
#ifndef NDEBUG
            validate_view();
            const auto payment = skill_payment_validate(card_table, quote, paid_dice);
            if(payment != skill_payment_validation::valid)
                throw view_input_error{ "use_skill_with_cached_cost", payment };
            if(is_controlled(library, card_table))
                throw view_input_error{ "use_skill_with_cached_cost", action_unavailable::controlled };
#endif
            const auto payer = *card_table[card_table.state().active_player].state().active_character;
            get<0>(executor_->context_.stack().top<detail::action_selection, substack_t>()) = detail::action_selection{
                detail::skill_selection{ .cost_offset = quote.offset_, .paid_dice = paid_dice, .energy_payer = payer }
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
            const auto quote = calculate_skill_cost(library, card_table, skill_index, targets);
            return use_skill_with_cached_cost(library, card_table, random_source, quote, paid_dice);
        }

        technique_cost_id calculate_technique_cost(const definition_library& library, const table& card_table,
            std::span<const technique_target_id> targets = {}) const
        {
#ifndef NDEBUG
            validate_view();
            validate_technique_index(0);
            validate_targets("calculate_technique_cost", targets, [&](auto prefix)
            {
                return technique_targets_validate(library, card_table, prefix);
            });
#endif
            const auto id = technique_id();
            const cost_of_technique event{ .technique = id,
                .targets = detail::select_action_targets(targets), .requirement = library[card_table[id].definition_id()].query(technique_initial_cost{}) };
#ifndef NDEBUG
            begin_cost_calculation("technique", 0, event);
#endif
            const auto offset = detail::calculate_cost(library, card_table, executor_->context_.stack(), event);
            return technique_cost_id{ offset
#ifndef NDEBUG
                , get<9>(window()), detail::cost_cache(executor_->context_.stack(), offset).identity
#endif
            };
        }

        const cost_of_technique& technique_cost(technique_cost_id id) const noexcept(detail::view_checks_disabled)
        {
            return cached_cost("technique", id);
        }

        constexpr technique_payment_validation technique_payment_validate(
            const table& card_table, technique_cost_id id, const dice_counts& paid_dice
        ) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
#endif

            const auto& cost = technique_cost(id);
            if(not payment_matches(cost.requirement.dice_requirement, paid_dice))
            {
                return technique_payment_validation::requirement_mismatch;
            }
            if(not card_table[cost.technique.character_id().player_id()].state().dice.contains(paid_dice))
            {
                return technique_payment_validation::insufficient_dice;
            }
            const auto active = *card_table[cost.technique.character_id().player_id()].state().active_character;
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
            const auto target_count = detail::action_targets(detail::select_action_targets(targets)).size();
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
        execution_state use_technique_with_cached_cost(const definition_library& library, table& card_table,
            TRandom& random_source, technique_cost_id quote, const dice_counts& paid_dice) const
        {
#ifndef NDEBUG
            validate_view();
            const auto payment = technique_payment_validate(card_table, quote, paid_dice);
            if(payment != technique_payment_validation::valid)
                throw view_input_error{ "use_technique_with_cached_cost", payment };
            if(is_controlled(library, card_table))
                throw view_input_error{ "use_technique_with_cached_cost", action_unavailable::controlled };
#endif
            const auto payer = *card_table[card_table.state().active_player].state().active_character;
            get<0>(executor_->context_.stack().top<detail::action_selection, substack_t>()) = detail::action_selection{
                detail::technique_selection{ .cost_offset = quote.offset_, .paid_dice = paid_dice, .energy_payer = payer }
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
            const auto quote = calculate_technique_cost(library, card_table, targets);
            return use_technique_with_cached_cost(library, card_table, random_source, quote, paid_dice);
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

        template<class TCost>
        const TCost& cached_cost(std::string_view action, action_cost_id<TCost> id) const noexcept(detail::view_checks_disabled)
        {
#ifndef NDEBUG
            validate_view();
            const auto& stack = executor_->context_.stack();
            const auto cache_end = stack.size() - detail::substack_tail_size;
            const auto cache_begin = cache_end - get<0>(stack.top<substack_t>()).size();
            if(id.window_ != get<9>(window()) || id.offset_ < cache_begin
                || id.offset_ > cache_end || cache_end - id.offset_ < sizeof(detail::cached_cost))
                throw view_input_error{ "cached_cost", action_cost_cache_error{
                    std::string{ action }, id.offset_, action_cost_cache_error::reason::expired_window } };
            // Copies retain existing quotes, while later appends may reuse the same offset in different branches.
            const auto& record = detail::cost_cache(executor_->context_.stack(), id.offset_);
            if(id.quote_ != record.identity)
                throw view_input_error{ "cached_cost", action_cost_cache_error{
                    std::string{ action }, id.offset_, action_cost_cache_error::reason::expired_window } };
            if(not record.complete)
                throw view_input_error{ "cached_cost", action_cost_cache_error{
                    std::string{ action }, id.offset_, action_cost_cache_error::reason::incomplete_calculation } };
#endif
            return detail::cost_event<TCost>(executor_->context_.stack(), id.offset_);
        }

#ifndef NDEBUG
        void validate_view() const
        {
            executor_->template validate_view<execution_state::action_selection>(version_);
        }


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

        template<class TCost>
        void begin_cost_calculation(std::string_view action, std::size_t index, const TCost& event) const
        {
            auto offset = get<8>(window());
            const auto& stack = executor_->context_.stack();
            while(offset != SIZE_MAX)
            {
                const auto& record = detail::cost_cache(stack, offset);
                if(record.type == detail::cost_event_types::index_of<TCost>())
                {
                    const auto& previous = detail::cost_event<TCost>(stack, offset);
                    const bool same = [&]
                    {
                        if constexpr(std::is_same_v<TCost, cost_of_switch>) return previous.target == event.target;
                        else if constexpr(std::is_same_v<TCost, cost_of_card>) return previous.card == event.card && previous.targets == event.targets;
                        else if constexpr(std::is_same_v<TCost, cost_of_skill>) return previous.skill == event.skill && previous.targets == event.targets;
                        else return previous.technique == event.technique && previous.targets == event.targets;
                    }();
                    if(same)
                        throw view_input_error{ "calculate_cost", action_cost_cache_error{
                            std::string{ action }, index, record.complete ? action_cost_cache_error::reason::already_calculated
                                : action_cost_cache_error::reason::incomplete_calculation } };
                }
                offset = record.previous;
            }
        }


        template<class TTarget, class TValidate>
        static void validate_targets(std::string_view operation, std::span<const TTarget> targets, TValidate&& validate)
        {
            const auto selected_targets = detail::select_action_targets(targets);
            targets = detail::action_targets(selected_targets);
            const auto count = targets.size();
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
