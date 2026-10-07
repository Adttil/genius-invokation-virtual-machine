#ifndef GIVM_EXECUTOR_HISTORY_ACCESS_ERROR_HPP
#define GIVM_EXECUTOR_HISTORY_ACCESS_ERROR_HPP

#include <cstddef>
#include <exception>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

#include "../table.hpp"

namespace givm
{
    struct definition_metadata_error
    {
        definition_category category;
        std::uint64_t value;
        std::size_t count;
    };

    struct history_field_not_found { std::string summary; std::string field; };
    struct history_field_type_mismatch
    {
        std::string summary;
        std::string field;
        std::string expected_type;
        std::string actual_type;
    };

    inline std::string error_string(const definition_metadata_error& error)
    {
        return "definition metadata ID out of range: category[" + std::to_string(static_cast<unsigned>(error.category))
            + "], value=" + std::to_string(error.value) + ", definition_count=" + std::to_string(error.count);
    }
    inline std::string error_string(const history_field_not_found& error)
    {
        return "history field not found: " + error.summary + "." + error.field;
    }
    inline std::string error_string(const history_field_type_mismatch& error)
    {
        return "history field type mismatch: " + error.summary + "." + error.field
            + ", expected " + error.expected_type + ", actual " + error.actual_type;
    }

    using history_access_error_reason = std::variant<definition_metadata_error, history_field_not_found, history_field_type_mismatch>;

    class history_access_error : public std::exception
    {
    public:
        const history_access_error_reason reason;

        explicit history_access_error(history_access_error_reason cause)
        : reason{ std::move(cause) }, message_{ std::visit([](const auto& error) { return error_string(error); }, reason) }
        {}

        const char* what() const noexcept override { return message_.c_str(); }

    private:
        std::string message_;
    };

    inline std::string error_string(const history_access_error& error) { return error.what(); }
}

namespace givm::detail
{
    template<class T>
    inline std::string history_type_name()
    {
        constexpr std::string_view names[]{ "bool", "int8_t", "uint8_t", "int16_t", "uint16_t",
            "int32_t", "uint32_t", "int64_t", "uint64_t", "float", "double" };
        auto result = std::string{ names[history_value_types::index_of<std::remove_extent_t<T>>()] };
        if constexpr(std::is_unbounded_array_v<T>) result += "[]";
        return result;
    }
}

#endif
