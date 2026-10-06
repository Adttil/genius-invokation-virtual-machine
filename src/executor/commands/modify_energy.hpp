#ifndef GIVM_EXECUTOR_COMMANDS_MODIFY_ENERGY_HPP
#define GIVM_EXECUTOR_COMMANDS_MODIFY_ENERGY_HPP

#include "../program_writer.hpp"

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <cstddef>
#include <cstdint>

#include <givm/executor/executor.hpp>
#include "../character_target.hpp"
#include "../broadcast.hpp"
#include <givm/definition.hpp>
#include <givm/macro_define.hpp>

namespace givm::detail
{
    template<bool Fixed, character_selection Selection = character_selection::character>
    inline execution_state execute_energy_modification(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn&)
    {
        const auto modify = [&](character_id target, std::int64_t delta)
        {
            auto& state = table[target].state();
            if(not state.alive) return;
            const auto previous = static_cast<std::int64_t>(state.energy);
            if(delta <= -previous)
                state.energy = 0;
            else if(delta >= static_cast<std::int64_t>(state.max_energy) - previous)
                state.energy = state.max_energy;
            else
                state.energy = static_cast<std::uint32_t>(previous + delta);
            if(state.energy != previous) append_event_record(context, energy_changed{ target,
                static_cast<std::uint32_t>(previous), state.energy });
        };
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, modify_energy>(library);
            context.advance(instruction_extent<1, modify_energy>);
            if constexpr(Selection == character_selection::character)
            {
                const auto target = resolve_character_target<false>(table, command.target);
                if(target) modify(*target, command.delta);
            }
            else
            {
                const auto self = table.state().self_player;
#ifndef NDEBUG
                debug_validate_entity(table, self, "modify_energy", "self_player");
#endif
                GIVM_ASSERT(self.index < 2);
                [[assume(self.index < 2)]];
                const auto player = table[command.target.player == relative_player::self ? self : other_player(self)];
                const auto characters = player.template characters<false>();
                const auto count = characters.size();
                if(count == 0 || not player.state().active_character) return continue_execution;

#ifndef NDEBUG
                debug_validate_entity(table, *player.state().active_character, "modify_energy", "active_character", true);
#endif
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
                        modify(character.id(), command.delta);
                }
            }
        }
        else
        {
            const auto [targets, delta] = context.stack().top<character_id[], std::int64_t>();
#ifndef NDEBUG
            for(std::size_t index = 0; index != targets.size(); ++index)
                debug_validate_entity(table, targets[index], "modify_energy", "targets[" + std::to_string(index) + "]");
#endif
            for(const auto target : targets)
            {
                const auto character = table[target];
                GIVM_ASSERT(character.is_valid());
                modify(target, delta);
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
            case character_selection::prioritized:
                std::unreachable();
            }
            writer.write(command);
        }
    }
}

namespace givm::detail
{
    inline std::vector<modify_energy::error_type> check(const modify_energy& command, const definition_compile_context&, program_kind kind)
    {
        using reason = modify_energy::error_type::reason;
        std::vector<modify_energy::error_type> errors;
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.target.player != relative_player::self && command.target.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_target_player, .value = static_cast<std::size_t>(command.target.player) });
        if(command.target.selection != character_selection::character && command.target.selection != character_selection::others
            && command.target.selection != character_selection::all)
            errors.push_back({ .cause = reason::invalid_target_selection, .value = static_cast<std::size_t>(command.target.selection) });
        return errors;
    }
}

#include <givm/macro_undef.hpp>

namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const modify_energy& command) noexcept
    {
        return command.target.offset == std::numeric_limits<std::int32_t>::max() ? TInputTypes::template index_of<modify_energy::input_type>() : std::size_t(-1);
    }
}

#endif
