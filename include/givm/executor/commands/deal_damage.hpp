#ifndef GIVM_EXECUTOR_COMMANDS_DEAL_DAMAGE_HPP
#define GIVM_EXECUTOR_COMMANDS_DEAL_DAMAGE_HPP

#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>
#include <utility>
#include <variant>

#include "../broadcast.hpp"
#include "attach.hpp"
#include "generate_combat_status.hpp"
#include "summon.hpp"
#include "../../definition.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    inline constexpr std::size_t no_damage_node = std::numeric_limits<std::size_t>::max();
    inline constexpr std::uint8_t no_overloaded_player = 2;

    // A pending node becomes a completion record; its link is reused. All
    // references between nodes are offsets, so growing/copying the stack is safe.
    struct damage_node
    {
        damage_calculation event{};
        std::size_t next = no_damage_node;
        element_application_cause cause = element_application_cause::damage;
        bool deals_damage = true;
    };

    struct damage_group
    {
        damage_node first;
        std::size_t pending = no_damage_node;
        std::size_t completed = no_damage_node;
        std::size_t last_completed = no_damage_node;
        std::size_t input_count = 0;
        std::size_t input_cursor = 0;
        execution_position instructions = 0;
        std::uint8_t overloaded_player = no_overloaded_player;
        bool first_used = false;
    };

    inline constexpr std::size_t damage_preparation_offset = 0;
    inline constexpr std::size_t damage_reaction_offset = 1;
    inline constexpr std::size_t damage_calculation_offset = 2;
    inline constexpr std::size_t damage_effect_offset = 3;
    inline constexpr std::size_t damage_health_resume_offset = 4;
    template<bool Observed>
    inline constexpr std::size_t damage_entity_resume_offset = 4 + Observed;
    template<bool Observed>
    inline constexpr std::size_t damage_attachment_resume_offset = 5 + Observed;
    template<bool Observed>
    inline constexpr std::size_t damage_overloaded_observation_offset = 6 + Observed;
    template<bool Observed>
    inline constexpr std::size_t damage_overloaded_broadcast_offset = 6 + 2 * Observed;
    template<bool Observed>
    inline constexpr std::size_t damage_after_reaction_offset = 7 + 2 * Observed;
    template<bool Observed>
    inline constexpr std::size_t damage_after_damage_offset = 8 + 2 * Observed;
    template<bool Observed>
    inline constexpr std::size_t damage_end_offset = 9 + 2 * Observed;

    inline damage_node& damage_node_at(frame_stack& stack, std::size_t offset) noexcept
    {
        return *std::launder(reinterpret_cast<damage_node*>(stack.data() + offset));
    }

    inline void prepend_damage(frame_stack& stack, damage_node node)
    {
        const auto frame = stack.top<damage_group, substack_t>();
        node.next = get<0>(frame).pending;
        std::size_t offset;
        if(not get<0>(frame).first_used)
        {
            std::construct_at(&get<0>(frame).first, node);
            get<0>(frame).first_used = true;
            offset = reinterpret_cast<unsigned char*>(&get<0>(frame).first) - stack.data();
        }
        else
        {
            const auto added = get<1>(frame).push(node);
            offset = reinterpret_cast<unsigned char*>(&get<0>(added)) - stack.data();
        }
        get<0>(frame).pending = offset;
    }

    inline damage_node make_damage_node(const damage& input, character_id target)
    {
        return { .event = {
            .source = input.source, .target = target, .value = input.value,
            .multiplier_numerator = input.multiplier_numerator,
            .multiplier_denominator = input.multiplier_denominator,
            .type = input.type, .flags = input.flags
        } };
    }

    inline void expand_damage(frame_stack& stack, const unrestricted_table& table, const damage& input)
    {
        switch(input.target.index())
        {
        case 0:
            prepend_damage(stack, make_damage_node(input, std::get<character_id>(input.target)));
            return;
        case 1:
            {
                const auto target = std::get<relative_character_target>(input.target);
                const auto player = table[target.player];
                const auto characters = player.template characters<false>();
                const auto count = characters.size();
                GIVM_ASSERT(count != 0 && player.state().active_character.has_value());
                [[assume(count != 0)]];
                const auto shift = static_cast<std::int64_t>(target.offset) % static_cast<std::int64_t>(count);
                const auto normalized = shift < 0 ? count - static_cast<std::size_t>(-shift) : static_cast<std::size_t>(shift);
                auto index = player.state().active_character->index + normalized;
                if(index >= count) index -= count;
                for(std::size_t visited = 0; visited != count; ++visited)
                {
                    const auto character = characters[index];
                    if(character && character.state().health != 0)
                    {
                        prepend_damage(stack, make_damage_node(input, character.id()));
                        return;
                    }
                    if(++index == count) index = 0;
                }
                return;
            }
        case 2:
            {
                const auto target = std::get<other_characters_target>(input.target);
                const auto characters = table[target.excluded.player_id].template characters<false>();
                const auto count = characters.size();
                // Reverse insertion preserves cyclic order after the excluded
                // character, including when that character has just died.
                auto index = target.excluded.index;
                for(std::size_t remaining = count; remaining > 1; --remaining)
                {
                    index = index == 0 ? count - 1 : index - 1;
                    const auto character = characters[index];
                    if(character && character.state().health != 0)
                        prepend_damage(stack, make_damage_node(input, character.id()));
                }
                return;
            }
        default:
            std::unreachable();
        }
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

    inline void complete_damage(execution_context& context)
    {
        auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        const auto offset = group.pending;
        auto& node = damage_node_at(context.stack(), offset);
        group.pending = node.next;
        if(node.deals_damage || node.event.reaction != elemental_reaction::none)
        {
            node.next = no_damage_node;
            if(group.completed == no_damage_node) group.completed = offset;
            else damage_node_at(context.stack(), group.last_completed).next = offset;
            group.last_completed = offset;
        }
    }

    // Empty means this local stage finished. A value means execution left the
    // local loop: a response program, an observation pause, or game termination.
    template<bool Observed>
    std::optional<execution_state> apply_group_damage_element(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        const auto node = damage_node_at(context.stack(), group.pending);
        const auto& event = node.event;
        const auto incoming = element_from_damage_type(event.type);
        if(event.reaction == elemental_reaction::none)
        {
            if(incoming != element::none || not node.deals_damage)
                table[event.target].state().aura = aura_without_reaction(event.reacted_aura, incoming);
            complete_damage(context);
            return std::nullopt;
        }
        // Replacement changes the reaction's effects, never its aura consumption.
        table[event.target].state().aura = aura_after_reaction(event.reacted_aura, incoming, event.reaction);
        // Retire the active head before prepending its reaction children.
        complete_damage(context);
        if(not event.replacement_reaction)
        {
            if(event.reaction == elemental_reaction::superconduct || event.reaction == elemental_reaction::electro_charged
                || event.reaction == elemental_reaction::swirl)
            {
                const auto aura_element = primary_element_from_aura(event.reacted_aura);
                const auto swirled_element = aura_element == element::anemo ? incoming : aura_element;
                const auto type = event.reaction == elemental_reaction::swirl
                    ? damage_type_from_element(swirled_element) : damage_type::piercing;
                expand_damage(context.stack(), table, damage{
                    .source = event.source, .target = other_characters_target{ event.target },
                    .value = 1, .type = type, .flags = damage_flag_bits::reaction_damage
                });
            }
            else if(event.reaction == elemental_reaction::quicken || event.reaction == elemental_reaction::burning
                || event.reaction == elemental_reaction::bloom)
            {
                const auto player = other_player(event.target.player_id);
                const auto resume = group.instructions + damage_entity_resume_offset<Observed> * sizeof(execute_fn);
                program_entry entry;
                if(event.reaction == elemental_reaction::burning)
                {
                    entry = prepare_summoning(library, table, context, random,
                        { .player = player, .definition = library.burning_flame_id(), .state = { 1, 1 } }, resume);
                }
                else
                {
                    const auto definition = event.reaction == elemental_reaction::quicken
                        ? library.catalyzing_field_id() : library.dendro_core_id();
                    entry = prepare_combat_status_generation(library, table, context, random,
                        { .player = player, .definition = definition }, resume);
                }
                if(entry) return context.enter(entry);
            }
            else if(event.reaction == elemental_reaction::frozen && table[event.target].state().health != 0)
            {
                const auto reapplication_resume = group.instructions + damage_entity_resume_offset<Observed> * sizeof(execute_fn);
                const auto replacement_resume = group.instructions + damage_attachment_resume_offset<Observed> * sizeof(execute_fn);
                return prepare_attachment_application(library, table, context, random,
                    { .target = event.target, .definition = library.frozen_id() }, reapplication_resume, replacement_resume);
            }
        }
        return std::nullopt;
    }

    template<bool Observed>
    std::optional<execution_state> continue_damage_after_health_reduction(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        const auto id = damage_node_at(context.stack(), group.pending).event.target;
        const auto character = table[id];
        if(character.state().health == 0)
        {
            // A future dying broadcast starts here, while attachments still
            // exist. Its failed-revival continuation checks termination first.
            if(all_characters_defeated(table, id.player_id))
            {
                const auto result = all_characters_defeated(table, other_player(id.player_id)) ? game_result::both_loss
                    : id.player_id == player_id{ 0 } ? game_result::player_1_win : game_result::player_0_win;
                return context.end_game(result);
            }
            for(const auto attachment : character.attachments()) attachment.erase();
            character.state().energy = 0;
        }
        return apply_group_damage_element<Observed>(library, table, context, random);
    }

    template<bool Observed>
    std::optional<execution_state> continue_damage_effect(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<damage_effect>(library, table, context, random)) return continue_execution;
        const auto event = get<0>(context.stack().top<damage_effect, execution_position>());
        pop_broadcast<damage_effect>(context);
        auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        damage_node_at(context.stack(), group.pending).event.value = event.value;
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
    std::optional<execution_state> continue_damage_calculation(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<damage_calculation>(library, table, context, random)) return continue_execution;
        auto event = get<0>(context.stack().top<damage_calculation, execution_position>());
        pop_broadcast<damage_calculation>(context);
        auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        auto& node = damage_node_at(context.stack(), group.pending);
        if(not event.replacement_reaction) apply_default_damage_reaction_effect(event, event.reaction);
        event.value = calculate_damage_value(event.value, event.multiplier_numerator, event.multiplier_denominator);
        std::construct_at(&node.event, event);
        const auto position = group.instructions + damage_effect_offset * sizeof(execute_fn);
        prepare_broadcast(library, damage_effect{
            .source = event.source, .target = event.target, .value = event.value,
            .type = event.type, .flags = event.flags, .reaction = event.reaction, .replacement_reaction = event.replacement_reaction
        }, table, context.stack(), position);
        return continue_damage_effect<Observed>(library, table, context, random);
    }

    template<bool Observed>
    std::optional<execution_state> prepare_damage_calculation(
        const definition_library& library, unrestricted_table& table, execution_context& context,
        random_fn& random, tag_id replacement_reaction = {})
    {
        const auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        auto& node = damage_node_at(context.stack(), group.pending);
        const damage_calculation event{
            .source = node.event.source, .target = node.event.target, .value = node.event.value,
            .multiplier_numerator = node.event.multiplier_numerator,
            .multiplier_denominator = node.event.multiplier_denominator,
            .type = node.event.type, .flags = node.event.flags,
            .reaction = node.event.reaction, .reacted_aura = node.event.reacted_aura, .replacement_reaction = replacement_reaction
        };
        if(not node.deals_damage)
        {
            std::construct_at(&node.event, event);
            return apply_group_damage_element<Observed>(library, table, context, random);
        }
        const auto position = group.instructions + damage_calculation_offset * sizeof(execute_fn);
        prepare_broadcast(library, event, table, context.stack(), position);
        return continue_damage_calculation<Observed>(library, table, context, random);
    }

    template<bool Observed>
    std::optional<execution_state> continue_damage_reaction(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<elemental_reaction_will_occur>(library, table, context, random)) return continue_execution;
        const auto event = get<0>(context.stack().top<elemental_reaction_will_occur, execution_position>());
        pop_broadcast<elemental_reaction_will_occur>(context);
        auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        if(not event.replacement_reaction && event.reaction == elemental_reaction::overloaded
            && group.overloaded_player == no_overloaded_player
            && table[event.target.player_id].state().active_character == event.target)
            group.overloaded_player = static_cast<std::uint8_t>(event.target.player_id.index);
        return prepare_damage_calculation<Observed>(library, table, context, random, event.replacement_reaction);
    }

    template<bool Observed>
    std::optional<execution_state> prepare_damage_reaction(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        const auto node = damage_node_at(context.stack(), group.pending);
        if(node.event.reaction == elemental_reaction::none)
            return prepare_damage_calculation<Observed>(library, table, context, random);
        const auto position = group.instructions + damage_reaction_offset * sizeof(execute_fn);
        prepare_broadcast(library, elemental_reaction_will_occur{
            .source = node.event.source, .target = node.event.target,
            .incoming_element = element_from_damage_type(node.event.type), .reacted_aura = node.event.reacted_aura,
            .reaction = node.event.reaction, .cause = node.cause
        }, table, context.stack(), position);
        return continue_damage_reaction<Observed>(library, table, context, random);
    }

    template<bool Observed>
    std::optional<execution_state> continue_damage_preparation(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<damage_preparation>(library, table, context, random)) return continue_execution;
        const auto event = get<0>(context.stack().top<damage_preparation, execution_position>());
        pop_broadcast<damage_preparation>(context);
        const auto aura = table[event.target].state().aura;
        const auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        auto& node = damage_node_at(context.stack(), group.pending);
        std::construct_at(&node.event, damage_calculation{
            .source = event.source, .target = event.target, .value = event.value,
            .multiplier_numerator = event.multiplier_numerator,
            .multiplier_denominator = event.multiplier_denominator,
            .type = event.type, .flags = event.flags,
            .reaction = reaction_from_aura(aura, element_from_damage_type(event.type)), .reacted_aura = aura
        });
        return prepare_damage_reaction<Observed>(library, table, context, random);
    }

    inline std::optional<execution_state> continue_damage_after_damage(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<after_damage>(library, table, context, random)) return continue_execution;
        pop_broadcast<after_damage>(context);
        auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        group.completed = damage_node_at(context.stack(), group.completed).next;
        return std::nullopt;
    }

    template<bool Observed>
    void prepare_group_after_damage(
        const definition_library& library, const unrestricted_table& table, execution_context& context)
    {
        const auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        const auto event = damage_node_at(context.stack(), group.completed).event;
        const auto position = group.instructions + damage_after_damage_offset<Observed> * sizeof(execute_fn);
        prepare_broadcast(library, after_damage{
            event.source, event.target, event.value, event.type, event.flags, event.reaction, event.replacement_reaction },
            table, context.stack(), position);
    }

    template<bool Observed>
    std::optional<execution_state> continue_damage_after_reaction(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<after_elemental_reaction>(library, table, context, random)) return continue_execution;
        pop_broadcast<after_elemental_reaction>(context);
        auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        const auto& node = damage_node_at(context.stack(), group.completed);
        if(not node.deals_damage)
        {
            group.completed = node.next;
            return std::nullopt;
        }
        prepare_group_after_damage<Observed>(library, table, context);
        return continue_damage_after_damage(library, table, context, random);
    }

    template<bool Inputs, bool Observed>
    execution_state continue_damage_completion(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto frame = context.stack().top<damage_group, substack_t>();
        while(get<0>(frame).completed != no_damage_node)
        {
            auto& group = get<0>(frame);
            const auto node = damage_node_at(context.stack(), group.completed);
            if(node.event.reaction != elemental_reaction::none)
            {
                const auto position = group.instructions + damage_after_reaction_offset<Observed> * sizeof(execute_fn);
                prepare_broadcast(library, after_elemental_reaction{
                    .source = node.event.source, .target = node.event.target,
                    .incoming_element = element_from_damage_type(node.event.type), .reacted_aura = node.event.reacted_aura,
                    .reaction = node.event.reaction, .cause = node.cause, .replacement_reaction = node.event.replacement_reaction
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
    execution_state continue_damage_overloaded_switch(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(not continue_broadcast<active_character_changed>(library, table, context, random)) return continue_execution;
        pop_broadcast<active_character_changed>(context);
        return continue_damage_completion<Inputs, Observed>(library, table, context, random);
    }

    template<bool Inputs>
    execution_state resume_damage_overloaded_observation(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto frame = context.stack().top<active_character_changed, execution_position>();
        const auto event = get<0>(frame);
        const auto position = get<1>(frame);
        context.stack().pop<active_character_changed, execution_position>();
        table[event.current.player_id].state().active_character = event.current;
        prepare_broadcast(library, event, table, context.stack(), position);
        return continue_damage_overloaded_switch<Inputs, true>(library, table, context, random);
    }

    template<bool Inputs, bool Observed>
    execution_state finish_damage_group(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        auto& group = get<0>(context.stack().top<damage_group, substack_t>());
        if(group.overloaded_player == no_overloaded_player)
            return continue_damage_completion<Inputs, Observed>(library, table, context, random);
        const auto player = table[player_id{ group.overloaded_player }];
        group.overloaded_player = no_overloaded_player;
        const auto characters = player.template characters<false>();
        GIVM_ASSERT(player.state().active_character.has_value());
        const auto current = *player.state().active_character;
        if(library.is_control_immune(std::as_const(table)[current]))
            return continue_damage_completion<Inputs, Observed>(library, table, context, random);
        auto index = current.index;
        for(std::size_t remaining = characters.size(); remaining != 0; --remaining)
        {
            if(++index == characters.size()) index = 0;
            const auto target = characters[index];
            if(not target || target.state().health == 0) continue;
            if(target.id() == current) break;

            const active_character_changed event{ .current = target.id() };
            const auto position = group.instructions + damage_overloaded_broadcast_offset<Observed> * sizeof(execute_fn);
            if constexpr(Observed)
            {
                const auto observation = group.instructions + damage_overloaded_observation_offset<Observed> * sizeof(execute_fn);
                // Collect responders after applying the change, in both modes.
                context.stack().push(event, position);
                context.jump(observation);
                return context.yield(execution_state::active_character_changed);
            }
            else
            {
                player.state().active_character = event.current;
                prepare_broadcast(library, event, table, context.stack(), position);
                return continue_damage_overloaded_switch<Inputs, Observed>(library, table, context, random);
            }
        }
        return continue_damage_completion<Inputs, Observed>(library, table, context, random);
    }

    template<bool Inputs, bool Observed>
    execution_state continue_damage_group(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto frame = context.stack().top<damage_group, substack_t>();
        while(true)
        {
            if(get<0>(frame).pending == no_damage_node)
            {
                if(get<0>(frame).input_cursor == get<0>(frame).input_count)
                    return finish_damage_group<Inputs, Observed>(library, table, context, random);

                const auto index = get<0>(frame).input_cursor++;
                damage input;
                if constexpr(Inputs)
                {
                    constexpr auto stride = align(sizeof(damage), max_alignment);
                    constexpr auto leading_padding = align(sizeof(damage_group), max_alignment) - sizeof(damage_group);
                    // The first input frame ends immediately below this group's
                    // fixed prefix; its address is already available here.
                    const auto input_end = reinterpret_cast<const unsigned char*>(&get<0>(frame)) - leading_padding;
                    std::memcpy(&input, input_end
                        - index * stride - sizeof(damage), sizeof(damage));
                }
                else
                {
                    // Calls can originate at any resume opcode. Locate the
                    // immutable descriptions using the group's instruction base.
                    const auto bytes = &context.instruction_data<0, unsigned char>(library) - context.position();
                    std::memcpy(&input, bytes + get<0>(frame).instructions
                        - (get<0>(frame).input_count - index) * padded_size<damage>, sizeof(damage));
                }
                expand_damage(context.stack(), table, input);
                if(get<0>(frame).pending == no_damage_node) continue;
            }

            const auto offset = get<0>(frame).pending;
            const auto node = damage_node_at(context.stack(), offset);
            const auto target = table[node.event.target];
            // Earlier hits and their response programs can invalidate a queued
            // target. This is execution semantics, not argument validation.
            if(not target || target.state().health == 0)
            {
                get<0>(frame).pending = node.next;
                continue;
            }
            const auto position = get<0>(frame).instructions + damage_preparation_offset * sizeof(execute_fn);
            prepare_broadcast(library, damage_preparation{
                .source = node.event.source, .target = node.event.target, .value = node.event.value,
                .multiplier_numerator = node.event.multiplier_numerator,
                .multiplier_denominator = node.event.multiplier_denominator,
                .type = node.event.type, .flags = node.event.flags
            }, table, context.stack(), position);
            if(const auto state = continue_damage_preparation<Observed>(library, table, context, random)) return *state;
        }
    }

    template<bool Inputs, bool Observed, auto Continue>
    execution_state resume_damage_group(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(const auto state = Continue(library, table, context, random)) return *state;
        return continue_damage_group<Inputs, Observed>(library, table, context, random);
    }

    template<bool Inputs, bool Observed, auto Continue>
    execution_state resume_damage_completion(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        if(const auto state = Continue(library, table, context, random)) return *state;
        return continue_damage_completion<Inputs, Observed>(library, table, context, random);
    }

    template<bool Inputs>
    execution_state resume_damage_health_observation(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        context.stack().pop<after_damage>();
        if(const auto state = continue_damage_after_health_reduction<true>(library, table, context, random)) return *state;
        return continue_damage_group<Inputs, true>(library, table, context, random);
    }

    template<bool Inputs, bool Observed>
    execution_state resume_damage_entity_generation(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        context.stack().pop<execution_position>();
        return continue_damage_group<Inputs, Observed>(library, table, context, random);
    }

    template<bool Inputs, bool Observed>
    execution_state prepare_damage_group(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn& random)
    {
        const auto count = context.instruction_data<1, std::size_t>(library);
        const auto position = context.position() + instruction_extent<1, std::size_t>
            + (Inputs ? 0 : count * padded_size<damage>);
        context.stack().push(damage_group{ .input_count = count, .instructions = position }, substack());
        return continue_damage_group<Inputs, Observed>(library, table, context, random);
    }

    template<bool Inputs, bool Observed>
    void compile_damage_resolution(program_writer& writer)
    {
        writer.write(execute_fn{ resume_damage_group<Inputs, Observed, continue_damage_preparation<Observed>> });
        writer.write(execute_fn{ resume_damage_group<Inputs, Observed, continue_damage_reaction<Observed>> });
        writer.write(execute_fn{ resume_damage_group<Inputs, Observed, continue_damage_calculation<Observed>> });
        writer.write(execute_fn{ resume_damage_group<Inputs, Observed, continue_damage_effect<Observed>> });
        if constexpr(Observed) writer.write(execute_fn{ resume_damage_health_observation<Inputs> });
        writer.write(execute_fn{ resume_damage_entity_generation<Inputs, Observed> });
        writer.write(execute_fn{ resume_damage_group<Inputs, Observed, continue_attachment_replacement> });
        if constexpr(Observed) writer.write(execute_fn{ resume_damage_overloaded_observation<Inputs> });
        writer.write(execute_fn{ continue_damage_overloaded_switch<Inputs, Observed> });
        writer.write(execute_fn{ resume_damage_completion<Inputs, Observed, continue_damage_after_reaction<Observed>> });
        writer.write(execute_fn{ resume_damage_completion<Inputs, Observed, continue_damage_after_damage> });
    }

    template<bool Inputs, bool Observed>
    void compile_damage_group(program_writer& writer, const givm::deal_damage& command)
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
