#ifndef GIVM_DEFINITION_PROGRAM_INPUTS_HPP
#define GIVM_DEFINITION_PROGRAM_INPUTS_HPP

#include <cstddef>
#include <cstdint>
#include <span>
#ifndef NDEBUG
#include <vector>
#endif

#include "program_entry.hpp"
#include "../utils/stack.hpp"

namespace givm::detail
{
    struct program_inputs_builder;

#ifndef NDEBUG
    struct program_input_description
    {
        std::size_t marker;
        std::uint32_t response_index = 0;
        program_entry entry;
        std::vector<program_input_description> children;
    };
#endif

    struct deferred_program_input
    {
        program_entry entry;
        std::size_t input_bytes;
    };
}

namespace givm
{
    class program_inputs
    {
    public:
        program_inputs() noexcept = default;
        program_inputs(const program_inputs&) = default;
        program_inputs(program_inputs&&) noexcept = default;
        program_inputs& operator=(const program_inputs&) = default;
        program_inputs& operator=(program_inputs&&) noexcept = default;
        ~program_inputs() = default;

        std::span<const unsigned char> bytes() const noexcept
        {
            return { frames_.data(), frames_.size() };
        }

#ifndef NDEBUG
        std::span<const detail::program_input_description> descriptions() const noexcept
        {
            return descriptions_;
        }
#endif

    private:
        friend struct detail::program_inputs_builder;
        frame_stack frames_;
#ifndef NDEBUG
        std::vector<detail::program_input_description> descriptions_;
#endif
    };
}

#endif
