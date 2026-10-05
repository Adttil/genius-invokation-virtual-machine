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
    struct invalid_response_index { std::uint32_t index; };
    using program_input_error_reason = std::variant<program_input_count_mismatch, program_input_type_mismatch,
        invalid_program_entry, program_invocation_mode_mismatch, repeated_program_invocation, invalid_response_index>;

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
    inline std::string error_string(const invalid_response_index& error)
    {
        return "return_response input cannot contain the dynamic marker: " + std::to_string(error.index);
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
    class program_input_validator
    {
    public:
        explicit program_input_validator(program_debug_view debug) noexcept : debug_{ debug } {}

        template<class TMarker>
        const debug_program_info& validate_parameters(program_entry entry, std::size_t count, TMarker marker) const
        {
            if(not entry) throw program_input_error{ invalid_program_entry::null_entry };
            if(entry.library_identity_ != debug_.library_identity)
                throw program_input_error{ invalid_program_entry::different_library };
            if(entry.debug_index_ >= debug_.programs.size())
                throw program_input_error{ invalid_program_entry::unknown_entry };
            const auto& program = debug_.programs[entry.debug_index_];
            if(program.position != entry.position_ || program.inputs_begin > debug_.inputs.size()
                || program.inputs_count > debug_.inputs.size() - program.inputs_begin)
                throw program_input_error{ invalid_program_entry::unknown_entry };
            const auto fail = [&](program_input_error_reason reason)
            {
                throw program_input_error{ std::move(reason), program.source, program.program_index };
            };
            if(program.inputs_count != count)
                fail(program_input_count_mismatch{ program.inputs_count, count });
            for(std::size_t index = 0; index != count; ++index)
            {
                const auto& expected = debug_.inputs[program.inputs_begin + index];
                const auto actual = marker(index);
                if(expected.marker != actual)
                    fail(program_input_type_mismatch{ index, expected.command_index, std::string{ expected.command },
                        std::string{ expected.command } + "_input", actual < debug_input_command_names.size()
                            ? std::string{ debug_input_command_names[actual] } + "_input" : "unknown input" });
            }
            return program;
        }

        const debug_program_info& validate(program_entry entry, std::span<const program_input_description> inputs) const
        {
            const auto& program = validate_parameters(entry, inputs.size(), [&](std::size_t index) { return inputs[index].marker; });
            for(const auto& input : inputs)
            {
                if(input.marker == command_input_types::index_of<return_response_input>())
                    validate_value(return_response_input{ input.response_index });
                else if(input.marker == command_input_types::index_of<defer_program_input>())
                    validate(input.entry, input.children);
            }
            return program;
        }

        template<class T>
        void validate_value(const T&) const noexcept {}

        void validate_value(const return_response_input& input) const
        {
            if(input.index == return_response::dynamic)
                throw program_input_error{ invalid_response_index{ input.index } };
        }

        void validate_value(const defer_program_input& input) const
        {
            validate(input.entry, input.inputs.descriptions());
        }

    private:
        program_debug_view debug_;
    };

    inline std::atomic_size_t next_program_library_identity{ 1 };
}
#endif

#endif
