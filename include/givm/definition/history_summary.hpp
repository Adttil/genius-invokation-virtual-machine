#ifndef GIVM_DEFINITION_HISTORY_SUMMARY_HPP
#define GIVM_DEFINITION_HISTORY_SUMMARY_HPP

#include <cstddef>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "../table.hpp"

namespace givm
{
    template<detail::history_value T>
    struct history_scalar_field
    {
        using value_type = T;

        std::string name;
    };

    template<detail::history_value T>
    struct history_array_field
    {
        using value_type = T[];

        std::string name;
        std::size_t count = 0;
    };

    namespace detail
    {
        template<class... T>
        using history_field_descriptors_for = std::variant<history_scalar_field<T>..., history_array_field<T>...>;
    }

    using history_field_descriptor = detail::history_value_types::apply<detail::history_field_descriptors_for>;
    using history_summary_layout = std::vector<history_field_descriptor>;

    template<detail::history_value T>
    inline history_scalar_field<T> history_field(std::string_view name)
    {
        return { std::string{ name } };
    }

    template<detail::history_value T>
    inline history_array_field<T> history_array(std::string_view name, std::size_t count)
    {
        return { std::string{ name }, count };
    }
}

#endif
