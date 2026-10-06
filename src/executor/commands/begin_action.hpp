#ifndef GIVM_EXECUTOR_COMMANDS_BEGIN_ACTION_HPP
#define GIVM_EXECUTOR_COMMANDS_BEGIN_ACTION_HPP


#include "../program_writer.hpp"

#include <givm/executor/executor.hpp>
#include <givm/executor/views/action_selection.hpp>

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include "../broadcast.hpp"
#include "remove_dice.hpp"
#include "set_active_character.hpp"
#include "use_skill.hpp"
#include <givm/definition.hpp>

#include <givm/macro_define.hpp>

namespace givm::detail
{
    // Offsets count execute_fn entries from the start of this command. They never
    // become runtime state or require a second dispatch after fetching the instruction.
    inline constexpr std::size_t prepare_action_phase_offset = 0;
    inline constexpr std::size_t action_phase_settlement_offset = 1;
    inline constexpr std::size_t action_phase_finish_offset = action_phase_settlement_offset + settlement_instruction_count;
    inline constexpr std::size_t onpay_instruction_count = 2 + settlement_instruction_count;
    inline constexpr std::size_t before_action_offset = action_phase_finish_offset + 1;
    inline constexpr std::size_t before_action_with_switch_offset = before_action_offset + 1;
    inline constexpr std::size_t before_action_settlement_offset = before_action_with_switch_offset + 1;
    inline constexpr std::size_t before_action_finish_offset = before_action_settlement_offset + settlement_instruction_count;
    inline constexpr std::size_t execute_action_selection_offset = before_action_finish_offset + 1;
    inline constexpr std::size_t switch_onpay_offset = execute_action_selection_offset + 1;
    inline constexpr std::size_t switch_action_offset = switch_onpay_offset + onpay_instruction_count;
    inline constexpr std::size_t switch_action_apply_offset = switch_action_offset + 1;
    template<bool Observed>
    inline constexpr std::size_t switch_action_settlement_offset = switch_action_apply_offset + Observed;
    template<bool Observed>
    inline constexpr std::size_t switch_action_finish_offset = switch_action_settlement_offset<Observed> + settlement_instruction_count;
    template<bool Observed>
    inline constexpr std::size_t card_onpay_offset = switch_action_finish_offset<Observed> + 1;
    template<bool Observed>
    inline constexpr std::size_t prepare_card_play_offset = card_onpay_offset<Observed> + onpay_instruction_count;
    template<bool Observed>
    inline constexpr std::size_t card_will_be_played_broadcast_offset = prepare_card_play_offset<Observed> + 1;
    template<bool Observed>
    inline constexpr std::size_t after_card_effect_offset = card_will_be_played_broadcast_offset<Observed> + response_instruction_count<card_will_be_played>;
    template<bool Observed>
    inline constexpr std::size_t card_played_settlement_offset = after_card_effect_offset<Observed> + response_instruction_count<card_effect>;
    template<bool Observed>
    inline constexpr std::size_t card_played_finish_offset = card_played_settlement_offset<Observed> + settlement_instruction_count;
    template<bool Observed>
    inline constexpr std::size_t skill_onpay_offset = card_played_finish_offset<Observed> + 1;
    template<bool Observed>
    inline constexpr std::size_t prepare_skill_use_offset = skill_onpay_offset<Observed> + onpay_instruction_count;
    template<bool Observed>
    inline constexpr std::size_t skill_will_be_used_broadcast_offset = prepare_skill_use_offset<Observed> + 1;
    template<bool Observed>
    inline constexpr std::size_t after_skill_effect_offset = skill_will_be_used_broadcast_offset<Observed> + response_instruction_count<skill_will_be_used>;
    template<bool Observed>
    inline constexpr std::size_t skill_used_settlement_offset = after_skill_effect_offset<Observed> + response_instruction_count<skill_effect>;
    template<bool Observed>
    inline constexpr std::size_t skill_used_finish_offset = skill_used_settlement_offset<Observed> + settlement_instruction_count;
    template<bool Observed>
    inline constexpr std::size_t technique_onpay_offset = skill_used_finish_offset<Observed> + 1;
    template<bool Observed>
    inline constexpr std::size_t prepare_technique_use_offset = technique_onpay_offset<Observed> + onpay_instruction_count;
    template<bool Observed>
    inline constexpr std::size_t technique_will_be_used_broadcast_offset = prepare_technique_use_offset<Observed> + 1;
    template<bool Observed>
    inline constexpr std::size_t after_technique_effect_offset = technique_will_be_used_broadcast_offset<Observed> + response_instruction_count<technique_will_be_used>;
    template<bool Observed>
    inline constexpr std::size_t technique_used_settlement_offset = after_technique_effect_offset<Observed> + response_instruction_count<technique_effect>;
    template<bool Observed>
    inline constexpr std::size_t technique_used_finish_offset = technique_used_settlement_offset<Observed> + settlement_instruction_count;
    template<bool Observed>
    inline constexpr std::size_t elemental_tuning_modification_broadcast_offset = technique_used_finish_offset<Observed> + 1;
    template<bool Observed>
    inline constexpr std::size_t elemental_tuning_completed_settlement_offset = elemental_tuning_modification_broadcast_offset<Observed> + response_instruction_count<elemental_tuning_modification>;
    template<bool Observed>
    inline constexpr std::size_t elemental_tuning_completed_finish_offset = elemental_tuning_completed_settlement_offset<Observed> + settlement_instruction_count;
    template<bool Observed>
    inline constexpr std::size_t first_round_end_settlement_offset = elemental_tuning_completed_finish_offset<Observed> + 1;
    template<bool Observed>
    inline constexpr std::size_t first_round_end_finish_offset = first_round_end_settlement_offset<Observed> + settlement_instruction_count;
    template<bool Observed>
    inline constexpr std::size_t prepared_skill_removal_offset = first_round_end_finish_offset<Observed> + 1;
    template<bool Observed>
    inline constexpr std::size_t prepared_skill_start_offset = prepared_skill_removal_offset<Observed> + settlement_instruction_count;
    template<bool Observed>
    inline constexpr std::size_t prepared_skill_effect_offset = prepared_skill_start_offset<Observed> + 1;
    template<bool Observed>
    inline constexpr std::size_t prepared_skill_settlement_offset = prepared_skill_effect_offset<Observed> + response_instruction_count<prepared_skill_effect>;
    template<bool Observed>
    inline constexpr std::size_t prepared_skill_finish_offset = prepared_skill_settlement_offset<Observed> + settlement_instruction_count;
    template<bool Observed>
    inline constexpr std::size_t second_round_end_settlement_offset = prepared_skill_finish_offset<Observed> + 1;

