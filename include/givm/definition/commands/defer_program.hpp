#ifndef GIVM_DEFINITION_COMMANDS_DEFER_PROGRAM_HPP
#define GIVM_DEFINITION_COMMANDS_DEFER_PROGRAM_HPP

#include <string>
#include "../program_inputs.hpp"

namespace givm
{
    struct defer_program_error
    {
        enum class reason { dynamic_input_in_root };
        reason cause;
    };

    inline std::string error_string(const defer_program_error&)
    {
        return "defer_program: cannot consume dynamic input in a root program";
    }

    struct defer_program_input
    {
        normal_effect entry;
        program_inputs inputs;
    };

    class fixed_defer_program_input
    {
    public:
        fixed_defer_program_input() noexcept = default;

        normal_effect entry() const noexcept { return entry_; }
        std::span<const unsigned char> bytes() const noexcept { return inputs_.bytes(); }

    private:
        normal_effect entry_;
        program_inputs inputs_;
        std::vector<detail::program_input_description> descriptions_;

        friend struct detail::program_inputs_builder;
        friend class detail::program_input_validator;
    };

    struct defer_program
    {
        using error_type = defer_program_error;
        using input_type = defer_program_input;

        fixed_defer_program_input input{};
    };
}

#endif
