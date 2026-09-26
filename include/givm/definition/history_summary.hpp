#ifndef GIVM_DEFINITION_HISTORY_SUMMARY_HPP
#define GIVM_DEFINITION_HISTORY_SUMMARY_HPP

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "../table/history_summary.hpp"

namespace givm
{
    struct history_field_descriptor
    {
        std::string name;
        history_value_type type = history_value_type::u8;
        bool is_array = false;
        std::size_t count = 1;
    };

    using history_summary_layout = std::vector<history_field_descriptor>;

    template<detail::history_value T>
    inline history_field_descriptor history_field(std::string_view name)
    {
        return { std::string{ name }, detail::history_value_type_of<T>, false, 1 };
    }

    template<detail::history_value T>
    inline history_field_descriptor history_array(std::string_view name, std::size_t count)
    {
        return { std::string{ name }, detail::history_value_type_of<T>, true, count };
    }
}

#endif
