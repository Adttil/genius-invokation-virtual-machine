#ifndef GIVM_EXECUTOR_CHARACTER_TARGET_HPP
#define GIVM_EXECUTOR_CHARACTER_TARGET_HPP

#include <cstddef>
#include <cstdint>
#include <optional>

#include <givm/definition.hpp>
#ifndef NDEBUG
#include "debug_validation.hpp"
#endif

#include <givm/macro_define.hpp>

namespace givm::detail
{
#ifndef NDEBUG
    inline void debug_validate_relative_character_target(const unrestricted_table& table, relative_character_target target,
        std::string_view command, std::string_view field)
    {
        if(target.player != relative_player::self && target.player != relative_player::opponent)
            throw command_input_error{ command, invalid_enum_argument{ std::string{ field } + ".player", static_cast<std::size_t>(target.player) } };
        if(target.selection != character_selection::character && target.selection != character_selection::others
            && target.selection != character_selection::all)
            throw command_input_error{ command, invalid_enum_argument{ std::string{ field } + ".selection", static_cast<std::size_t>(target.selection) } };
        debug_validate_entity(table, table.state().self_player, command, "self_player");
        const auto player = target.player == relative_player::self ? table.state().self_player : other_player(table.state().self_player);
        if(const auto active = table[player].state().active_character)
        {
            debug_validate_entity(table, *active, command, "active_character", true);
            if(active->player_id != player)
                throw command_input_error{ command, invalid_entity_relation{ "active_character", invalid_entity_relation::reason::inactive_character } };
        }
    }
#endif

    template<bool SkipDefeated>
    inline std::optional<character_id> resolve_character_target(
        const unrestricted_table& table, relative_character_target target)
    {
#ifndef NDEBUG
        debug_validate_relative_character_target(table, target, "relative_character_target", "target");
#endif
        const auto self = table.state().self_player;
        GIVM_ASSERT(self.index < 2);
        [[assume(self.index < 2)]];
        const auto player = table[target.player == relative_player::self ? self : other_player(self)];
        const auto characters = player.template characters<false>();
        const auto count = characters.size();
        if(count == 0) return std::nullopt;

        if(not player.state().active_character) return std::nullopt;
        const auto shift = static_cast<std::int64_t>(target.offset) % static_cast<std::int64_t>(count);
        const auto normalized = shift < 0 ? count - static_cast<std::size_t>(-shift)
                                         : static_cast<std::size_t>(shift);
        auto index = player.state().active_character->index + normalized;
        if(index >= count) index -= count;
        for(std::size_t visited = 0; visited != count; ++visited)
        {
            const auto character = characters[index];
            if(character)
            {
                if constexpr(SkipDefeated)
                {
                    if(character.state().health != 0) return character.id();
                }
                else return character.id();
            }
            if(++index == count) index = 0;
        }
        return std::nullopt;
    }
}

#include <givm/macro_undef.hpp>
#endif
