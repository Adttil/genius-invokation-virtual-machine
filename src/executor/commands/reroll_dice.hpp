#ifndef GIVM_EXECUTOR_COMMANDS_REROLL_DICE_HPP
#define GIVM_EXECUTOR_COMMANDS_REROLL_DICE_HPP

#include "../program_writer.hpp"

#include <vector>

#include <cstddef>
#include <cstdint>
#include <span>

#include <givm/executor/executor.hpp>
#include <givm/executor/views/dice_reroll_selection.hpp>
#include <givm/definition.hpp>
#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <givm/macro_define.hpp>

namespace givm::detail
{
    inline constexpr std::uint32_t packed_dice_per_random = 10;

    inline constexpr size_t packed_dice_random_count(size_t dice_count) noexcept
    {
        return (dice_count + packed_dice_per_random - 1) / packed_dice_per_random;
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
        unrestricted_table& table,
        player_id player,
        dice_reroll_lane& lane,
        std::span<const std::uint32_t> pool,
        const dice_counts& selected,
        std::uint32_t selected_count
    )
    {
#ifndef NDEBUG
        debug_validate_entity(table, player, "reroll_dice", "player");
        if(not table[player].state().dice.contains(selected))
            throw command_input_error{ "reroll_dice", insufficient_dice_argument{ player, selected, table[player].state().dice } };
#endif
        auto& state = table[player].state();
        GIVM_ASSERT(pool.size() * packed_dice_per_random >= static_cast<size_t>(lane.cursor) + selected_count);

        dice_counts drawn;
        for(std::uint32_t index = 0; index < selected_count; ++index)
        {
            ++drawn[draw_packed_dice(pool, lane.cursor)];
        }

        state.dice -= selected;
        state.dice += drawn;
    }

#ifndef NDEBUG
    inline void debug_validate_dice_reroll(const unrestricted_table& table, const reroll_dice_input& input)
    {
        debug_validate_entity(table, input.player, "reroll_dice", "player");
        const auto total_draws = static_cast<std::uint64_t>(table[input.player].state().dice.total()) * input.reroll_count;
        if(total_draws > std::numeric_limits<std::uint32_t>::max())
            throw command_input_error{ "reroll_dice", invalid_numeric_argument{ "reroll_count * dice_count", total_draws, std::numeric_limits<std::uint32_t>::max() } };
    }
#endif

    template<bool Fixed>
    inline execution_state prepare_single_player_dice_reroll(
        const definition_library& library, unrestricted_table& table,
        execution_context& context, random_fn& random)
    {
        reroll_dice_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, reroll_dice>(library);
#ifndef NDEBUG
            debug_validate_entity(table, table.state().self_player.get(), "reroll_dice", "self_player");
#endif
            input = {
                .player = command.player == relative_player::self
                    ? table.state().self_player.get() : other_player(table.state().self_player.get()),
                .reroll_count = command.reroll_count
            };
#ifndef NDEBUG
            debug_validate_dice_reroll(table, input);
#endif
            context.advance(instruction_extent<1, reroll_dice>);
        }
        else
        {
            input = get<0>(context.stack().top<reroll_dice_input>());
#ifndef NDEBUG
            debug_validate_dice_reroll(table, input);
#endif
            context.stack().pop<reroll_dice_input>();
            context.enter_next();
        }

        if(input.reroll_count == 0)
            return context.enter_next();
        GIVM_ASSERT(input.player.index() < 2);
        const auto dice_count = table[input.player].state().dice.total();
        if(dice_count == 0)
            return context.enter_next();

        const auto random_count = packed_dice_random_count(static_cast<size_t>(dice_count) * input.reroll_count);
        auto random_pool = get<0>(context.stack().push(
            dynamic_array<std::uint32_t>(random_count),
            single_player_dice_reroll{ input.player, { .remaining = input.reroll_count } },
            dice_counts{}
        ));
        for(auto& value : random_pool)
            value = random();
        return context.yield(execution_state::dice_reroll_selection);
    }

    inline execution_state apply_single_player_dice_reroll(
        const definition_library&, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        auto&& [random_pool, phase, selected] =
            context.stack().top<std::uint32_t[], single_player_dice_reroll, dice_counts>();
        const auto selected_count = selected.total();
        if(selected_count == 0)
            phase.lane.remaining = 0;
        else
        {
            draw_selected_dice(table, phase.player, phase.lane, random_pool, selected, selected_count);
            --phase.lane.remaining;
        }

        if(phase.lane.remaining == 0)
        {
            context.stack().pop<std::uint32_t[], single_player_dice_reroll, dice_counts>();
            return context.enter_next();
        }
        selected = {};
        return context.yield(execution_state::dice_reroll_selection);
    }

    inline void compile(program_writer& writer, const reroll_dice& command, compile_mode)
    {
        if(command.player == static_cast<relative_player>(-1))
            writer.write(execute_fn{ prepare_single_player_dice_reroll<false> });
        else
        {
            GIVM_ASSERT(command.player == relative_player::self || command.player == relative_player::opponent);
            if(command.reroll_count == 0)
                return;
            writer.write(execute_fn{ prepare_single_player_dice_reroll<true> });
            writer.write(command);
        }
        writer.write(execute_fn{ apply_single_player_dice_reroll });
    }
}

namespace givm::detail
{
    inline std::vector<reroll_dice::error_type> check(const reroll_dice& command, const definition_compile_context&, program_kind kind)
    {
        using reason = reroll_dice::error_type::reason;
        std::vector<reroll_dice::error_type> errors;
        if(command.player == static_cast<relative_player>(-1))
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.player != relative_player::self && command.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_player, .value = static_cast<std::size_t>(command.player) });
        return errors;
    }
}

#include <givm/macro_undef.hpp>

namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const reroll_dice& command) noexcept
    {
        return command.player == static_cast<relative_player>(-1) ? TInputTypes::template index_of<reroll_dice::input_type>() : std::size_t(-1);
    }
}

#endif
