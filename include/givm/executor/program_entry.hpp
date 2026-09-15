#ifndef GIVM_EXECUTOR_PROGRAM_ENTRY_HPP
#define GIVM_EXECUTOR_PROGRAM_ENTRY_HPP

#include <cstddef>
#include "instruction.hpp"

namespace givm
{
    class definition_compile_context;

    namespace detail
    {
        class execution_context;

        using execution_position = std::size_t;
        inline constexpr execution_position null_program_position = 0;
        inline constexpr execution_position entry_position = sizeof(execute_fn);
    }

    template<class TContext>
    class program_entry
    {
    public:
        constexpr program_entry() noexcept = default;

        [[nodiscard]] static constexpr program_entry null() noexcept
        {
            return {};
        }

        [[nodiscard]] constexpr bool is_null() const noexcept
        {
            return position_ == detail::null_program_position;
        }

        [[nodiscard]] constexpr explicit operator bool() const noexcept
        {
            return not is_null();
        }

        friend constexpr bool operator==(program_entry, program_entry) noexcept = default;

    private:
        constexpr explicit program_entry(std::size_t position) noexcept
        : position_{ position }
        {}

        std::size_t position_ = detail::null_program_position;

        friend class definition_compile_context;
        friend class detail::execution_context;
    };
}

#endif
