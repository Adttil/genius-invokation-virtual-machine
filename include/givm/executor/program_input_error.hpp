#ifndef GIVM_EXECUTOR_PROGRAM_INPUT_ERROR_HPP
#define GIVM_EXECUTOR_PROGRAM_INPUT_ERROR_HPP

#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#ifndef NDEBUG
#include <array>
#include <atomic>
#include <span>
#endif

#include "../definition_common.hpp"

namespace givm
{
    struct program_input_count_mismatch { std::size_t expected; std::size_t actual; };
    struct program_input_type_mismatch
    {
        std::size_t input_index;
        std::size_t command_index;
        std::string command;
        std::string expected;
        std::string actual;
    };
    enum class invalid_program_entry { null_entry, different_library, unknown_entry };
    struct program_invocation_mode_mismatch { bool expected_substack; bool actual_substack; };
    struct repeated_program_invocation {};
    using program_input_error_reason = std::variant<program_input_count_mismatch, program_input_type_mismatch,
        invalid_program_entry, program_invocation_mode_mismatch, repeated_program_invocation>;

    inline std::string error_string(const program_input_count_mismatch& error)
    {
        return "input count mismatch: expected " + std::to_string(error.expected) + ", actual " + std::to_string(error.actual);
    }
    inline std::string error_string(const program_input_type_mismatch& error)
    {
        return "input[" + std::to_string(error.input_index) + "] for command[" + std::to_string(error.command_index)
            + "] " + error.command + ": expected " + error.expected + ", actual " + error.actual;
    }
    inline std::string error_string(invalid_program_entry error)
    {
        switch(error)
        {
        case invalid_program_entry::null_entry: return "invoke requires a non-null program entry";
        case invalid_program_entry::different_library: return "program entry belongs to a different definition library";
        case invalid_program_entry::unknown_entry: return "program entry has no matching input description";
        }
        return {};
    }
    inline std::string error_string(const program_invocation_mode_mismatch& error)
    {
        return std::string{ "invocation mode mismatch: expected " } + (error.expected_substack ? "invoke(substack_t{}, ...)" : "invoke(...)")
            + ", actual " + (error.actual_substack ? "invoke(substack_t{}, ...)" : "invoke(...)");
    }
    inline std::string error_string(repeated_program_invocation)
    {
        return "a response may invoke a program only once";
    }

    class program_input_error : public std::exception
    {
    public:
        const std::optional<definition_name> source;
        const std::optional<std::size_t> program_index;
        const program_input_error_reason reason;

        program_input_error(program_input_error_reason cause, std::optional<definition_name> definition = {}, std::optional<std::size_t> program = {})
        : source{ std::move(definition) }, program_index{ program }, reason{ std::move(cause) }, message_{ format() }
        {}

        const char* what() const noexcept override { return message_.c_str(); }

    private:
        std::string format() const
        {
            std::string result;
            if(source) result = detail::source_definition_name_text(*source) + "; ";
            if(program_index) result += "response program[" + std::to_string(*program_index) + "]: ";
            result += std::visit([](const auto& error) { return error_string(error); }, reason);
            return result;
        }
        std::string message_;
    };

    inline std::string error_string(const program_input_error& error) { return error.what(); }
}

#ifndef NDEBUG
namespace givm::detail
{
    extern const std::array<std::string_view, command_input_types::size()> debug_input_command_names;

    struct debug_input_requirement
    {
        std::size_t marker;
        std::size_t command_index;
        std::string_view command;
    };
    struct debug_program_info
    {
        std::optional<definition_name> source;
        std::size_t program_index;
        std::size_t position;
        std::size_t inputs_begin;
        std::size_t inputs_count;
    };
    struct program_debug_view
    {
        std::size_t library_identity = 0;
        std::span<const debug_program_info> programs;
        std::span<const debug_input_requirement> inputs;
    };
    inline std::atomic_size_t next_program_library_identity{ 1 };
}
#endif

#endif