    inline execution_state complete_onpay_response(const definition_library&, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        table.state().self_player = get<0>(context.stack().top<response_return>()).previous_player;
        end_response<true>(context);
        context.stack().pop<response_return>();
        return context.jump(get<0>(context.stack().top<response_return>()).position);
    }

    inline void compile_onpay_response(program_writer& writer, execute_fn continuation)
    {
        writer.write(continuation);
        compile_settlement(writer, begin_settlement);
        writer.write(execute_fn{ complete_onpay_response });
    }

    template<std::size_t From, std::size_t To>
    inline execution_state jump_to_action_instruction(execution_context& context) noexcept
    {
        return context.jump(context.position() - From * sizeof(execute_fn) + To * sizeof(execute_fn));
    }

    inline execution_state prepare_action_phase(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&
    )
    {
        begin_response<true>(context);
        append_event_record(context, action_phase_started{});
        return context.enter_next();
    }

    template<bool Observed>
    inline execution_state broadcast_action_phase(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        end_response<true>(context);
        context.enter_next();
        if constexpr(Observed)
        {
            return context.yield(execution_state::action_started);
        }
        return continue_execution;
    }

    template<bool Switch, bool Observed>
    inline execution_state prepare_before_action(
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
        jump_to_action_instruction<from, before_action_settlement_offset>(context);
        begin_response<true>(context);
        append_event_record(context, before_action{});
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
        const auto active = table[*active_character];
        const bool has_technique = active.has(equipment_type::technique)
            && library[active.get(equipment_type::technique).definition_id()].can_handle<technique_effect, attachment_view>();
        const auto technique_handlers = has_technique ? collect_all_broadcast_targets<cost_of_technique>(library, table)
            : std::vector<technique_cost_handler_id>{};
        auto&& [
#ifndef NDEBUG
                debug_cost_states,
#endif
                stored_technique_handlers, technique_costs, technique_onpay_entries, technique_onpay_offsets, technique_onpay_sizes,
                stored_skill_handlers, skill_costs, skill_onpay_entries, skill_onpay_offsets, skill_onpay_sizes,
                stored_card_handlers, card_costs, card_onpay_entries, card_onpay_offsets, card_onpay_sizes,
                handlers, costs, onpay_entries, onpay_offsets, onpay_sizes, onpay_cursor, selection, cached_inputs] = context.stack().push(
#ifndef NDEBUG
            dynamic_array<std::uint8_t>(switch_count + card_count + skill_count + (has_technique ? 1uz : 0uz)),
#endif
            dynamic_array<technique_cost_handler_id>(technique_handlers),
            dynamic_array<cost_of_technique>(has_technique ? 1uz : 0uz),
            dynamic_array<program_entry>(technique_handlers.size()),
            dynamic_array<std::size_t>(technique_handlers.size()),
            dynamic_array<std::size_t>(technique_handlers.size()),
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

#ifndef NDEBUG
        std::ranges::fill(debug_cost_states, std::uint8_t{});
#endif

        if(has_technique)
        {
            std::construct_at(&technique_costs[0], cost_of_technique{ .technique = active.get(equipment_type::technique).id() });
        }

        stack_count_t skill_index = 0;
        for(auto skill : table[*active_character].skills())
        {
            if(is_active_skill(skill))
            {
                auto flags = library.skill_flags(skill.definition_id());
                if(flags.contains(skill_flag_bits::normal_attack))
                {
                    if(player.state().dice.total() % 2 == 0) flags.set(skill_flag_bits::charged_attack);
                    if(player.state().can_plunge) flags.set(skill_flag_bits::plunging_attack);
                }
                std::construct_at(&skill_costs[skill_index++], cost_of_skill{ .skill = skill.id(), .flags = flags });
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

    template<bool Observed>
    inline execution_state finish_prepared_skill_action(const definition_library&, unrestricted_table&,
        execution_context& context, random_fn&)
    {
        const auto speed = get<0>(context.stack().top<action_speed>());
        context.stack().pop<action_speed>();
        end_response<true>(context);
        if(speed == action_speed::combat)
            return jump_to_action_instruction<prepared_skill_finish_offset<Observed>, before_action_with_switch_offset>(context);
        return jump_to_action_instruction<prepared_skill_finish_offset<Observed>, before_action_offset>(context);
    }

    inline execution_state finish_prepared_skill_effect(const definition_library& library,
        unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_single_response<prepared_skill_effect, attachment_id>(library, table, context, random))
            return continue_execution;
        const auto speed = get<0>(context.stack().top<prepared_skill_effect, response_return>()).speed;
        pop_single_response<prepared_skill_effect, attachment_id>(context);
        context.stack().push(speed);
        return context.advance(response_extent<prepared_skill_effect>);
    }

    template<bool Observed>
    inline execution_state continue_prepared_skill_removal(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        context.enter_next();
        const auto event = get<0>(context.stack().top<prepared_skill_effect>());
        context.stack().pop<prepared_skill_effect>();
        // The consumed attachment retains its identity and state for its own effect.
        prepare_single_response(event, event.attachment, table, context, context.position(), true);
        auto& stack = context.stack();
        const auto frame = stack.top<prepared_skill_effect, response_return>();
        auto effect = get<0>(frame);
        const auto speed_offset = reinterpret_cast<const unsigned char*>(&get<0>(frame).speed) - stack.data();
        const auto attachment = std::as_const(table)[effect.attachment];
        auto response = context.make_handle_context(library, attachment, random);
        stack.push(response_return{ table.state().self_player, get<1>(frame).position + sizeof(execute_fn) });
        begin_response<true>(context);
        const auto entry = library[attachment.definition_id()].handle<prepared_skill_effect>(effect, response, 0);
        // Keep the action's speed across input packing before entering its effect.
        *reinterpret_cast<action_speed*>(stack.data() + speed_offset) = effect.speed;
        if(effect.speed == action_speed::combat)
            table[effect.attachment.character_id.player_id].state().can_plunge = false;
        if(entry)
        {
            table.state().self_player = effect.attachment.character_id.player_id;
            return context.enter(entry);
        }
        return complete_single_response<prepared_skill_effect, attachment_id>(library, table, context, random);
    }

    template<bool Observed>
    inline execution_state broadcast_before_action(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        end_response<true>(context);
        const auto active = *table[table.state().active_player].state().active_character;
        std::optional<attachment_id> prepared;
        for(auto attachment : table[active].attachments())
        {
            const auto definition = attachment.definition_id();
            if(library.is_control(definition))
            {
                context.enter_next();
                return prepare_action_selection(library, table, context);
            }
            if(not prepared && library[definition].can_handle<prepared_skill_effect, attachment_view>())
                prepared = attachment.id();
        }
        if(prepared)
        {
            jump_to_action_instruction<before_action_finish_offset, prepared_skill_removal_offset<Observed>>(context);
            begin_response<true>(context);
            context.stack().push(prepared_skill_effect{ .attachment = *prepared });
            table[*prepared].erase();
            append_removal_record<attachment_removal_effect>(context, *prepared, attachment_removed{ *prepared });
            return continue_execution;
        }
        context.enter_next();
        return prepare_action_selection(library, table, context);
    }

    template<bool Observed>
    inline execution_state execute_action_selection(
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
        begin_response<true>(context);
        if(std::holds_alternative<round_end_selection>(selection))
        {
            context.stack().pop<
#ifndef NDEBUG
                std::uint8_t[],
#endif
                technique_cost_handler_id[], cost_of_technique[], program_entry[], std::size_t[], std::size_t[],
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
                jump_to_action_instruction<execute_action_selection_offset, first_round_end_settlement_offset<Observed>>(context);
            }
            else
            {
                jump_to_action_instruction<execute_action_selection_offset, second_round_end_settlement_offset<Observed>>(context);
            }
            append_event_record(context, round_end_declared{});
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
        if(std::holds_alternative<technique_selection>(selection))
        {
            jump_to_action_instruction<execute_action_selection_offset, technique_onpay_offset<Observed>>(context);
            context.stack().push(response_return{ table.state().self_player, context.position() });
            return continue_execution;
        }
        if(std::holds_alternative<skill_selection>(selection))
        {
            jump_to_action_instruction<execute_action_selection_offset, skill_onpay_offset<Observed>>(context);
            context.stack().push(response_return{ table.state().self_player, context.position() });
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
            context.stack().push(response_return{ table.state().self_player, context.position() });
            return continue_execution;
        }

#ifndef NDEBUG
        const auto* selected = std::get_if<switch_selection>(&selection);
        GIVM_ASSERT(selected != nullptr);
        GIVM_ASSERT(selected->switch_cost_index < costs.size());
#endif

        context.enter_next();
        context.stack().push(response_return{ table.state().self_player, context.position() });
        return continue_execution;
    }

    template<std::size_t From, std::size_t Next>
    inline execution_state pay_action_cost(const definition_library&, unrestricted_table& table,
        execution_context& context, player_id player, const dice_counts& paid_dice, std::uint32_t energy)
    {
        auto& player_state = table[player].state();
        if(paid_dice.total() != 0)
        {
            player_state.dice -= paid_dice;
            append_event_record(context, dice_removed{ .player = player, .dice = paid_dice });
        }
        if(energy != 0)
        {
            const auto character = *player_state.active_character;
            auto& character_energy = table[character].state().energy;
            const energy_changed event{ .target = character, .previous = character_energy,
                .current = character_energy - energy };
            character_energy -= energy;
            append_event_record(context, event);
        }
        return jump_to_action_instruction<From, Next>(context);
    }

    inline execution_state continue_switch_onpay(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        auto [action_frame, return_frame] = context.stack().top<frame<
                switch_handler_id[],
                cost_of_switch[],
                program_entry[], std::size_t[], std::size_t[],
                stack_count_t,
                action_selection, substack_t
            >, frame<response_return>>();
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
                const auto player = std::visit([&](auto id) { return table[id].player().id(); }, handlers[column]);
                const auto offset = onpay_offsets[index];
                const auto size = onpay_sizes[index];
                auto& stack = context.stack();
                stack.push(response_return{ table.state().self_player, get<0>(return_frame).position + sizeof(execute_fn) });
                const auto capacity = stack.size() + size;
                if(capacity > stack.capacity()) stack.reserve(std::bit_ceil(capacity));
                begin_response<true>(context);
                const auto prepared = context.copy_program_inputs(
                    entry, std::span<const unsigned char>{ stack.data() + offset, size });
                table.state().self_player = player;
                return context.enter(prepared);
            }
        }

        const auto paid_dice = selected->paid_dice;
        const auto energy = costs[selected->switch_cost_index].requirement.energy;
        context.stack().pop<response_return>();
        return pay_action_cost<switch_onpay_offset, switch_action_offset>(
            library, table, context, table.state().active_player, paid_dice, energy
        );
    }

    template<bool Observed>
    inline execution_state broadcast_switch_action(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        end_response<true>(context);

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
#ifndef NDEBUG
            std::uint8_t[],
#endif
            technique_cost_handler_id[], cost_of_technique[], program_entry[], std::size_t[], std::size_t[],
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
            return jump_to_action_instruction<switch_action_finish_offset<Observed>, before_action_with_switch_offset>(context);
        }
        return jump_to_action_instruction<switch_action_finish_offset<Observed>, before_action_offset>(context);
    }

