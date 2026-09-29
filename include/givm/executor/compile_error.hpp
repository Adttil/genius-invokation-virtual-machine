#ifndef GIVM_EXECUTOR_COMPILE_ERROR_HPP
#define GIVM_EXECUTOR_COMPILE_ERROR_HPP

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../definition_common.hpp"
#include "history_access_error.hpp"

namespace givm
{
    enum class program_kind { initialization, round, response };
    enum class compile_stage { source_selection, history_layout, definition, program };

    struct compile_location
    {
        compile_stage stage;
        std::optional<definition_name> source;
        std::optional<program_kind> program;
        std::optional<std::size_t> program_index;
        std::optional<std::size_t> command_index;
    };

    struct definition_resolution_error
    {
        enum class reason { undeclared_dependency, not_found };
        definition_name definition;
        reason cause;
    };

    struct history_field_empty_name { std::size_t field_index; };
    struct history_field_duplicate_name
    {
        std::string field;
        std::size_t first_index;
        std::size_t repeated_index;
    };
    struct history_field_layout_overflow
    {
        std::string field;
        std::size_t field_index;
        std::size_t count;
        std::size_t element_size;
        std::size_t alignment;
        std::size_t preceding_size;
    };
    struct history_storage_layout_overflow
    {
        std::size_t preceding_size;
        std::size_t summary_size;
        std::size_t alignment;
    };
    struct history_field_access_error
    {
        enum class reason { layouts_unavailable, no_current_summary };
        std::string summary;
        std::string field;
        reason cause;
    };

    namespace detail
    {
        template<class... T>
        using command_error_types_for = type_list<typename T::error_type...>;
        using command_error_types = command_types::apply<command_error_types_for>;
        using common_compile_error_types = type_list<source_conflict, source_missing_dependency, source_selection_error,
            definition_resolution_error, definition_metadata_error, history_field_empty_name, history_field_duplicate_name,
            history_field_layout_overflow, history_storage_layout_overflow, history_field_access_error,
            history_field_not_found, history_field_type_mismatch>;
    }

    using compile_error_reason = type_list_cat<detail::common_compile_error_types, detail::command_error_types>::apply<std::variant>;
    struct compile_error
    {
        compile_location location;
        compile_error_reason reason;
    };

    inline std::string error_string(const definition_resolution_error& error)
    {
        return std::string{ error.cause == definition_resolution_error::reason::undeclared_dependency
            ? "undeclared dependency: " : "definition not found: " } + detail::source_definition_name_text(error.definition);
    }
    inline std::string error_string(const history_field_empty_name& error)
    {
        return "empty history field name at field[" + std::to_string(error.field_index) + "]";
    }
    inline std::string error_string(const history_field_duplicate_name& error)
    {
        return "duplicate history field \"" + error.field + "\": field[" + std::to_string(error.first_index)
            + "] and field[" + std::to_string(error.repeated_index) + "]";
    }
    inline std::string error_string(const history_field_layout_overflow& error)
    {
        return "history field layout overflow: field[" + std::to_string(error.field_index) + "] \"" + error.field
            + "\", count=" + std::to_string(error.count) + ", element_size=" + std::to_string(error.element_size)
            + ", alignment=" + std::to_string(error.alignment) + ", preceding_size=" + std::to_string(error.preceding_size);
    }
    inline std::string error_string(const history_storage_layout_overflow& error)
    {
        return "history storage layout overflow: preceding_size=" + std::to_string(error.preceding_size)
            + ", summary_size=" + std::to_string(error.summary_size) + ", alignment=" + std::to_string(error.alignment);
    }
    inline std::string error_string(const history_field_access_error& error)
    {
        return std::string{ error.cause == history_field_access_error::reason::layouts_unavailable
            ? "history layouts unavailable: " : "no current history summary: " } + error.summary + "." + error.field;
    }
    inline std::string error_string(const compile_error& error)
    {
        constexpr std::string_view stages[]{ "source selection", "history layout", "definition compilation", "program compilation" };
        std::string result{ stages[static_cast<std::size_t>(error.location.stage)] };
        if(error.location.source) result += ": " + detail::source_definition_name_text(*error.location.source);
        if(error.location.program)
        {
            constexpr std::string_view kinds[]{ "initialization", "round", "response" };
            result += "; ";
            result += kinds[static_cast<std::size_t>(*error.location.program)];
        }
        if(error.location.program_index) result += " program[" + std::to_string(*error.location.program_index) + "]";
        if(error.location.command_index) result += " command[" + std::to_string(*error.location.command_index) + "]";
        result += ": ";
        result += std::visit([](const auto& reason) { return error_string(reason); }, error.reason);
        return result;
    }

    inline std::string error_string(const std::vector<compile_error>& errors)
    {
        std::string result;
        for(const auto& error : errors)
        {
            if(not result.empty()) result += '\n';
            result += error_string(error);
        }
        return result;
    }
}

#endif
