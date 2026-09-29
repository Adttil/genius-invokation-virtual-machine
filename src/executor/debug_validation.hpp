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

#include <givm/definition.hpp>
#include <givm/table.hpp>
#include <givm/executor/command_input_error.hpp>
#include <givm/executor/library.hpp>
#include <givm/executor/views/action_selection.hpp>

namespace givm::detail
{
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
