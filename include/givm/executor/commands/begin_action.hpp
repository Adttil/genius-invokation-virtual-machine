#ifndef GIVM_EXECUTOR_COMMANDS_BEGIN_ACTION_HPP
#define GIVM_EXECUTOR_COMMANDS_BEGIN_ACTION_HPP

#include "../executor.hpp"

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include "../broadcast.hpp"
#include "../../definition/events.hpp"
#include "../../definition/commands.hpp"

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

        struct elemental_tuning_selection
        {
            stack_count_t card_index = 0;
            elemental_dice from;
        };

        using action_selection = std::variant<
            round_end_selection, switch_selection, card_selection, skill_selection, elemental_tuning_selection
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
    using switch_handler_id = handler_id<cost_of_switch>;
    using card_cost_handler_id = handler_id<cost_of_card>;
    using skill_cost_handler_id = handler_id<cost_of_skill>;

    // Offsets count execute_fn entries from the start of this command. They never
    // become runtime state or require a second dispatch after fetching the instruction.
    inline constexpr std::size_t prepare_action_phase_offset = 0;
    inline constexpr std::size_t action_phase_broadcast_offset = 1;
    inline constexpr std::size_t before_action_offset = 2;
    inline constexpr std::size_t before_action_with_switch_offset = 3;
    inline constexpr std::size_t before_action_broadcast_offset = 4;
    inline constexpr std::size_t execute_action_selection_offset = 5;
    inline constexpr std::size_t switch_onpay_offset = 6;
    inline constexpr std::size_t switch_dice_payment_offset = 7;
    inline constexpr std::size_t switch_dice_energy_payment_offset = 8;
    inline constexpr std::size_t switch_energy_payment_offset = 9;
    inline constexpr std::size_t switch_action_offset = 10;
    inline constexpr std::size_t switch_action_apply_offset = 11;

    template<bool Observed>
    inline constexpr std::size_t switch_action_broadcast_offset = 11 + Observed;
    template<bool Observed>
    inline constexpr std::size_t card_onpay_offset = 12 + Observed;
    template<bool Observed>
    inline constexpr std::size_t card_dice_payment_offset = 13 + Observed;
    template<bool Observed>
    inline constexpr std::size_t card_dice_energy_payment_offset = 14 + Observed;
    template<bool Observed>
    inline constexpr std::size_t card_energy_payment_offset = 15 + Observed;
    template<bool Observed>
    inline constexpr std::size_t prepare_card_play_offset = 16 + Observed;
    template<bool Observed>
    inline constexpr std::size_t card_will_be_played_broadcast_offset = 17 + Observed;
    template<bool Observed>
    inline constexpr std::size_t after_card_effect_offset = 18 + Observed;
    template<bool Observed>
    inline constexpr std::size_t card_played_broadcast_offset = 19 + Observed;
    template<bool Observed>
    inline constexpr std::size_t skill_onpay_offset = 20 + Observed;
    template<bool Observed>
    inline constexpr std::size_t skill_dice_payment_offset = 21 + Observed;
    template<bool Observed>
    inline constexpr std::size_t skill_dice_energy_payment_offset = 22 + Observed;
    template<bool Observed>
    inline constexpr std::size_t skill_energy_payment_offset = 23 + Observed;
    template<bool Observed>
    inline constexpr std::size_t prepare_skill_use_offset = 24 + Observed;
    template<bool Observed>
    inline constexpr std::size_t skill_will_be_used_broadcast_offset = 25 + Observed;
    template<bool Observed>
    inline constexpr std::size_t after_skill_effect_offset = 26 + Observed;
    template<bool Observed>
    inline constexpr std::size_t skill_used_broadcast_offset = 27 + Observed;
    template<bool Observed>
    inline constexpr std::size_t elemental_tuning_modification_broadcast_offset = 28 + Observed;
    template<bool Observed>
    inline constexpr std::size_t elemental_tuning_completed_broadcast_offset = 29 + Observed;
    template<bool Observed>
    inline constexpr std::size_t first_round_end_broadcast_offset = 30 + Observed;
    template<bool Observed>
    inline constexpr std::size_t second_round_end_broadcast_offset = 31 + Observed;

    template<std::size_t From, std::size_t To>
    execution_state jump_to_action_instruction(execution_context& context) noexcept
    {
        return context.jump(context.position() - From * sizeof(execute_fn) + To * sizeof(execute_fn));
    }

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
            auto response = execution_context::make_handle_context(stack, card_table, random);
            const auto handler_id = get<0>(frame)[column];
            const auto entry = std::visit([&](auto handler) -> program_entry
            {
                const auto entity = card_table[handler];
                if(entity)
                {
                    return library[entity.definition_id()].template handle<cost_of_switch>(
                        entity, get<1>(frame)[cost_index], response
                    );
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
            auto response = execution_context::make_handle_context(stack, card_table, random);
            const auto handler_id = get<0>(frame)[column];
            const auto entry = std::visit([&](auto handler) -> program_entry
            {
                const auto entity = card_table[handler];
                if(entity)
                {
                    return library[entity.definition_id()].template handle<cost_of_card>(
                        entity, get<1>(frame)[cost_index], response
                    );
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
            auto response = execution_context::make_handle_context(stack, card_table, random);
            const auto handler_id = get<0>(frame)[column];
            const auto entry = std::visit([&](auto handler) -> program_entry
            {
                const auto entity = card_table[handler];
                if(entity)
                {
                    return library[entity.definition_id()].template handle<cost_of_skill>(
                        entity, get<1>(frame)[cost_index], response
                    );
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

    inline execution_state prepare_action_phase(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        prepare_broadcast(library, action_phase_started{}, table, context.stack(), context.position() + sizeof(execute_fn));
        return context.enter_next();
    }

    template<bool Observed>
    execution_state broadcast_action_phase(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<action_phase_started>(library, table, context, random))
        {
            return continue_execution;
        }
        pop_broadcast<action_phase_started>(context);
        context.enter_next();
        if constexpr(Observed)
        {
            return context.yield(execution_state::action_started);
        }
        return continue_execution;
    }

    template<bool Switch, bool Observed>
    execution_state prepare_before_action(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        if constexpr(Switch)
        {
            auto& state = table.state();
            if(not state.first_ended)
            {
                state.active_player = other_player(state.active_player);
            }
        }
        constexpr auto from = Switch ? before_action_with_switch_offset : before_action_offset;
        jump_to_action_instruction<from, before_action_broadcast_offset>(context);
        prepare_broadcast(library, before_action{}, table, context.stack(), context.position());
        if constexpr(Switch && Observed)
        {
            return context.yield(execution_state::action_started);
        }
        return continue_execution;
    }

    inline execution_state prepare_action_selection(
        const definition_library& library, unrestricted_table& table, execution_context& context
    )
    {
        const auto player = table[table.state().active_player];
        const auto active_character = player.state().active_character;
        GIVM_ASSERT(active_character.has_value());
        const auto is_switch_target = [active = *active_character](const auto& character)
        {
            return character.id() != active && character.state().health != 0;
        };

        stack_count_t switch_count = 0;
        for(auto character : player.characters())
        {
            if(is_switch_target(character))
            {
                ++switch_count;
            }
        }

        const auto cost_handlers = collect_all_broadcast_targets<cost_of_switch>(library, table);
        const auto handler_count = static_cast<stack_count_t>(cost_handlers.size());
        const auto matrix_size = switch_count * handler_count;
        const auto card_count = static_cast<stack_count_t>(player.hand_card_count());
        const auto card_handlers = card_count == 0 ? std::vector<card_cost_handler_id>{}
            : collect_all_broadcast_targets<cost_of_card>(library, table);
        const auto is_active_skill = [&library](const auto& skill)
        {
            return library[skill.definition_id()].template can_handle<skill_effect, skill_view>();
        };
        stack_count_t skill_count = 0;
        for(auto skill : table[*active_character].skills())
        {
            if(is_active_skill(skill))
            {
                ++skill_count;
            }
        }
        const auto skill_handlers = skill_count == 0 ? std::vector<skill_cost_handler_id>{}
            : collect_all_broadcast_targets<cost_of_skill>(library, table);
        auto&& [stored_skill_handlers, skill_costs, skill_onpay_entries, skill_onpay_offsets, skill_onpay_sizes,
                stored_card_handlers, card_costs, card_onpay_entries, card_onpay_offsets, card_onpay_sizes,
                handlers, costs, onpay_entries, onpay_offsets, onpay_sizes, onpay_cursor, selection, cached_inputs] = context.stack().push(
            dynamic_array<skill_cost_handler_id>(skill_handlers),
            dynamic_array<cost_of_skill>(skill_count),
            dynamic_array<program_entry>(skill_count * skill_handlers.size()),
            dynamic_array<std::size_t>(skill_count * skill_handlers.size()),
            dynamic_array<std::size_t>(skill_count * skill_handlers.size()),
            dynamic_array<card_cost_handler_id>(card_handlers),
            dynamic_array<cost_of_card>(card_count),
            dynamic_array<program_entry>(card_count * card_handlers.size()),
            dynamic_array<std::size_t>(card_count * card_handlers.size()),
            dynamic_array<std::size_t>(card_count * card_handlers.size()),
            dynamic_array<switch_handler_id>(cost_handlers),
            dynamic_array<cost_of_switch>(switch_count),
            dynamic_array<program_entry>(matrix_size),
            dynamic_array<std::size_t>(matrix_size),
            dynamic_array<std::size_t>(matrix_size),
            stack_count_t{},
            action_selection{},
            substack()
        );

        stack_count_t skill_index = 0;
        for(auto skill : table[*active_character].skills())
        {
            if(is_active_skill(skill))
            {
                std::construct_at(&skill_costs[skill_index++], cost_of_skill{ .skill = skill.id() });
            }
        }

        stack_count_t card_index = 0;
        for(auto card : player.hand_cards())
        {
            std::construct_at(&card_costs[card_index++], cost_of_card{
                .card = card.id(), .requirement = { .speed = action_speed::fast }
            });
        }

        stack_count_t index = 0;
        for(auto character : player.characters())
        {
            if(is_switch_target(character))
            {
                std::construct_at(&costs[index++], default_switch_cost(character.id()));
            }
        }
        return context.yield(execution_state::action_selection);
    }

    inline execution_state broadcast_before_action(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<before_action>(library, table, context, random))
        {
            return continue_execution;
        }
        pop_broadcast<before_action>(context);
        context.enter_next();
        return prepare_action_selection(library, table, context);
    }

    template<bool Observed>
    execution_state execute_action_selection(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        auto&& [costs, onpay_entries, onpay_offsets, onpay_sizes, onpay_cursor, selection, cached_inputs] =
            context.stack().top<
                cost_of_switch[],
                program_entry[], std::size_t[], std::size_t[],
                stack_count_t,
                action_selection, substack_t
            >();
        if(std::holds_alternative<round_end_selection>(selection))
        {
            context.stack().pop<
                skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
                card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
                switch_handler_id[],
                cost_of_switch[],
                program_entry[], std::size_t[], std::size_t[],
                stack_count_t,
                action_selection, substack_t
            >();
            auto& state = table.state();
            const bool is_first = not state.first_ended;
            if(is_first)
            {
                state.first_ended = true;
            }
            if(is_first)
            {
                jump_to_action_instruction<execute_action_selection_offset, first_round_end_broadcast_offset<Observed>>(context);
            }
            else
            {
                jump_to_action_instruction<execute_action_selection_offset, second_round_end_broadcast_offset<Observed>>(context);
            }
            prepare_broadcast(library, round_end_declared{}, table, context.stack(), context.position());
            if constexpr(Observed)
            {
                return context.yield(execution_state::round_end_declared);
            }
            return continue_execution;
        }

        if(const auto* selected = std::get_if<elemental_tuning_selection>(&selection))
        {
            const auto card = get<0>(context.stack().top<
                cost_of_card[], program_entry[], std::size_t[], std::size_t[],
                switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
                stack_count_t, action_selection, substack_t
            >())[selected->card_index].card;
            const auto active = *table[card.player_id].state().active_character;
            const elemental_tuning_modification event{
                .card = card, .from = selected->from,
                .to = static_cast<elemental_dice>(table[active].state().element)
            };
            jump_to_action_instruction<
                execute_action_selection_offset, elemental_tuning_modification_broadcast_offset<Observed>
            >(context);
            prepare_broadcast(library, event, table, context.stack(), context.position());
            return continue_execution;
        }

        onpay_cursor = 0;
        if(std::holds_alternative<skill_selection>(selection))
        {
            jump_to_action_instruction<execute_action_selection_offset, skill_onpay_offset<Observed>>(context);
            context.stack().push(context.position());
            return continue_execution;
        }
        if(const auto* selected = std::get_if<card_selection>(&selection))
        {
            const auto card_index = selected->card_cost_index;
            const auto& cost = get<0>(context.stack().top<
                cost_of_card[], program_entry[], std::size_t[], std::size_t[],
                switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
                stack_count_t, action_selection, substack_t
            >())[card_index];
            table[cost.card].erase();
            jump_to_action_instruction<execute_action_selection_offset, card_onpay_offset<Observed>>(context);
            context.stack().push(context.position());
            return continue_execution;
        }

#ifndef NDEBUG
        const auto* selected = std::get_if<switch_selection>(&selection);
        GIVM_ASSERT(selected != nullptr);
        GIVM_ASSERT(selected->switch_cost_index < costs.size());
#endif

        context.enter_next();
        context.stack().push(context.position());
        return continue_execution;
    }

    template<std::size_t From, std::size_t Payment, std::size_t Next>
    execution_state pay_action_cost(
        const definition_library& library, unrestricted_table& table, execution_context& context,
        player_id player, const dice_counts& paid_dice, std::uint32_t energy
    )
    {
        auto& player_state = table[player].state();
        const bool has_dice_payment = paid_dice.total() != 0;
        if(has_dice_payment)
        {
            player_state.dice -= paid_dice;
        }
        if(energy != 0)
        {
            const auto character = *player_state.active_character;
            auto& character_energy = table[character].state().energy;
            const energy_changed event{
                .target = character, .previous = character_energy, .current = character_energy - energy
            };
            character_energy -= energy;
            if(has_dice_payment)
            {
                context.stack().push(event);
                jump_to_action_instruction<From, Payment + 1>(context);
                prepare_broadcast(library, dice_removed{ .player = player, .dice = paid_dice }, table, context.stack(), context.position());
                return continue_execution;
            }
            jump_to_action_instruction<From, Payment + 2>(context);
            prepare_broadcast(library, event, table, context.stack(), context.position());
            return continue_execution;
        }
        if(has_dice_payment)
        {
            jump_to_action_instruction<From, Payment>(context);
            prepare_broadcast(library, dice_removed{ .player = player, .dice = paid_dice }, table, context.stack(), context.position());
            return continue_execution;
        }
        return jump_to_action_instruction<From, Next>(context);
    }

    inline execution_state continue_switch_onpay(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        auto [action_frame, return_frame] = context.stack().top<frame<
                switch_handler_id[],
                cost_of_switch[],
                program_entry[], std::size_t[], std::size_t[],
                stack_count_t,
                action_selection, substack_t
            >, frame<execution_position>>();
        auto&& [handlers, costs, onpay_entries, onpay_offsets, onpay_sizes, onpay_cursor, selection, cached_inputs] =
            action_frame;
        const auto* selected = std::get_if<switch_selection>(&selection);
        GIVM_ASSERT(selected != nullptr);
        GIVM_ASSERT(selected->switch_cost_index < costs.size());
        const auto handler_count = static_cast<stack_count_t>(handlers.size());
        const auto row_begin = selected->switch_cost_index * handler_count;
        while(onpay_cursor < handler_count)
        {
            const auto column = onpay_cursor++;
            const auto index = row_begin + column;
            const auto entry = onpay_entries[index];
            if(entry)
            {
                const auto offset = onpay_offsets[index];
                const auto size = onpay_sizes[index];
                auto& stack = context.stack();
                const auto capacity = stack.size() + size;
                if(capacity > stack.capacity()) stack.reserve(std::bit_ceil(capacity));
                auto invoke = context.make_program_invoker();
                return context.enter(invoke(entry, std::span<const unsigned char>{ stack.data() + offset, size }));
            }
        }

        const auto paid_dice = selected->paid_dice;
        const auto energy = costs[selected->switch_cost_index].requirement.energy;
        context.stack().pop<execution_position>();
        return pay_action_cost<switch_onpay_offset, switch_dice_payment_offset, switch_action_offset>(
            library, table, context, table.state().active_player, paid_dice, energy
        );
    }

    template<std::size_t From, std::size_t To>
    execution_state broadcast_action_dice_payment(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<dice_removed>(library, table, context, random))
        {
            return continue_execution;
        }
        pop_broadcast<dice_removed>(context);
        return jump_to_action_instruction<From, To>(context);
    }

    inline execution_state broadcast_action_dice_energy_payment(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<dice_removed>(library, table, context, random))
        {
            return continue_execution;
        }
        pop_broadcast<dice_removed>(context);
        const auto event = get<0>(context.stack().top<energy_changed>());
        context.stack().pop<energy_changed>();
        prepare_broadcast(library, event, table, context.stack(), context.position() + sizeof(execute_fn));
        return context.enter_next();
    }

    inline execution_state broadcast_action_energy_payment(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<energy_changed>(library, table, context, random))
        {
            return continue_execution;
        }
        pop_broadcast<energy_changed>(context);
        return context.enter_next();
    }

    template<bool Observed>
    execution_state execute_switch_action(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        auto&& [costs, onpay_entries, onpay_offsets, onpay_sizes, onpay_cursor, selection, cached_inputs] =
            context.stack().top<
                cost_of_switch[],
                program_entry[], std::size_t[], std::size_t[],
                stack_count_t,
                action_selection, substack_t
            >();
        const auto* selected = std::get_if<switch_selection>(&selection);
        GIVM_ASSERT(selected != nullptr);
        GIVM_ASSERT(selected->switch_cost_index < costs.size());
        const auto target = costs[selected->switch_cost_index].target;
        GIVM_ASSERT(static_cast<bool>(table[target]));
        if constexpr(not Observed)
        {
            table[target.player_id].state().active_character = target;
        }
        prepare_broadcast(library, active_character_changed{ .current = target }, table, context.stack(),
            context.position() + (switch_action_broadcast_offset<Observed> - switch_action_offset) * sizeof(execute_fn));
        context.enter_next();
        if constexpr(Observed)
        {
            return context.yield(execution_state::active_character_changed);
        }
        return continue_execution;
    }

    template<bool Observed>
    execution_state broadcast_switch_action(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<active_character_changed>(library, table, context, random))
        {
            return continue_execution;
        }
        pop_broadcast<active_character_changed>(context);

        auto&& [costs, onpay_entries, onpay_offsets, onpay_sizes, onpay_cursor, selection, cached_inputs] =
            context.stack().top<
                cost_of_switch[],
                program_entry[], std::size_t[], std::size_t[],
                stack_count_t,
                action_selection, substack_t
            >();
        const auto* selected = std::get_if<switch_selection>(&selection);
        GIVM_ASSERT(selected != nullptr);
        GIVM_ASSERT(selected->switch_cost_index < costs.size());
        const auto speed = costs[selected->switch_cost_index].requirement.speed;
        context.stack().pop<
            skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[],
            cost_of_switch[],
            program_entry[], std::size_t[], std::size_t[],
            stack_count_t,
            action_selection, substack_t
        >();
        if(speed == action_speed::combat)
        {
            return jump_to_action_instruction<switch_action_broadcast_offset<Observed>, before_action_with_switch_offset>(context);
        }
        return jump_to_action_instruction<switch_action_broadcast_offset<Observed>, before_action_offset>(context);
    }

    inline execution_state apply_switch_action(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        const auto& event = get<0>(context.stack().top<active_character_changed, execution_position>());
        table[event.current.player_id].state().active_character = event.current;
        context.enter_next();
        return broadcast_switch_action<true>(library, table, context, random);
    }

    template<bool Observed>
    execution_state continue_card_onpay(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        auto [action_frame, return_frame] = context.stack().top<frame<
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >, frame<execution_position>>();
        auto&& [handlers, costs, onpay_entries, onpay_offsets, onpay_sizes,
                switch_handlers, switch_costs, switch_onpay_entries, switch_onpay_offsets, switch_onpay_sizes,
                onpay_cursor, selection, cached_inputs] = action_frame;
        const auto* selected = std::get_if<card_selection>(&selection);
        GIVM_ASSERT(selected != nullptr);
        const auto handler_count = static_cast<stack_count_t>(handlers.size());
        const auto row_begin = selected->card_cost_index * handler_count;
        while(onpay_cursor < handler_count)
        {
            const auto column = onpay_cursor++;
            const auto index = row_begin + column;
            const auto entry = onpay_entries[index];
            if(entry)
            {
                const auto offset = onpay_offsets[index];
                const auto size = onpay_sizes[index];
                auto& stack = context.stack();
                const auto capacity = stack.size() + size;
                if(capacity > stack.capacity()) stack.reserve(std::bit_ceil(capacity));
                auto invoke = context.make_program_invoker();
                return context.enter(invoke(entry, std::span<const unsigned char>{ stack.data() + offset, size }));
            }
        }

        const auto paid_dice = selected->paid_dice;
        const auto& cost = costs[selected->card_cost_index];
        context.stack().pop<execution_position>();
        return pay_action_cost<card_onpay_offset<Observed>, card_dice_payment_offset<Observed>, prepare_card_play_offset<Observed>>(
            library, table, context, cost.card.player_id, paid_dice, cost.requirement.energy
        );
    }

    inline execution_state prepare_card_play(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        auto&& [costs, onpay_entries, onpay_offsets, onpay_sizes, switch_handlers, switch_costs,
                switch_onpay_entries, switch_onpay_offsets, switch_onpay_sizes, onpay_cursor, selection, cached_inputs] = context.stack().top<
            cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >();
        const auto* selected = std::get_if<card_selection>(&selection);
        GIVM_ASSERT(selected != nullptr);
        const auto& cost = costs[selected->card_cost_index];
        prepare_broadcast(library, card_will_be_played{
            .card = cost.card, .definition_id = table[cost.card].definition_id(),
            .targets = selected->targets, .speed = cost.requirement.speed
        }, table, context.stack(), context.position() + sizeof(execute_fn));
        return context.enter_next();
    }

    inline execution_state finish_card_effect(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        context.stack().pop<execution_position>();
        auto&& [costs, onpay_entries, onpay_offsets, onpay_sizes, switch_handlers, switch_costs,
                switch_onpay_entries, switch_onpay_offsets, switch_onpay_sizes, onpay_cursor, selection, cached_inputs] = context.stack().top<
            cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >();
        const auto* selected = std::get_if<card_selection>(&selection);
        GIVM_ASSERT(selected != nullptr);
        const auto& cost = costs[selected->card_cost_index];
        prepare_broadcast(library, card_played{
            .card = cost.card, .definition_id = table[cost.card].definition_id(),
            .targets = selected->targets, .speed = cost.requirement.speed
        }, table, context.stack(), context.position() + sizeof(execute_fn));
        return context.enter_next();
    }

    inline execution_state broadcast_card_will_be_played(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<card_will_be_played>(library, table, context, random))
        {
            return continue_execution;
        }
        const auto event = get<0>(context.stack().top<
            card_will_be_played, execution_position>());
        pop_broadcast<card_will_be_played>(context);
        context.enter_next();
        context.stack().push(context.position());
        if(not event.effect_cancelled)
        {
            card_effect effect{ .card = event.card, .targets = event.targets };
            // The played card is already out of hand, but retains its definition and state.
            const auto card = std::as_const(table)[event.card];
            auto response = context.make_handle_context(table, random);
            const auto entry = library[card.definition_id()].handle<card_effect>(card, effect, response);
            if(entry)
            {
                return context.enter(entry);
            }
        }
        return finish_card_effect(library, table, context, random);
    }

    template<bool Observed>
    execution_state broadcast_card_played(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<card_played>(library, table, context, random))
        {
            return continue_execution;
        }
        const auto speed = get<0>(context.stack().top<card_played, execution_position>()).speed;
        pop_broadcast<card_played>(context);
        context.stack().pop<
            skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >();
        if(speed == action_speed::combat)
        {
            return jump_to_action_instruction<card_played_broadcast_offset<Observed>, before_action_with_switch_offset>(context);
        }
        return jump_to_action_instruction<card_played_broadcast_offset<Observed>, before_action_offset>(context);
    }

    template<bool Observed>
    execution_state continue_skill_onpay(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        auto [action_frame, return_frame] = context.stack().top<frame<
            skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >, frame<execution_position>>();
        auto&& [handlers, costs, onpay_entries, onpay_offsets, onpay_sizes,
                card_handlers, card_costs, card_onpay_entries, card_onpay_offsets, card_onpay_sizes,
                switch_handlers, switch_costs, switch_onpay_entries, switch_onpay_offsets, switch_onpay_sizes,
                onpay_cursor, selection, cached_inputs] = action_frame;
        const auto* selected = std::get_if<skill_selection>(&selection);
        GIVM_ASSERT(selected != nullptr);
        const auto handler_count = static_cast<stack_count_t>(handlers.size());
        const auto row_begin = selected->skill_cost_index * handler_count;
        while(onpay_cursor < handler_count)
        {
            const auto column = onpay_cursor++;
            const auto index = row_begin + column;
            const auto entry = onpay_entries[index];
            if(entry)
            {
                const auto offset = onpay_offsets[index];
                const auto size = onpay_sizes[index];
                auto& stack = context.stack();
                const auto capacity = stack.size() + size;
                if(capacity > stack.capacity()) stack.reserve(std::bit_ceil(capacity));
                auto invoke = context.make_program_invoker();
                return context.enter(invoke(entry, std::span<const unsigned char>{ stack.data() + offset, size }));
            }
        }

        const auto paid_dice = selected->paid_dice;
        const auto& cost = costs[selected->skill_cost_index];
        context.stack().pop<execution_position>();
        return pay_action_cost<skill_onpay_offset<Observed>, skill_dice_payment_offset<Observed>, prepare_skill_use_offset<Observed>>(
            library, table, context, cost.skill.character_id.player_id, paid_dice, cost.requirement.energy
        );
    }

    inline execution_state prepare_skill_use(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        auto&& [costs, onpay_entries, onpay_offsets, onpay_sizes, card_handlers, card_costs, card_onpay_entries, card_onpay_offsets, card_onpay_sizes,
                switch_handlers, switch_costs, switch_onpay_entries, switch_onpay_offsets, switch_onpay_sizes, onpay_cursor, selection, cached_inputs] = context.stack().top<
            cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >();
        const auto& selected = std::get<skill_selection>(selection);
        const auto& cost = costs[selected.skill_cost_index];
        prepare_broadcast(library, skill_will_be_used{
            .skill = cost.skill, .targets = selected.targets, .speed = cost.requirement.speed
        }, table, context.stack(), context.position() + sizeof(execute_fn));
        return context.enter_next();
    }

    inline execution_state finish_skill_effect(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        const auto event = get<0>(context.stack().top<skill_used, execution_position>());
        context.stack().pop<skill_used, execution_position>();
        prepare_broadcast(library, event, table, context.stack(), context.position() + sizeof(execute_fn));
        return context.enter_next();
    }

    inline execution_state broadcast_skill_will_be_used(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<skill_will_be_used>(library, table, context, random))
        {
            return continue_execution;
        }
        const auto event = get<0>(context.stack().top<skill_will_be_used, execution_position>());
        pop_broadcast<skill_will_be_used>(context);
        context.enter_next();
        context.stack().push(skill_used{
            .skill = event.skill, .targets = event.targets, .speed = event.speed,
            .effect_cancelled = event.effect_cancelled
        }, context.position());
        if(not event.effect_cancelled)
        {
            skill_effect effect{ .skill = event.skill, .targets = event.targets };
            const auto skill = std::as_const(table)[event.skill];
            auto response = context.make_handle_context(table, random);
            const auto entry = library[skill.definition_id()].handle<skill_effect>(skill, effect, response);
            if(entry)
            {
                return context.enter(entry);
            }
        }
        return finish_skill_effect(library, table, context, random);
    }

    template<bool Observed>
    execution_state broadcast_skill_used(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<skill_used>(library, table, context, random))
        {
            return continue_execution;
        }
        const auto speed = get<0>(context.stack().top<skill_used, execution_position>()).speed;
        pop_broadcast<skill_used>(context);
        context.stack().pop<
            skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >();
        if(speed == action_speed::combat)
        {
            return jump_to_action_instruction<skill_used_broadcast_offset<Observed>, before_action_with_switch_offset>(context);
        }
        return jump_to_action_instruction<skill_used_broadcast_offset<Observed>, before_action_offset>(context);
    }

    inline execution_state broadcast_elemental_tuning_modification(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<elemental_tuning_modification>(library, table, context, random))
        {
            return continue_execution;
        }
        const auto event = get<0>(context.stack().top<elemental_tuning_modification, execution_position>());
        pop_broadcast<elemental_tuning_modification>(context);
        table[event.card].erase();
        auto& dice = table[event.card.player_id].state().dice;
        --dice[event.from];
        ++dice[event.to];
        prepare_broadcast(library, elemental_tuning_completed{
            .card = event.card, .from = event.from, .to = event.to
        }, table, context.stack(), context.position() + sizeof(execute_fn));
        return context.enter_next();
    }

    template<bool Observed>
    execution_state broadcast_elemental_tuning_completed(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<elemental_tuning_completed>(library, table, context, random))
        {
            return continue_execution;
        }
        pop_broadcast<elemental_tuning_completed>(context);
        context.stack().pop<
            skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >();
        return jump_to_action_instruction<elemental_tuning_completed_broadcast_offset<Observed>, before_action_offset>(context);
    }

    template<bool Observed>
    execution_state broadcast_first_round_end(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<round_end_declared>(library, table, context, random))
        {
            return continue_execution;
        }
        pop_broadcast<round_end_declared>(context);
        table.state().active_player = other_player(table.state().active_player);
        jump_to_action_instruction<first_round_end_broadcast_offset<Observed>, before_action_offset>(context);
        if constexpr(Observed)
        {
            return context.yield(execution_state::action_started);
        }
        return continue_execution;
    }

    inline execution_state broadcast_second_round_end(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<round_end_declared>(library, table, context, random))
        {
            return continue_execution;
        }
        pop_broadcast<round_end_declared>(context);
        return context.enter_next();
    }

    template<bool Observed>
    void compile_begin_action(program_writer& writer)
    {
        writer.write<execute_fn>(&prepare_action_phase);
        writer.write<execute_fn>(&broadcast_action_phase<Observed>);
        writer.write<execute_fn>(&prepare_before_action<false, Observed>);
        writer.write<execute_fn>(&prepare_before_action<true, Observed>);
        writer.write<execute_fn>(&broadcast_before_action);
        writer.write<execute_fn>(&execute_action_selection<Observed>);
        writer.write<execute_fn>(&continue_switch_onpay);
        writer.write<execute_fn>(&broadcast_action_dice_payment<switch_dice_payment_offset, switch_action_offset>);
        writer.write<execute_fn>(&broadcast_action_dice_energy_payment);
        writer.write<execute_fn>(&broadcast_action_energy_payment);
        writer.write<execute_fn>(&execute_switch_action<Observed>);
        if constexpr(Observed)
        {
            writer.write<execute_fn>(&apply_switch_action);
        }
        writer.write<execute_fn>(&broadcast_switch_action<Observed>);
        writer.write<execute_fn>(&continue_card_onpay<Observed>);
        writer.write<execute_fn>(&broadcast_action_dice_payment<card_dice_payment_offset<Observed>, prepare_card_play_offset<Observed>>);
        writer.write<execute_fn>(&broadcast_action_dice_energy_payment);
        writer.write<execute_fn>(&broadcast_action_energy_payment);
        writer.write<execute_fn>(&prepare_card_play);
        writer.write<execute_fn>(&broadcast_card_will_be_played);
        writer.write<execute_fn>(&finish_card_effect);
        writer.write<execute_fn>(&broadcast_card_played<Observed>);
        writer.write<execute_fn>(&continue_skill_onpay<Observed>);
        writer.write<execute_fn>(&broadcast_action_dice_payment<skill_dice_payment_offset<Observed>, prepare_skill_use_offset<Observed>>);
        writer.write<execute_fn>(&broadcast_action_dice_energy_payment);
        writer.write<execute_fn>(&broadcast_action_energy_payment);
        writer.write<execute_fn>(&prepare_skill_use);
        writer.write<execute_fn>(&broadcast_skill_will_be_used);
        writer.write<execute_fn>(&finish_skill_effect);
        writer.write<execute_fn>(&broadcast_skill_used<Observed>);
        writer.write<execute_fn>(&broadcast_elemental_tuning_modification);
        writer.write<execute_fn>(&broadcast_elemental_tuning_completed<Observed>);
        writer.write<execute_fn>(&broadcast_first_round_end<Observed>);
        writer.write<execute_fn>(&broadcast_second_round_end);
    }

    inline void compile(program_writer& writer, const begin_action&, compile_mode mode)
    {
        if(mode == compile_mode::observed)
        {
            compile_begin_action<true>(writer);
        }
        else
        {
            compile_begin_action<false>(writer);
        }
    }
}

#include "../../macro_undef.hpp"
#endif
