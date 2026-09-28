#ifndef GIVM_EXECUTOR_HISTORY_ACCESS_ERROR_HPP
#define GIVM_EXECUTOR_HISTORY_ACCESS_ERROR_HPP

#include <exception>
#include <string>
#include <utility>
#include <variant>

#include "compile_error.hpp"

namespace givm
{
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

#endif
