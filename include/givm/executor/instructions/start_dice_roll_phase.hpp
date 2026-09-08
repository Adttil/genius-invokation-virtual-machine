#ifndef GIVM_EXECUTOR_INSTRUCTIONS_START_DICE_ROLL_PHASE_HPP
#define GIVM_EXECUTOR_INSTRUCTIONS_START_DICE_ROLL_PHASE_HPP

#include "../executor.hpp"

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <span>

#include "../broadcast.hpp"
#include "../events.hpp"
#include "../selector.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    inline constexpr std::uint32_t packed_dice_per_random = 10;

    struct dice_reroll_lane
    {
        std::uint32_t remaining = 0;
        std::uint32_t cursor = 0;
    };

    struct dice_reroll_phase
    {
        std::uint32_t dice_count = 0;
        std::uint32_t player0_random_count = 0;
        dice_reroll_lane first;
        dice_reroll_lane second;
    };

    namespace detail
    {
        inline constexpr size_t packed_dice_random_count(size_t dice_count) noexcept
        {
            return (dice_count + packed_dice_per_random - 1) / packed_dice_per_random;
        }

        inline dice_counts selected_reroll_dice_from_mask(
            dice_counts dice,
            std::bitset<selection_capacity> selected_mask
        )
        {
            dice_counts selected_dice;
            size_t selection_index = 0;
            for(size_t dice_index = 0; dice_index < 8; ++dice_index)
            {
                const auto current_dice = static_cast<elemental_dice>(dice_index);
                for(std::uint8_t count = 0; count < dice[current_dice]; ++count)
                {
                    GIVM_ASSERT(selection_index < selection_capacity);
                    if(selected_mask[selection_index])
                    {
                        ++selected_dice[current_dice];
                    }
                    ++selection_index;
                }
            }
            GIVM_ASSERT(selected_mask.count() == selected_dice.total());
            return selected_dice;
        }

        inline elemental_dice draw_packed_dice(std::span<const std::uint32_t> pool, std::uint32_t& cursor) noexcept
        {
            const auto random_index = cursor / packed_dice_per_random;
            const auto dice_index = cursor % packed_dice_per_random;
            GIVM_ASSERT(random_index < pool.size());
            const auto value = pool[random_index] >> (dice_index * 3);
            ++cursor;
            return elemental_dice_from_random(value);
        }

        inline void draw_selected_dice(
            card_table& table,
            player_id player,
            dice_reroll_lane& lane,
            std::span<const std::uint32_t> pool,
            std::bitset<selection_capacity> selected_mask
        )
        {
            auto& state = table[player].state();
            const auto selected = selected_reroll_dice_from_mask(state.dice, selected_mask);
            const auto selected_count = selected.total();
            GIVM_ASSERT(pool.size() * packed_dice_per_random >= static_cast<size_t>(lane.cursor) + selected_count);

            dice_counts drawn;
            for(std::uint32_t index = 0; index < selected_count; ++index)
            {
                ++drawn[draw_packed_dice(pool, lane.cursor)];
            }

            state.dice -= selected;
            state.dice += drawn;
        }

        inline void process_reroll_lane(
            card_table& table,
            player_id player,
            dice_reroll_lane& lane,
            std::span<const std::uint32_t> pool,
            selector& input
        )
        {
            GIVM_ASSERT(input.player == player);
            GIVM_ASSERT(lane.remaining != 0);

            const auto selected = input.selected;
            if(selected.none())
            {
                lane.remaining = 0;
                return;
            }

            draw_selected_dice(table, player, lane, pool, selected);
            --lane.remaining;
        }

        inline player_id next_reroll_player(const dice_reroll_phase& phase) noexcept
        {
            GIVM_ASSERT(phase.first.remaining != 0 || phase.second.remaining != 0);
            if(phase.first.remaining != 0)
            {
                return player_id{ 0 };
            }
            return player_id{ 1 };
        }
    }

    struct start_dice_roll_phase
    {
        using context_type = void;

        std::uint32_t count = 8;
        std::array<std::uint32_t, 2> reroll_count{ 1, 1 };

        enum class stage_type : stage_t
        {
            prepare_broadcast,
            apply_preparation,
            reroll
        };
        bool execute(card_table& table, execution_context& context, random_fn& random) const
        {
            const auto stage = static_cast<stage_type>(context.current_stage());
            if(stage == stage_type::prepare_broadcast)
            {
                detail::prepare_broadcast(
                    dice_roll_preparation{
                        .count = count,
                        .reroll_count = reroll_count
                    },
                    table,
                    context.stack()
                );
                context.current_stage() = static_cast<stage_t>(stage_type::apply_preparation);
                return true;
            }

            if(stage == stage_type::apply_preparation)
            {
                if(not detail::continue_broadcast<dice_roll_preparation>(table, context, random))
                {
                    return true;
                }

                auto&& [targets, cursor, event_ref, current_handler, broadcast_stage] =
                    context.stack().top<
                        detail::handler_id<dice_roll_preparation>[],
                        stack_count_t,
                        dice_roll_preparation,
                        detail::handler_id<dice_roll_preparation>,
                        stage_t
                    >();
                (void)targets;
                (void)cursor;
                (void)current_handler;
                (void)broadcast_stage;
                const auto event = event_ref;
                GIVM_ASSERT(event.count <= selection_capacity);

                for(size_t player_index = 0; player_index < 2; ++player_index)
                {
                    const player_id player{ player_index };
                    const auto fixed_count = event.fixed_dice[player_index].total();
                    GIVM_ASSERT(fixed_count <= event.count);

                    auto& dice = table[player].state().dice;
                    dice = event.fixed_dice[player_index];
                    for(size_t index = fixed_count; index < event.count; ++index)
                    {
                        ++dice[elemental_dice_from_random(random())];
                    }
                }

                dice_reroll_phase phase{
                    .dice_count = event.count,
                    .first = { .remaining = event.reroll_count[0] },
                    .second = { .remaining = event.reroll_count[1] }
                };

                detail::pop_broadcast<dice_roll_preparation>(context);
                if(phase.first.remaining == 0 && phase.second.remaining == 0)
                {
                    return context.enter_next();
                }

                const auto first_pool_size = static_cast<size_t>(phase.first.remaining) * phase.dice_count;
                const auto second_pool_size = static_cast<size_t>(phase.second.remaining) * phase.dice_count;
                const auto first_random_count = detail::packed_dice_random_count(first_pool_size);
                const auto second_random_count = detail::packed_dice_random_count(second_pool_size);
                phase.player0_random_count = static_cast<std::uint32_t>(first_random_count);
                auto&& [random_pool, stored_phase, input, stored_stage] = context.stack().push(
                    dynamic_array<std::uint32_t>(first_random_count + second_random_count),
                    phase,
                    selector{ .player = detail::next_reroll_player(phase) },
                    static_cast<stage_t>(stage_type::reroll)
                );
                (void)stored_phase;
                (void)input;
                (void)stored_stage;
                for(auto& value : random_pool)
                {
                    value = random();
                }

                return context.yield();
            }

            auto&& [random_pool, phase, input, stored_stage] =
                context.stack().top<std::uint32_t[], dice_reroll_phase, selector, stage_t>();
            (void)stage;
            (void)stored_stage;

            if(input.player == player_id{ 0 })
            {
                detail::process_reroll_lane(
                    table,
                    player_id{ 0 },
                    phase.first,
                    random_pool.subspan(0, phase.player0_random_count),
                    input
                );
            }
            else
            {
                GIVM_ASSERT(input.player == player_id{ 1 });
                detail::process_reroll_lane(
                    table,
                    player_id{ 1 },
                    phase.second,
                    random_pool.subspan(phase.player0_random_count),
                    input
                );
            }

            if(phase.first.remaining == 0 && phase.second.remaining == 0)
            {
                context.stack().pop<std::uint32_t[], dice_reroll_phase, selector, stage_t>();
                return context.enter_next();
            }

            input.player = detail::next_reroll_player(phase);
            input.selected.reset();
            return context.yield();
        }
    };
}

#include "../../macro_undef.hpp"
#endif
