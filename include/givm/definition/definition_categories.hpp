#ifndef GIVM_DEFINITION_DEFINITION_CATEGORIES_HPP
#define GIVM_DEFINITION_DEFINITION_CATEGORIES_HPP

#include "../table.hpp"
#include "../utils/type_list.hpp"
#include <array>

namespace givm::detail
{
    inline constexpr auto definition_categories = []
    {
        std::array<definition_category, static_cast<std::size_t>(definition_category::null)> result{};
        for(std::size_t i = 0; i != result.size(); ++i)
            result[i] = static_cast<definition_category>(i);
        return result;
    }();

    template<definition_category Category>
    using definition_views = decltype([]<std::size_t... I>(std::index_sequence<I...>)
    {
        return type_list<entity_view<entity_categories_of<Category>[I]>...>{};
    }(std::make_index_sequence<entity_categories_of<Category>.size()>{}));
}

#endif
