#ifndef GIVM_EXECUTOR_DEBUG_VALIDATION_HPP
#define GIVM_EXECUTOR_DEBUG_VALIDATION_HPP

#ifndef NDEBUG
#include <algorithm>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

#include "../definition.hpp"
#include "../table.hpp"
#include "command_input_error.hpp"
#include "library.hpp"

namespace givm::detail
{
    template<class TTable, class TId>
    inline void debug_validate_entity(const TTable& table, TId id, std::string_view command,
        std::string_view field, bool allow_removed = false)
    {
        if constexpr(requires { std::variant_size<TId>::value; })
            std::visit([&](auto value) { debug_validate_entity(table, value, command, field, allow_removed); }, id);
        else if constexpr(std::is_same_v<TId, std::monostate>)
            return;
        else
        {
            bool in_range;
            if constexpr(std::is_same_v<TId, player_id>)
                in_range = id.index < 2;
            else if constexpr(requires { id.character_id; })
            {
                debug_validate_entity(table, id.character_id, command, std::string{ field } + ".character", allow_removed);
                const auto character = table[id.character_id];
                if constexpr(std::is_same_v<TId, skill_id>)
                    in_range = id.index < character.template skills<false>().size();
                else
                    in_range = id.index < character.template attachments<false>().size();
            }
            else if constexpr(requires { id.card_id; })
            {
                debug_validate_entity(table, id.card_id, command, std::string{ field } + ".card", allow_removed);
                in_range = table.debug_entity_in_range(id);
            }
            else
            {
                debug_validate_entity(table, id.player_id, command, std::string{ field } + ".player", allow_removed);
                const auto player = table[id.player_id];
                if constexpr(std::is_same_v<TId, character_id>)
                    in_range = id.index < player.template characters<false>().size();
                else if constexpr(std::is_same_v<TId, hand_card_id>)
                    in_range = id.index < player.template hand_cards<false>().size();
                else if constexpr(std::is_same_v<TId, deck_card_id>)
                    in_range = table.debug_entity_in_range(id);
                else if constexpr(std::is_same_v<TId, support_id>)
                    in_range = id.index < player.template supports<false>().size();
                else if constexpr(std::is_same_v<TId, summon_id>)
                    in_range = id.index < player.template summons<false>().size();
                else
                    in_range = id.index < player.template combat_statuses<false>().size();
            }
            if(not in_range)
                throw command_input_error{ command, invalid_entity_argument{
                    std::string{ field }, id, invalid_entity_argument::reason::out_of_range } };
            if constexpr(not std::is_same_v<TId, player_id>)
                if(not allow_removed && not table[id].is_valid())
                    throw command_input_error{ command, invalid_entity_argument{
                        std::string{ field }, id, invalid_entity_argument::reason::removed } };
        }
    }

    template<class TCategory>
    inline void debug_validate_definition(const definition_library& library, definition_id<TCategory> id,
        std::string_view command, std::string_view field)
    {
        const auto count = library.template definition_count<TCategory>();
        if(id.value() >= count)
            throw command_input_error{ command, invalid_definition_argument{
                std::string{ field }, definition_types::index_of<TCategory>(), id.value(), count } };
    }

    template<class T>
    inline void debug_validate_unique(std::span<const T> values, std::string_view command, std::string_view field)
    {
        for(std::size_t index = 0; index != values.size(); ++index)
            for(std::size_t first = 0; first != index; ++first)
                if(values[first] == values[index])
                    throw command_input_error{ command, duplicate_entity_argument{ std::string{ field }, first, index } };
    }

    template<class TTable, class TCategory>
    inline void debug_validate_required_entity(const TTable& table, player_id player, definition_id<TCategory> definition,
        std::string_view command, std::string_view field)
    {
        debug_validate_entity(table, player, command, "player");
        auto entities = [&]
        {
            if constexpr(std::is_same_v<TCategory, support_view>) return table[player].supports();
            else if constexpr(std::is_same_v<TCategory, summon_view>) return table[player].summons();
            else return table[player].combat_statuses();
        }();
        if(not std::ranges::any_of(entities, [&](const auto entity) { return entity.definition_id() == definition; }))
            throw command_input_error{ command, missing_entity_argument{
                std::string{ field }, command_entity_id{ player }, definition.value() } };
    }

    template<class TTable>
    inline void debug_validate_active_character(const TTable& table, player_id player, std::string_view command)
    {
        debug_validate_entity(table, player, command, "player");
        const auto active = table[player].state().active_character;
        if(not active)
            throw command_input_error{ command, missing_entity_argument{ "active_character", command_entity_id{ player } } };
        debug_validate_entity(table, *active, command, "active_character");
    }
}
#endif

#endif
