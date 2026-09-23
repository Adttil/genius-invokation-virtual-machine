#ifndef GIVM_EXECUTOR_CHARACTER_TARGET_HPP
#define GIVM_EXECUTOR_CHARACTER_TARGET_HPP

#include <cstddef>
#include <cstdint>
#include <optional>

#include "../definition.hpp"
#include "../macro_define.hpp"

namespace givm::detail
{
    template<bool SkipDefeated>
    inline std::optional<character_id> resolve_character_target(
        const unrestricted_table& table, relative_character_target target)
    {
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

#include "../macro_undef.hpp"
#endif
