#ifndef GIVM_DEFINITION_SOURCE_ERROR_HPP
#define GIVM_DEFINITION_SOURCE_ERROR_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../enums/definition_category.hpp"

namespace givm
{
    struct definition_name
    {
        definition_category category;
        std::string name;
    };

    struct source_conflict
    {
        enum class reason { different_object, different_type };

        definition_name definition;
        reason cause;
        std::optional<std::size_t> first_input_index;
        std::optional<std::size_t> second_input_index;
    };

    struct source_missing_dependency
    {
        definition_name source;
        std::size_t input_index;
        definition_name dependency;
    };

    using source_add_error = std::variant<source_conflict, source_missing_dependency>;

    struct source_selection_error
    {
        definition_name definition;
        std::optional<definition_name> required_by;
    };

    using source_preparation_error = std::variant<source_conflict, source_missing_dependency, source_selection_error>;

    namespace detail
    {
        std::string source_definition_name_text(const definition_name& definition);
    }

    std::string error_string(const source_conflict& error);

    std::string error_string(const source_missing_dependency& error);

    std::string error_string(const source_selection_error& error);

    std::string error_string(const std::vector<source_add_error>& errors);

    std::string error_string(const std::vector<source_conflict>& errors);

    std::string error_string(const std::vector<source_preparation_error>& errors);
}

#endif
