#ifndef GIVM_EXECUTOR_PROGRAM_INPUT_ERROR_HPP
#define GIVM_EXECUTOR_PROGRAM_INPUT_ERROR_HPP

#include <algorithm>
#include <array>
#include <exception>
#include <expected>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#ifndef NDEBUG
#include <atomic>
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
    enum class invalid_effect { null_entry, different_library, unknown_entry };
    struct repeated_program_invocation {};
    struct invalid_response_index { std::uint32_t index; };
    using program_input_error_reason = std::variant<program_input_count_mismatch, program_input_type_mismatch,
        invalid_effect, repeated_program_invocation, invalid_response_index>;

    inline std::string error_string(const program_input_count_mismatch& error)
    {
        return "input count mismatch: expected " + std::to_string(error.expected) + ", actual " + std::to_string(error.actual);
    }
    inline std::string error_string(const program_input_type_mismatch& error)
    {
        return "input[" + std::to_string(error.input_index) + "] for command[" + std::to_string(error.command_index)
            + "] " + error.command + ": expected " + error.expected + ", actual " + error.actual;
    }
    inline std::string error_string(invalid_effect error)
    {
        switch(error)
        {
        case invalid_effect::null_entry: return "invoke requires a non-null effect";
        case invalid_effect::different_library: return "effect belongs to a different definition library";
        case invalid_effect::unknown_entry: return "effect has no matching input description";
        }
        return {};
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
#ifndef NDEBUG
        std::size_t library_identity = 0;
#endif
        std::span<const debug_program_info> programs;
        std::span<const debug_input_requirement> inputs;
    };
    struct program_input_records
    {
        // Compilation owns these records; only Debug retains them in the library.
        std::vector<debug_input_requirement> inputs;
        std::vector<debug_program_info> programs;
#ifndef NDEBUG
        std::size_t library_identity = 0;
#endif

        program_debug_view view() const noexcept
        {
            return {
#ifndef NDEBUG
                library_identity,
#endif
                programs, inputs };
        }
    };
    struct program_input_failure
    {
        program_input_error_reason reason;
        const debug_program_info* program = nullptr;
    };
    class program_input_validator
    {
    public:
        explicit program_input_validator(program_debug_view debug) noexcept : debug_{ debug } {}

        template<event_category Category, class TMarker>
        std::expected<const debug_program_info*, program_input_failure> check_parameters(
            effect<Category> entry, std::size_t count, TMarker marker) const
        {
            if(not entry) return std::unexpected{ program_input_failure{ invalid_effect::null_entry } };
#ifndef NDEBUG
            if(entry.library_identity_ != debug_.library_identity)
                return std::unexpected{ program_input_failure{ invalid_effect::different_library } };
            if(entry.debug_index_ >= debug_.programs.size())
                return std::unexpected{ program_input_failure{ invalid_effect::unknown_entry } };
            const auto& program = debug_.programs[entry.debug_index_];
#else
            const auto found = std::ranges::find(debug_.programs, entry.position_, &debug_program_info::position);
            if(found == debug_.programs.end())
                return std::unexpected{ program_input_failure{ invalid_effect::unknown_entry } };
            const auto& program = *found;
#endif
            if(program.position != entry.position_ || program.inputs_begin > debug_.inputs.size()
                || program.inputs_count > debug_.inputs.size() - program.inputs_begin)
                return std::unexpected{ program_input_failure{ invalid_effect::unknown_entry } };
            if(program.inputs_count != count)
                return std::unexpected{ program_input_failure{
                    program_input_count_mismatch{ program.inputs_count, count }, &program } };
            for(std::size_t index = 0; index != count; ++index)
            {
                const auto& expected = debug_.inputs[program.inputs_begin + index];
                const auto actual = marker(index);
                if(expected.marker != actual)
                    return std::unexpected{ program_input_failure{ program_input_type_mismatch{
                        index, expected.command_index, std::string{ expected.command },
                        std::string{ expected.command } + "_input", actual < debug_input_command_names.size()
                            ? std::string{ debug_input_command_names[actual] } + "_input" : "unknown input" }, &program } };
            }
            return &program;
        }

        template<event_category Category>
        std::expected<const debug_program_info*, program_input_failure> check(
            effect<Category> entry, std::span<const program_input_description> inputs) const
        {
            auto result = check_parameters(entry, inputs.size(), [&](std::size_t index) { return inputs[index].marker; });
            if(not result) return result;
            for(const auto& input : inputs)
            {
                if(input.marker == command_input_types::index_of<return_response_input>())
                {
                    if(auto error = check_value(return_response_input{ input.response_index }))
                        return std::unexpected{ std::move(*error) };
                }
                else if(input.marker == command_input_types::index_of<defer_program_input>())
                {
                    auto child = check(input.entry, input.children);
                    if(not child) return child;
                }
            }
            return result;
        }

        template<class T>
        std::optional<program_input_failure> check_value(const T&) const noexcept { return {}; }

        std::optional<program_input_failure> check_value(const return_response_input& input) const
        {
            if(input.index == return_response::dynamic)
                return program_input_failure{ invalid_response_index{ input.index } };
            return {};
        }

        std::optional<program_input_failure> check_value(const fixed_defer_program_input& input) const
        {
            auto result = check(input.entry_, input.descriptions_);
            if(not result) return std::move(result.error());
            return {};
        }

#ifndef NDEBUG
        std::optional<program_input_failure> check_value(const defer_program_input& input) const
        {
            auto result = check(input.entry, input.inputs.descriptions());
            if(not result) return std::move(result.error());
            return {};
        }

        template<event_category Category, class TMarker>
        const debug_program_info& validate_parameters(effect<Category> entry, std::size_t count, TMarker marker) const
        {
            return checked(check_parameters(entry, count, marker));
        }

        template<event_category Category>
        const debug_program_info& validate(effect<Category> entry, std::span<const program_input_description> inputs) const
        {
            return checked(check(entry, inputs));
        }

        template<class T>
        void validate_value(const T& input) const
        {
            if(auto error = check_value(input)) throw_error(std::move(*error));
        }
#endif

    private:
#ifndef NDEBUG
        [[noreturn]] static void throw_error(program_input_failure error)
        {
            if(error.program)
                throw program_input_error{ std::move(error.reason), error.program->source, error.program->program_index };
            throw program_input_error{ std::move(error.reason) };
        }

        static const debug_program_info& checked(std::expected<const debug_program_info*, program_input_failure> result)
        {
            if(not result) throw_error(std::move(result.error()));
            return **result;
        }
#endif

        program_debug_view debug_;
    };

#ifndef NDEBUG
    inline std::atomic_size_t next_program_library_identity{ 1 };
#endif
}

#endif
