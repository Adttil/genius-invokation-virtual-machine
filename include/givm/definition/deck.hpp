#ifndef GIVM_DEFINITION_DECK_HPP
#define GIVM_DEFINITION_DECK_HPP

#include <cstddef>
#include <expected>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "../table.hpp"
#include "issued_id_map.hpp"

namespace givm
{
    struct deck_link_error
    {
        enum class definition_kind { card, character };

        definition_kind kind;
        std::size_t index;
        std::string name;
    };

    inline std::string error_string(const deck_link_error& error)
    {
        const auto kind = error.kind == deck_link_error::definition_kind::card ? "card" : "character";
        return std::string{ "unavailable " } + kind + " definition at " + kind + "_names["
            + std::to_string(error.index) + "]: " + error.name;
    }

    inline std::string error_string(const std::vector<deck_link_error>& errors)
    {
        std::string result;
        for(const auto& error : errors)
        {
            if(not result.empty()) result += '\n';
            result += error_string(error);
        }
        return result;
    }

    template<class TCardNames, class TCharacterNames>
    inline std::expected<linked_deck, std::vector<deck_link_error>> link_deck(
        const issued_id_map& id_map,
        TCardNames&& card_names,
        TCharacterNames&& character_names
    )
    {
        linked_deck result;
        std::vector<deck_link_error> errors;
        std::size_t index = 0;
        for(auto&& item : card_names)
        {
            const std::string_view name{ item };
            if(not id_map.has<definition_category::card>(name))
                errors.push_back({ deck_link_error::definition_kind::card, index, std::string{ name } });
            else
                result.cards.push_back(id_map.get_id<definition_category::card>(name));
            ++index;
        }
        index = 0;
        for(auto&& item : character_names)
        {
            const std::string_view name{ item };
            if(not id_map.has<definition_category::character>(name))
                errors.push_back({ deck_link_error::definition_kind::character, index, std::string{ name } });
            else
                result.characters.push_back(id_map.get_id<definition_category::character>(name));
            ++index;
        }
        if(not errors.empty()) return std::unexpected{ std::move(errors) };
        return result;
    }
}

#endif