    template<bool Observed>
    inline execution_state execute_switch_action(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        const auto frame = context.stack().top<cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t>();
        const auto* selected = std::get_if<switch_selection>(&get<5>(frame));
        GIVM_ASSERT(selected != nullptr);
        GIVM_ASSERT(selected->switch_cost_index < get<0>(frame).size());
        const auto target = get<0>(frame)[selected->switch_cost_index].target;
        GIVM_ASSERT(static_cast<bool>(table[target]));
        const active_character_changed event{ .current = target };
        const auto settlement_position = context.position()
            + (switch_action_settlement_offset<Observed> - switch_action_offset) * sizeof(execute_fn);
        if constexpr(Observed)
        {
            context.stack().push(event);
            context.enter_next();
            return context.yield(execution_state::active_character_changed);
        }
        apply_active_character_switch(library, table, context, event);
        return context.jump(settlement_position);
    }

    inline execution_state apply_switch_action(const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        const auto event = get<0>(context.stack().top<active_character_changed>());
        context.stack().pop<active_character_changed>();
        apply_active_character_switch(library, table, context, event);
        return context.enter_next();
    }

    template<bool Observed>
    inline execution_state continue_card_onpay(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        auto [action_frame, return_frame] = context.stack().top<frame<
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >, frame<response_return>>();
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
                const auto player = std::visit([&](auto id) { return table[id].player().id(); }, handlers[column]);
                const auto offset = onpay_offsets[index];
                const auto size = onpay_sizes[index];
                auto& stack = context.stack();
                stack.push(response_return{ table.state().self_player, get<0>(return_frame).position + sizeof(execute_fn) });
                const auto capacity = stack.size() + size;
                if(capacity > stack.capacity()) stack.reserve(std::bit_ceil(capacity));
                begin_response<true>(context);
                const auto prepared = context.copy_program_inputs(
                    entry, std::span<const unsigned char>{ stack.data() + offset, size });
                table.state().self_player = player;
                return context.enter(prepared);
            }
        }

        const auto paid_dice = selected->paid_dice;
        const auto& cost = costs[selected->card_cost_index];
        context.stack().pop<response_return>();
        return pay_action_cost<card_onpay_offset<Observed>, prepare_card_play_offset<Observed>>(
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
        execution_context& context, random_fn& random
    )
    {
        if(not continue_single_response<card_effect, hand_card_id>(library, table, context, random)) return continue_execution;
        pop_single_response<card_effect, hand_card_id>(context);
        auto&& [costs, onpay_entries, onpay_offsets, onpay_sizes, switch_handlers, switch_costs,
                switch_onpay_entries, switch_onpay_offsets, switch_onpay_sizes, onpay_cursor, selection, cached_inputs] = context.stack().top<
            cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >();
        const auto* selected = std::get_if<card_selection>(&selection);
        GIVM_ASSERT(selected != nullptr);
        const auto& cost = costs[selected->card_cost_index];
        append_event_record(context, card_played{
            .card = cost.card, .definition_id = table[cost.card].definition_id(),
            .targets = selected->targets, .speed = cost.requirement.speed
        });
        context.stack().push(cost.requirement.speed);
        return context.advance(response_extent<card_effect>);
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
            card_will_be_played, response_return>());
        pop_broadcast<card_will_be_played>(context);
        if(event.speed == action_speed::combat) table[event.card.player_id].state().can_plunge = false;
        context.advance(response_extent<card_will_be_played>);
        // The played card is already out of hand, but retains its definition and state.
        prepare_single_response(card_effect{ .card = event.card, .targets = event.targets },
            event.card, table, context, context.position(), true, not event.effect_cancelled);
        return finish_card_effect(library, table, context, random);
    }

    template<bool Observed>
    inline execution_state broadcast_card_played(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        const auto speed = get<0>(context.stack().top<action_speed>());
        context.stack().pop<action_speed>();
        end_response<true>(context);
        context.stack().pop<
#ifndef NDEBUG
            std::uint8_t[],
#endif
            technique_cost_handler_id[], cost_of_technique[], program_entry[], std::size_t[], std::size_t[],
            skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >();
        if(speed == action_speed::combat)
        {
            return jump_to_action_instruction<card_played_finish_offset<Observed>, before_action_with_switch_offset>(context);
        }
        return jump_to_action_instruction<card_played_finish_offset<Observed>, before_action_offset>(context);
    }

    template<bool Observed>
    inline execution_state continue_skill_onpay(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        auto [action_frame, return_frame] = context.stack().top<frame<
            skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >, frame<response_return>>();
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
                const auto player = std::visit([&](auto id) { return table[id].player().id(); }, handlers[column]);
                const auto offset = onpay_offsets[index];
                const auto size = onpay_sizes[index];
                auto& stack = context.stack();
                stack.push(response_return{ table.state().self_player, get<0>(return_frame).position + sizeof(execute_fn) });
                const auto capacity = stack.size() + size;
                if(capacity > stack.capacity()) stack.reserve(std::bit_ceil(capacity));
                begin_response<true>(context);
                const auto prepared = context.copy_program_inputs(
                    entry, std::span<const unsigned char>{ stack.data() + offset, size });
                table.state().self_player = player;
                return context.enter(prepared);
            }
        }

        const auto paid_dice = selected->paid_dice;
        const auto& cost = costs[selected->skill_cost_index];
        context.stack().pop<response_return>();
        return pay_action_cost<skill_onpay_offset<Observed>, prepare_skill_use_offset<Observed>>(
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
            .skill = cost.skill, .flags = cost.flags, .targets = selected.targets, .speed = cost.requirement.speed
        }, table, context.stack(), context.position() + sizeof(execute_fn));
        return context.enter_next();
    }

    template<bool Observed>
    inline execution_state broadcast_skill_used(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        const auto speed = get<0>(context.stack().top<action_speed>());
        context.stack().pop<action_speed>();
        end_response<true>(context);
        context.stack().pop<
#ifndef NDEBUG
            std::uint8_t[],
#endif
            technique_cost_handler_id[], cost_of_technique[], program_entry[], std::size_t[], std::size_t[],
            skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >();
        if(speed == action_speed::combat)
        {
            return jump_to_action_instruction<skill_used_finish_offset<Observed>, before_action_with_switch_offset>(context);
        }
        return jump_to_action_instruction<skill_used_finish_offset<Observed>, before_action_offset>(context);
    }

    template<bool Observed>
    inline execution_state continue_technique_onpay(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        auto [action_frame, return_frame] = context.stack().top<frame<
            technique_cost_handler_id[], cost_of_technique[], program_entry[], std::size_t[], std::size_t[],
            skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >, frame<response_return>>();
        auto&& [handlers, costs, onpay_entries, onpay_offsets, onpay_sizes,
                skill_handlers, skill_costs, skill_onpay_entries, skill_onpay_offsets, skill_onpay_sizes,
                card_handlers, card_costs, card_onpay_entries, card_onpay_offsets, card_onpay_sizes,
                switch_handlers, switch_costs, switch_onpay_entries, switch_onpay_offsets, switch_onpay_sizes,
                onpay_cursor, selection, cached_inputs] = action_frame;
        const auto* selected = std::get_if<technique_selection>(&selection);
        GIVM_ASSERT(selected != nullptr);
        const auto handler_count = static_cast<stack_count_t>(handlers.size());
        while(onpay_cursor < handler_count)
        {
            const auto column = onpay_cursor++;
            const auto index = column;
            const auto entry = onpay_entries[index];
            if(entry)
            {
                const auto player = std::visit([&](auto id) { return table[id].player().id(); }, handlers[column]);
                const auto offset = onpay_offsets[index];
                const auto size = onpay_sizes[index];
                auto& stack = context.stack();
                stack.push(response_return{ table.state().self_player, get<0>(return_frame).position + sizeof(execute_fn) });
                const auto capacity = stack.size() + size;
                if(capacity > stack.capacity()) stack.reserve(std::bit_ceil(capacity));
                begin_response<true>(context);
                const auto prepared = context.copy_program_inputs(
                    entry, std::span<const unsigned char>{ stack.data() + offset, size });
                table.state().self_player = player;
                return context.enter(prepared);
            }
        }

        const auto paid_dice = selected->paid_dice;
        const auto& cost = costs[0];
        context.stack().pop<response_return>();
        return pay_action_cost<technique_onpay_offset<Observed>, prepare_technique_use_offset<Observed>>(
            library, table, context, cost.technique.character_id.player_id, paid_dice, cost.requirement.energy
        );
    }

    template<bool Observed>
    inline execution_state broadcast_technique_used(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        const auto speed = get<0>(context.stack().top<action_speed>());
        context.stack().pop<action_speed>();
        end_response<true>(context);
        context.stack().pop<
#ifndef NDEBUG
            std::uint8_t[],
#endif
            technique_cost_handler_id[], cost_of_technique[], program_entry[], std::size_t[], std::size_t[],
            skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >();
        if(speed == action_speed::combat)
        {
            return jump_to_action_instruction<technique_used_finish_offset<Observed>, before_action_with_switch_offset>(context);
        }
        return jump_to_action_instruction<technique_used_finish_offset<Observed>, before_action_offset>(context);
    }

    template<bool Observed>
    inline execution_state finish_technique_effect(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_single_response<technique_effect, attachment_id>(library, table, context, random)) return continue_execution;
        pop_single_response<technique_effect, attachment_id>(context);
        const auto event = get<0>(context.stack().top<technique_used>());
        context.stack().pop<technique_used>();
        append_event_record(context, event);
        context.stack().push(event.speed);
        return context.advance(response_extent<technique_effect>);
    }

    template<bool Observed>
    inline execution_state broadcast_technique_will_be_used(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        if(not continue_broadcast<technique_will_be_used>(library, table, context, random))
        {
            return continue_execution;
        }
        const auto event = get<0>(context.stack().top<technique_will_be_used, response_return>());
        pop_broadcast<technique_will_be_used>(context);
        if(event.speed == action_speed::combat) table[event.technique.character_id.player_id].state().can_plunge = false;
        context.advance(response_extent<technique_will_be_used>);
        context.stack().push(technique_used{
            .technique = event.technique, .targets = event.targets, .speed = event.speed,
            .effect_cancelled = event.effect_cancelled
        });
        prepare_single_response(technique_effect{ .technique = event.technique, .targets = event.targets },
            event.technique, table, context, context.position(), false, not event.effect_cancelled);
        return finish_technique_effect<Observed>(library, table, context, random);
    }

    template<bool Observed>
    inline execution_state prepare_technique_use(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        auto&& [costs, onpay_entries, onpay_offsets, onpay_sizes, skill_handlers, skill_costs, skill_onpay_entries, skill_onpay_offsets, skill_onpay_sizes, card_handlers, card_costs, card_onpay_entries, card_onpay_offsets, card_onpay_sizes,
                switch_handlers, switch_costs, switch_onpay_entries, switch_onpay_offsets, switch_onpay_sizes, onpay_cursor, selection, cached_inputs] = context.stack().top<
            cost_of_technique[], program_entry[], std::size_t[], std::size_t[],
            skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >();
        const auto& selected = std::get<technique_selection>(selection);
        const auto& cost = costs[0];
        prepare_broadcast(library, technique_will_be_used{
            .technique = cost.technique, .targets = selected.targets, .speed = cost.requirement.speed
        }, table, context.stack(), context.position() + sizeof(execute_fn));
        context.enter_next();
        return broadcast_technique_will_be_used<Observed>(library, table, context, random);
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
        const auto event = get<0>(context.stack().top<elemental_tuning_modification, response_return>());
        pop_broadcast<elemental_tuning_modification>(context);
        table[event.card].erase();
        auto& dice = table[event.card.player_id].state().dice;
        --dice[event.from];
        ++dice[event.to];
        append_event_record(context, elemental_tuning_completed{
            .card = event.card, .from = event.from, .to = event.to
        });
        return context.advance(response_extent<elemental_tuning_modification>);
    }

    template<bool Observed>
    inline execution_state broadcast_elemental_tuning_completed(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        end_response<true>(context);
        context.stack().pop<
#ifndef NDEBUG
            std::uint8_t[],
#endif
            technique_cost_handler_id[], cost_of_technique[], program_entry[], std::size_t[], std::size_t[],
            skill_cost_handler_id[], cost_of_skill[], program_entry[], std::size_t[], std::size_t[],
            card_cost_handler_id[], cost_of_card[], program_entry[], std::size_t[], std::size_t[],
            switch_handler_id[], cost_of_switch[], program_entry[], std::size_t[], std::size_t[],
            stack_count_t, action_selection, substack_t
        >();
        return jump_to_action_instruction<elemental_tuning_completed_finish_offset<Observed>, before_action_offset>(context);
    }

    template<bool Observed>
    inline execution_state broadcast_first_round_end(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random
    )
    {
        end_response<true>(context);
        table.state().active_player = other_player(table.state().active_player);
        jump_to_action_instruction<first_round_end_finish_offset<Observed>, before_action_offset>(context);
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
        end_response<true>(context);
        return context.enter_next();
    }

    template<bool Observed>
    inline void compile_begin_action(program_writer& writer)
    {
        writer.write<execute_fn>(&prepare_action_phase);
        compile_settlement(writer, begin_settlement);
        writer.write(execute_fn{ broadcast_action_phase<Observed> });
        writer.write<execute_fn>(&prepare_before_action<false, Observed>);
        writer.write<execute_fn>(&prepare_before_action<true, Observed>);
        compile_settlement(writer, begin_settlement);
        writer.write(execute_fn{ broadcast_before_action<Observed> });
        writer.write<execute_fn>(&execute_action_selection<Observed>);
        compile_onpay_response(writer, continue_switch_onpay);
        writer.write<execute_fn>(&execute_switch_action<Observed>);
        if constexpr(Observed)
        {
            writer.write<execute_fn>(&apply_switch_action);
        }
        compile_settlement(writer, begin_settlement);
        writer.write(execute_fn{ broadcast_switch_action<Observed> });
        compile_onpay_response(writer, continue_card_onpay<Observed>);
        writer.write<execute_fn>(&prepare_card_play);
        compile_broadcast<card_will_be_played>(writer, broadcast_card_will_be_played);
        compile_single_response<card_effect, hand_card_id>(writer, finish_card_effect);
        compile_settlement(writer, begin_settlement);
        writer.write(execute_fn{ broadcast_card_played<Observed> });
        compile_onpay_response(writer, continue_skill_onpay<Observed>);
        writer.write<execute_fn>(&prepare_skill_use);
        compile_broadcast<skill_will_be_used>(writer, broadcast_skill_will_be_used<true>);
        compile_single_response<skill_effect, skill_id>(writer, finish_skill_effect<true>);
        compile_settlement(writer, begin_settlement);
        writer.write(execute_fn{ broadcast_skill_used<Observed> });
        compile_onpay_response(writer, continue_technique_onpay<Observed>);
        writer.write<execute_fn>(&prepare_technique_use<Observed>);
        compile_broadcast<technique_will_be_used>(writer, broadcast_technique_will_be_used<Observed>);
        compile_single_response<technique_effect, attachment_id>(writer, finish_technique_effect<Observed>);
        compile_settlement(writer, begin_settlement);
        writer.write(execute_fn{ broadcast_technique_used<Observed> });
        compile_broadcast<elemental_tuning_modification>(writer, broadcast_elemental_tuning_modification);
        compile_settlement(writer, begin_settlement);
        writer.write(execute_fn{ broadcast_elemental_tuning_completed<Observed> });
        compile_settlement(writer, begin_settlement);
        writer.write(execute_fn{ broadcast_first_round_end<Observed> });
        compile_settlement(writer, begin_settlement);
        writer.write(execute_fn{ continue_prepared_skill_removal<Observed> });
        compile_single_response<prepared_skill_effect, attachment_id>(writer, finish_prepared_skill_effect);
        compile_settlement(writer, begin_settlement);
        writer.write(execute_fn{ finish_prepared_skill_action<Observed> });
        compile_settlement(writer, begin_settlement);
        writer.write(execute_fn{ broadcast_second_round_end });
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

namespace givm::detail
{
    inline std::vector<begin_action::error_type> check(const begin_action&, const definition_compile_context&, program_kind)
    {
        return {};
    }
}

#include <givm/macro_undef.hpp>
#endif
