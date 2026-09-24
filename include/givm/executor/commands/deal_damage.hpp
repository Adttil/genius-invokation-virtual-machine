#ifndef GIVM_EXECUTOR_COMMANDS_DEAL_DAMAGE_HPP
#define GIVM_EXECUTOR_COMMANDS_DEAL_DAMAGE_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>
#include <utility>
#include <variant>

#include "../broadcast.hpp"
#include "../character_target.hpp"
#include "apply_element.hpp"
#include "../../definition.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline constexpr std::uint8_t no_overloaded_player = 2;

    // Each child frame has exactly this stride; resolution walks it linearly.
    struct alignas(max_alignment) damage_record
    {
        damage_calculation event{};
        bool applied = false;
    };

    struct damage_target_cursor
    {
        std::size_t next = 0;
        std::size_t remaining = 0;
        std::size_t count = 0;
    };

    struct damage_group
    {
        damage_preparation input{};
        damage_target_cursor targets;
        damage_target_cursor swirl_targets;
        std::size_t swirl_parent = 0;
        std::size_t input_count = 0;
        std::size_t input_cursor = 0;
        std::size_t count = 0;
        std::size_t cursor = 0;
        execution_position instructions = 0;
        character_selection selection = character_selection::character;
        std::uint8_t overloaded_player = no_overloaded_player;
    };

    inline constexpr std::size_t damage_preparation_offset = 0;
    inline constexpr std::size_t damage_reaction_offset = 1;
    inline constexpr std::size_t damage_calculation_offset = 2;
    inline constexpr std::size_t damage_effect_offset = 3;
    inline constexpr std::size_t damage_dying_offset = 4;
    inline constexpr std::size_t damage_health_resume_offset = 5;
    template<bool Observed>
    inline constexpr std::size_t damage_entity_resume_offset = 5 + Observed;
    template<bool Observed>
    inline constexpr std::size_t damage_attachment_resume_offset = 6 + Observed;
    template<bool Observed>
    inline constexpr std::size_t damage_overloaded_removal_offset = 7 + Observed;
    template<bool Observed>
    inline constexpr std::size_t damage_overloaded_observation_offset = 8 + Observed;
    template<bool Observed>
    inline constexpr std::size_t damage_overloaded_broadcast_offset = 8 + 2 * Observed;
    template<bool Observed>
    inline constexpr std::size_t damage_after_reaction_offset = 9 + 2 * Observed;
    template<bool Observed>
    inline constexpr std::size_t damage_after_damage_offset = 10 + 2 * Observed;
    template<bool Observed>
    inline constexpr std::size_t damage_end_offset = 11 + 2 * Observed;

    inline damage_record& damage_record_at(damage_group& group, std::size_t index) noexcept
    {
        return *std::launder(reinterpret_cast<damage_record*>(
            reinterpret_cast<unsigned char*>(&group) + sizeof(group) + index * sizeof(damage_record)));
    }

    inline damage_target_cursor damage_targets(
        const unrestricted_table& table, character_id anchor, character_selection selection)
    {
        const auto count = table[anchor.player_id].template characters<false>().size();
        auto next = anchor.index;
        if(selection == character_selection::others && ++next == count) next = 0;
        return { next, selection == character_selection::character ? 1
            : count - (selection == character_selection::others), count };
    }

    inline std::optional<character_id> next_damage_target(
        const unrestricted_table& table, player_id player, damage_target_cursor& cursor)
    {
        while(cursor.remaining != 0)
        {
            const character_id id{ player, cursor.next };
            --cursor.remaining;
            if(++cursor.next == cursor.count) cursor.next = 0;
            const auto target = table[id];
            if(target && target.state().health != 0) return id;
        }
        return std::nullopt;
    }

    inline void append_damage(frame_stack& stack, const damage_calculation& event)
    {
        const auto frame = stack.top<damage_group, substack_t>();
        // Copy before push: event may belong to the same relocating stack.
        const damage_record record{ event };
        get<1>(frame).push(record);
        ++get<0>(frame).count;
    }

    constexpr void add_reaction_damage_bonus(damage_calculation& event, std::uint32_t value) noexcept
    {
        const auto max_value = std::numeric_limits<std::uint32_t>::max();
        event.value = max_value - event.value < value ? max_value : event.value + value;
    }

    constexpr void apply_default_damage_reaction_effect(damage_calculation& event, elemental_reaction reaction) noexcept
    {
        switch(reaction)
        {
        case elemental_reaction::melt:
        case elemental_reaction::vaporize:
        case elemental_reaction::overloaded:
            add_reaction_damage_bonus(event, 2);
            break;
        case elemental_reaction::superconduct:
        case elemental_reaction::electro_charged:
        case elemental_reaction::quicken:
        case elemental_reaction::burning:
        case elemental_reaction::bloom:
        case elemental_reaction::frozen:
            add_reaction_damage_bonus(event, 1);
            break;
        default:
            // Other default reaction effects are filled separately.
            break;
        }
    }

    constexpr std::uint32_t calculate_damage_value(std::uint32_t value, std::uint16_t numerator, std::uint16_t denominator) noexcept
    {
        GIVM_ASSERT(denominator != 0);
        const auto result = static_cast<std::uint64_t>(value) * numerator / denominator;
        return result > std::numeric_limits<std::uint32_t>::max()
            ? std::numeric_limits<std::uint32_t>::max() : static_cast<std::uint32_t>(result);
    }

    constexpr damage_type damage_type_from_element(element value) noexcept
    {
        switch(value)
        {
        case element::cryo: return damage_type::cryo;
        case element::hydro: return damage_type::hydro;
        case element::pyro: return damage_type::pyro;
        case element::electro: return damage_type::electro;
        case element::anemo: return damage_type::anemo;
        case element::geo: return damage_type::geo;
        case element::dendro: return damage_type::dendro;
        case element::none: return damage_type::physical;
        }
        std::unreachable();
    }

    inline bool all_characters_defeated(const unrestricted_table& table, player_id player)
    {
        for(const auto character : table[player].characters())
            if(character.state().health != 0) return false;
        return true;
    }

    // Empty means the caller can keep running without another opcode dispatch.
    template<bool Observed>
    inline std::optional<execution_state> finish_group_damage(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        const auto event = damage_record_at(group, group.cursor++).event;
        if(not event.replacement_reaction)
            return prepare_default_reaction_entities(library, table, context, random, event.reaction, event.target,
                group.instructions + damage_entity_resume_offset<Observed> * sizeof(execute_fn),
                group.instructions + damage_attachment_resume_offset<Observed> * sizeof(execute_fn));
        return std::nullopt;
    }

    template<bool Observed>
    inline std::optional<execution_state> continue_damage_dying(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<character_will_be_defeated>(library, table, context, random)) return continue_execution;
        const auto id = get<0>(context.stack().top<character_will_be_defeated, response_return>()).target;
        pop_broadcast<character_will_be_defeated>(context);
        const auto character = table[id];
        if(character.state().health == 0)
        {
            if(all_characters_defeated(table, id.player_id))
            {
                const auto result = all_characters_defeated(table, other_player(id.player_id)) ? game_result::both_loss
                    : id.player_id == player_id{ 0 } ? game_result::player_1_win : game_result::player_0_win;
                return context.end_game(result);
            }
            for(const auto attachment : character.attachments()) attachment.erase();
            character.state().energy = 0;
            character.state().aura = element_aura::none;
        }
        return finish_group_damage<Observed>(library, table, context, random);
    }

    template<bool Observed>
    inline std::optional<execution_state> continue_damage_after_health_reduction(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        const auto id = damage_record_at(group, group.cursor).event.target;
        if(table[id].state().health != 0)
            return finish_group_damage<Observed>(library, table, context, random);
        const auto position = group.instructions + damage_dying_offset * sizeof(execute_fn);
        prepare_broadcast(library, character_will_be_defeated{ .target = id }, table, context.stack(), position);
        return continue_damage_dying<Observed>(library, table, context, random);
    }

    template<bool Observed>
    inline std::optional<execution_state> continue_damage_effect(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<damage_effect>(library, table, context, random)) return continue_execution;
        const auto event = get<0>(context.stack().top<damage_effect, response_return>());
        pop_broadcast<damage_effect>(context);
        auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        if(table[event.target].state().health == 0)
        {
            ++group.cursor;
            return std::nullopt;
        }
        auto& record = damage_record_at(group, group.cursor);
        record.event.value = event.value;
        record.applied = true;
        auto& state = table[event.target].state();
        state.health = event.value >= state.health ? 0 : state.health - event.value;
        if constexpr(Observed)
        {
            if(event.value != 0)
            {
                const auto position = group.instructions + damage_health_resume_offset * sizeof(execute_fn);
                context.stack().push(after_damage{
                    event.source, event.target, event.value, event.type, event.flags, event.reaction, event.replacement_reaction });
                context.jump(position);
                return context.yield(execution_state::health_reduced);
            }
        }
        return continue_damage_after_health_reduction<Observed>(library, table, context, random);
    }

    template<bool Observed>
    inline std::optional<execution_state> continue_damage_calculation(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<damage_calculation>(library, table, context, random)) return continue_execution;
        auto event = get<0>(context.stack().top<damage_calculation, response_return>());
        pop_broadcast<damage_calculation>(context);
        auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        if(table[event.target].state().health == 0)
        {
            ++group.cursor;
            return std::nullopt;
        }
        auto& record = damage_record_at(group, group.cursor);
        if(not event.replacement_reaction) apply_default_damage_reaction_effect(event, event.reaction);
        event.value = calculate_damage_value(event.value, event.multiplier_numerator, event.multiplier_denominator);
        std::construct_at(&record.event, event);
        const auto position = group.instructions + damage_effect_offset * sizeof(execute_fn);
        prepare_broadcast(library, damage_effect{
            .source = event.source, .target = event.target, .value = event.value,
            .type = event.type, .flags = event.flags, .reaction = event.reaction, .replacement_reaction = event.replacement_reaction
        }, table, context.stack(), position);
        return continue_damage_effect<Observed>(library, table, context, random);
    }

    inline void finish_damage_preparation(unrestricted_table& table, execution_context& context)
    {
        const auto frame = context.stack().top<damage_group, substack_t>();
        auto& group = get<0>(frame);
        const auto parent = group.count - 1;
        const auto event = damage_record_at(group, parent).event;
        const auto incoming = element_from_damage_type(event.type);
        // A nested response can defeat the target while choosing a replacement.
        // Its cleared aura must not be restored by this already prepared hit.
        if(incoming != element::none && table[event.target].state().health != 0)
            table[event.target].state().aura = event.reaction == elemental_reaction::none
                ? aura_without_reaction(event.reacted_aura, incoming)
                : aura_after_reaction(event.reacted_aura, incoming, event.reaction);
        if(event.replacement_reaction) return;
        if(event.reaction == elemental_reaction::swirl)
        {
            GIVM_ASSERT(incoming == element::anemo);
            group.swirl_parent = parent;
            group.swirl_targets = damage_targets(table, event.target, character_selection::others);
        }
        else if(event.reaction == elemental_reaction::superconduct || event.reaction == elemental_reaction::electro_charged)
        {
            // Piercing children have neither attribute preparation nor a reaction;
            // the complete range can be appended here without a resumable cursor.
            auto targets = damage_targets(table, event.target, character_selection::others);
            while(const auto target = next_damage_target(table, event.target.player_id, targets))
                append_damage(context.stack(), damage_calculation{
                    .source = event.source, .target = *target, .value = 1, .type = damage_type::piercing,
                    .flags = damage_flag_bits::reaction_damage, .reacted_aura = table[*target].state().aura });
        }
        else if(event.reaction == elemental_reaction::overloaded
            && group.overloaded_player == no_overloaded_player
            && table[event.target.player_id].state().active_character == event.target)
            group.overloaded_player = static_cast<std::uint8_t>(event.target.player_id.index);
    }

    inline std::optional<execution_state> continue_damage_reaction(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<elemental_reaction_will_occur>(library, table, context, random)) return continue_execution;
        const auto event = get<0>(context.stack().top<elemental_reaction_will_occur, response_return>());
        pop_broadcast<elemental_reaction_will_occur>(context);
        auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        auto& record = damage_record_at(group, group.count - 1);
        const auto original = record.event;
        std::construct_at(&record.event, damage_calculation{
            .source = original.source, .target = original.target, .value = original.value,
            .multiplier_numerator = original.multiplier_numerator,
            .multiplier_denominator = original.multiplier_denominator,
            .type = original.type, .flags = original.flags, .reaction = original.reaction,
            .reacted_aura = original.reacted_aura, .replacement_reaction = event.replacement_reaction });
        finish_damage_preparation(table, context);
        return std::nullopt;
    }

    inline std::optional<execution_state> prepare_damage_reaction(
        const definition_library& library, unrestricted_table& table, execution_context& context,
        random_fn& random, const damage_preparation& input, character_id target)
    {
        const auto aura = table[target].state().aura;
        const auto incoming = element_from_damage_type(input.type);
        const damage_calculation event{
            .source = input.source, .target = target, .value = input.value,
            .multiplier_numerator = input.multiplier_numerator,
            .multiplier_denominator = input.multiplier_denominator,
            .type = input.type, .flags = input.flags,
            .reaction = reaction_from_aura(aura, incoming), .reacted_aura = aura };
        append_damage(context.stack(), event);
        if(event.reaction == elemental_reaction::none)
        {
            finish_damage_preparation(table, context);
            return std::nullopt;
        }
        const auto position = get<0>(context.stack().top<damage_group, substack_t>()).instructions
            + damage_reaction_offset * sizeof(execute_fn);
        prepare_broadcast(library, elemental_reaction_will_occur{
            .source = event.source, .target = target, .incoming_element = incoming, .reacted_aura = aura,
            .reaction = event.reaction, .cause = element_application_cause::damage
        }, table, context.stack(), position);
        return continue_damage_reaction(library, table, context, random);
    }

    inline std::optional<execution_state> continue_damage_preparation(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<damage_preparation>(library, table, context, random)) return continue_execution;
        const auto event = get<0>(context.stack().top<damage_preparation, response_return>());
        pop_broadcast<damage_preparation>(context);
        auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        std::construct_at(&group.input, event);
        group.targets = damage_targets(table, event.target, group.selection);
        return std::nullopt;
    }

    inline std::optional<execution_state> continue_damage_after_damage(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<after_damage>(library, table, context, random)) return continue_execution;
        pop_broadcast<after_damage>(context);
        auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        ++group.cursor;
        return std::nullopt;
    }

    template<bool Observed>
    inline void prepare_group_after_damage(
        const definition_library& library, const unrestricted_table& table, execution_context& context)
    {
        auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        const auto event = damage_record_at(group, group.cursor).event;
        const auto position = group.instructions + damage_after_damage_offset<Observed> * sizeof(execute_fn);
        prepare_broadcast(library, after_damage{
            event.source, event.target, event.value, event.type, event.flags, event.reaction, event.replacement_reaction },
            table, context.stack(), position);
    }

    template<bool Observed>
    inline std::optional<execution_state> continue_damage_after_reaction(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<after_elemental_reaction>(library, table, context, random)) return continue_execution;
        pop_broadcast<after_elemental_reaction>(context);
        prepare_group_after_damage<Observed>(library, table, context);
        return continue_damage_after_damage(library, table, context, random);
    }

    template<bool Inputs, bool Observed>
    inline execution_state continue_damage_completion(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto frame = context.stack().top<damage_group, substack_t>();
        while(get<0>(frame).cursor != get<0>(frame).count)
        {
            auto& group = get<0>(frame);
            const auto node = damage_record_at(group, group.cursor);
            if(not node.applied)
            {
                ++group.cursor;
                continue;
            }
            if(node.event.reaction != elemental_reaction::none)
            {
                const auto position = group.instructions + damage_after_reaction_offset<Observed> * sizeof(execute_fn);
                prepare_broadcast(library, after_elemental_reaction{
                    .source = node.event.source, .target = node.event.target,
                    .incoming_element = element_from_damage_type(node.event.type), .reacted_aura = node.event.reacted_aura,
                    .reaction = node.event.reaction, .cause = element_application_cause::damage, .replacement_reaction = node.event.replacement_reaction
                }, table, context.stack(), position);
                if(const auto state = continue_damage_after_reaction<Observed>(library, table, context, random)) return *state;
            }
            else
            {
                prepare_group_after_damage<Observed>(library, table, context);
                if(const auto state = continue_damage_after_damage(library, table, context, random)) return *state;
            }
        }
        const auto position = get<0>(frame).instructions + damage_end_offset<Observed> * sizeof(execute_fn);
        const auto input_count = get<0>(frame).input_count;
        context.stack().pop<damage_group, substack_t>();
        if constexpr(Inputs)
            for(std::size_t index = 0; index != input_count; ++index) context.stack().pop<damage>();
        return context.jump(position);
    }

    template<bool Inputs, bool Observed>
    inline execution_state continue_damage_overloaded_switch(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(const auto state = continue_reaction_overloaded_switch(library, table, context, random)) return *state;
        return continue_damage_completion<Inputs, Observed>(library, table, context, random);
    }

    template<bool Inputs, bool Observed>
    inline execution_state resume_damage_overloaded_removal(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(const auto state = continue_switch_prepared_removal(library, table, context, random)) return *state;
        return continue_damage_overloaded_switch<Inputs, Observed>(library, table, context, random);
    }

    template<bool Inputs>
    inline execution_state resume_damage_overloaded_observation(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(const auto state = resume_reaction_overloaded_observation(library, table, context, random)) return *state;
        return continue_damage_completion<Inputs, true>(library, table, context, random);
    }

    template<bool Inputs, bool Observed>
    inline execution_state finish_damage_group(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        group.cursor = 0;
        if(group.overloaded_player == no_overloaded_player)
            return continue_damage_completion<Inputs, Observed>(library, table, context, random);
        const auto player = player_id{ group.overloaded_player };
        group.overloaded_player = no_overloaded_player;
        if(const auto state = prepare_reaction_overloaded_switch<Observed>(library, table, context, random, player,
            group.instructions + damage_overloaded_removal_offset<Observed> * sizeof(execute_fn),
            group.instructions + damage_overloaded_observation_offset<Observed> * sizeof(execute_fn),
            group.instructions + damage_overloaded_broadcast_offset<Observed> * sizeof(execute_fn))) return *state;
        return continue_damage_completion<Inputs, Observed>(library, table, context, random);
    }

    template<bool Inputs, bool Observed>
    inline execution_state continue_damage_group(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto frame = context.stack().top<damage_group, substack_t>();
        while(get<0>(frame).cursor != get<0>(frame).count)
        {
            auto& group = get<0>(frame);
            const auto event = damage_record_at(group, group.cursor).event;
            const auto target = table[event.target];
            // A previous hit or nested response may have defeated this target.
            // The other prepared records, including its reaction children, remain.
            if(not target || target.state().health == 0)
            {
                ++group.cursor;
                continue;
            }
            const auto position = group.instructions + damage_calculation_offset * sizeof(execute_fn);
            prepare_broadcast(library, event, table, context.stack(), position);
            if(const auto state = continue_damage_calculation<Observed>(library, table, context, random)) return *state;
        }
        return finish_damage_group<Inputs, Observed>(library, table, context, random);
    }

    template<bool Inputs, bool Observed>
    inline execution_state continue_damage_group_preparation(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto frame = context.stack().top<damage_group, substack_t>();
        while(true)
        {
            auto& group = get<0>(frame);
            if(group.swirl_targets.remaining != 0)
            {
                const auto parent = damage_record_at(group, group.swirl_parent).event;
                if(const auto target = next_damage_target(table, parent.target.player_id, group.swirl_targets))
                {
                    const damage_preparation input{
                        .source = parent.source, .target = *target, .value = 1,
                        .type = damage_type_from_element(primary_element_from_aura(parent.reacted_aura)),
                        .flags = damage_flag_bits::reaction_damage };
                    if(const auto state = prepare_damage_reaction(library, table, context, random, input, *target)) return *state;
                }
                continue;
            }
            if(group.targets.remaining != 0)
            {
                if(const auto target = next_damage_target(table, group.input.target.player_id, group.targets))
                {
                    // Range copies inherit the original description's prepared
                    // properties, but each target has its own aura and reaction.
                    const auto input = group.input;
                    if(const auto state = prepare_damage_reaction(library, table, context, random, input, *target)) return *state;
                }
                continue;
            }
            if(group.input_cursor == group.input_count)
                return continue_damage_group<Inputs, Observed>(library, table, context, random);

            const auto index = group.input_cursor++;
            std::conditional_t<Inputs, damage, fixed_damage> input;
            damage_source_id source;
            std::optional<character_id> target;
            if constexpr(Inputs)
            {
                constexpr auto stride = align(sizeof(damage), max_alignment);
                constexpr auto padding = align(sizeof(damage_group), max_alignment) - sizeof(damage_group);
                const auto input_end = reinterpret_cast<const unsigned char*>(&group) - padding;
                std::memcpy(&input, input_end - index * stride - sizeof(damage), sizeof(damage));
                source = input.source;
                if(const auto* id = std::get_if<character_id>(&input.target))
                {
                    target = *id;
                    group.selection = input.selection;
                }
                else
                {
                    const auto& relative = std::get<relative_character_target>(input.target);
                    target = resolve_character_target<true>(table, relative);
                    group.selection = relative.selection;
                }
            }
            else
            {
                const auto bytes = &context.instruction_data<0, unsigned char>(library) - context.position();
                std::memcpy(&input, bytes + group.instructions
                    - (group.input_count - index) * padded_size<fixed_damage>, sizeof(fixed_damage));
                const auto id = resolve_character_target<false>(table, input.source);
                if(not id) continue;
                source = *id;
                group.selection = input.target.selection;
                target = resolve_character_target<true>(table, input.target);
            }
            if(not target) continue;
            const auto character = table[*target];
            if(not character || character.state().health == 0) continue;
            const auto position = group.instructions + damage_preparation_offset * sizeof(execute_fn);
            prepare_broadcast(library, damage_preparation{
                .source = source, .target = *target, .value = input.value,
                .multiplier_numerator = input.multiplier_numerator,
                .multiplier_denominator = input.multiplier_denominator,
                .type = input.type, .flags = input.flags
            }, table, context.stack(), position);
            if(const auto state = continue_damage_preparation(library, table, context, random)) return *state;
        }
    }

    template<bool Inputs, bool Observed, auto Continue>
    inline execution_state resume_damage_group_preparation(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(const auto state = Continue(library, table, context, random)) return *state;
        return continue_damage_group_preparation<Inputs, Observed>(library, table, context, random);
    }

    template<bool Inputs, bool Observed, auto Continue>
    inline execution_state resume_damage_group(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(const auto state = Continue(library, table, context, random)) return *state;
        return continue_damage_group<Inputs, Observed>(library, table, context, random);
    }

    template<bool Inputs, bool Observed, auto Continue>
    inline execution_state resume_damage_completion(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(const auto state = Continue(library, table, context, random)) return *state;
        return continue_damage_completion<Inputs, Observed>(library, table, context, random);
    }

    template<bool Inputs>
    inline execution_state resume_damage_health_observation(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        context.stack().pop<after_damage>();
        if(const auto state = continue_damage_after_health_reduction<true>(library, table, context, random)) return *state;
        return continue_damage_group<Inputs, true>(library, table, context, random);
    }

    template<bool Inputs, bool Observed>
    inline execution_state resume_damage_entity_generation(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        context.stack().pop<response_return>();
        return continue_damage_group<Inputs, Observed>(library, table, context, random);
    }

    template<bool Inputs, bool Observed>
    inline execution_state prepare_damage_group(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto count = context.instruction_data<1, std::size_t>(library);
        const auto position = context.position() + instruction_extent<1, std::size_t>
            + (Inputs ? 0 : count * padded_size<fixed_damage>);
        context.stack().push(damage_group{ .input_count = count, .instructions = position }, substack());
        return continue_damage_group_preparation<Inputs, Observed>(library, table, context, random);
    }

    template<bool Inputs, bool Observed>
    inline void compile_damage_resolution(program_writer& writer)
    {
        writer.write(execute_fn{ resume_damage_group_preparation<Inputs, Observed, continue_damage_preparation> });
        writer.write(execute_fn{ resume_damage_group_preparation<Inputs, Observed, continue_damage_reaction> });
        writer.write(execute_fn{ resume_damage_group<Inputs, Observed, continue_damage_calculation<Observed>> });
        writer.write(execute_fn{ resume_damage_group<Inputs, Observed, continue_damage_effect<Observed>> });
        writer.write(execute_fn{ resume_damage_group<Inputs, Observed, continue_damage_dying<Observed>> });
        if constexpr(Observed) writer.write(execute_fn{ resume_damage_health_observation<Inputs> });
        writer.write(execute_fn{ resume_damage_entity_generation<Inputs, Observed> });
        writer.write(execute_fn{ resume_damage_group<Inputs, Observed, continue_attachment_replacement> });
        writer.write(execute_fn{ resume_damage_overloaded_removal<Inputs, Observed> });
        if constexpr(Observed) writer.write(execute_fn{ resume_damage_overloaded_observation<Inputs> });
        writer.write(execute_fn{ continue_damage_overloaded_switch<Inputs, Observed> });
        writer.write(execute_fn{ resume_damage_completion<Inputs, Observed, continue_damage_after_reaction<Observed>> });
        writer.write(execute_fn{ resume_damage_completion<Inputs, Observed, continue_damage_after_damage> });
    }

    template<bool Inputs, bool Observed>
    inline void compile_damage_group(program_writer& writer, const givm::deal_damage& command)
    {
        writer.write(execute_fn{ prepare_damage_group<Inputs, Observed> });
        if constexpr(Inputs)
        {
            GIVM_ASSERT(command.input_count != 0);
            writer.write(command.input_count);
        }
        else
        {
            writer.write(command.damages.size());
            for(const auto& input : command.damages) writer.write(input);
        }
        compile_damage_resolution<Inputs, Observed>(writer);
    }

    inline void compile(program_writer& writer, const givm::deal_damage& command, compile_mode mode)
    {
        if(command.damages.empty())
        {
            if(mode == compile_mode::observed) compile_damage_group<true, true>(writer, command);
            else compile_damage_group<true, false>(writer, command);
        }
        else
        {
            if(mode == compile_mode::observed) compile_damage_group<false, true>(writer, command);
            else compile_damage_group<false, false>(writer, command);
        }
    }
}

#include "../../macro_undef.hpp"
#endif
