#ifndef GIVM_EXECUTOR_COMMANDS_MODIFY_ENERGY_HPP
#define GIVM_EXECUTOR_COMMANDS_MODIFY_ENERGY_HPP

#include <cstddef>
#include <cstdint>

#include "../executor.hpp"
#include "../character_target.hpp"
#include "../../definition/commands.hpp"
#include "../../macro_define.hpp"

namespace givm::detail
{
    template<bool Fixed, character_selection Selection = character_selection::character>
    inline execution_state execute_energy_modification(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn&)
    {
        const auto modify = [](character_state& state, std::int64_t delta)
        {
            const auto previous = static_cast<std::int64_t>(state.energy);
            if(delta <= -previous)
                state.energy = 0;
            else if(delta >= static_cast<std::int64_t>(state.max_energy) - previous)
                state.energy = state.max_energy;
            else
                state.energy = static_cast<std::uint32_t>(previous + delta);
        };
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, modify_energy>(library);
            context.advance(instruction_extent<1, modify_energy>);
            if constexpr(Selection == character_selection::character)
            {
                const auto target = resolve_character_target<false>(table, command.target);
                if(target) modify(table[*target].state(), command.delta);
            }
            else
            {
                const auto self = table.state().self_player;
                GIVM_ASSERT(self.index < 2);
                [[assume(self.index < 2)]];
                const auto player = table[command.target.player == relative_player::self ? self : other_player(self)];
                const auto characters = player.template characters<false>();
                const auto count = characters.size();
                if(count == 0 || not player.state().active_character) return continue_execution;

                std::size_t anchor = 0;
                if constexpr(Selection == character_selection::others)
                {
                    const auto shift = static_cast<std::int64_t>(command.target.offset) % static_cast<std::int64_t>(count);
                    const auto normalized = shift < 0 ? count - static_cast<std::size_t>(-shift)
                                                     : static_cast<std::size_t>(shift);
                    anchor = player.state().active_character->index + normalized;
                    if(anchor >= count) anchor -= count;
                }
                for(std::size_t index = 0; index != count; ++index)
                {
                    if constexpr(Selection == character_selection::others)
                        if(index == anchor) continue;
                    const auto character = characters[index];
                    if(character && character.state().health != 0)
                        modify(character.state(), command.delta);
                }
            }
        }
        else
        {
            const auto [targets, delta] = context.stack().top<character_id[], std::int64_t>();
            for(const auto target : targets)
            {
                const auto character = table[target];
                GIVM_ASSERT(character.is_valid());
                modify(character.state(), delta);
            }
            context.stack().pop<character_id[], std::int64_t>();
            context.enter_next();
        }
        return continue_execution;
    }

    inline void compile(program_writer& writer, const givm::modify_energy& command, compile_mode)
    {
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
            writer.write(execute_fn{ execute_energy_modification<false> });
        else
        {
            switch(command.target.selection)
            {
            case character_selection::character:
                writer.write(execute_fn{ execute_energy_modification<true> });
                break;
            case character_selection::others:
                writer.write(execute_fn{ execute_energy_modification<true, character_selection::others> });
                break;
            case character_selection::all:
                writer.write(execute_fn{ execute_energy_modification<true, character_selection::all> });
                break;
            }
            writer.write(command);
        }
    }
}

#include "../../macro_undef.hpp"
#endif
