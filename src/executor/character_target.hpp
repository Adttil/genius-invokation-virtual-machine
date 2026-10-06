#ifndef GIVM_EXECUTOR_CHARACTER_TARGET_HPP
#define GIVM_EXECUTOR_CHARACTER_TARGET_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

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
            && target.selection != character_selection::all && target.selection != character_selection::prioritized)
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
                    if(character.state().alive && character.state().health != 0) return character.id();
                }
                else return character.id();
            }
            if(++index == count) index = 0;
        }
        return std::nullopt;
    }

    inline std::optional<character_id> resolve_damage_target(
        const unrestricted_table& table, relative_character_target target)
    {
        if(target.selection == character_selection::prioritized) return resolve_character_target<true>(table, target);
        return resolve_character_target<false>(table, target);
    }

    inline std::vector<character_id> collect_character_targets(const unrestricted_table& table,
        character_id anchor, character_selection selection, bool include_defeated = false)
    {
        const auto characters = table[anchor.player_id].template characters<false>();
        std::vector<character_id> targets;
        auto index = anchor.index;
        const auto count = characters.size();
        const auto remaining = selection == character_selection::character ? 1
            : selection == character_selection::prioritized ? count
            : count - (selection == character_selection::others);
        if(selection == character_selection::others && ++index == count) index = 0;
        for(std::size_t visited = 0; visited != remaining; ++visited)
        {
            const auto character = characters[index];
            if(character && (include_defeated || character.state().alive))
            {
                if(selection != character_selection::prioritized || character.state().health != 0)
                {
                    targets.push_back(character.id());
                    if(selection == character_selection::prioritized) break;
                }
            }
            if(++index == count) index = 0;
        }
        return targets;
    }
}

#include <givm/macro_undef.hpp>
#endif
