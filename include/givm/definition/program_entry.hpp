#ifndef GIVM_DEFINITION_PROGRAM_ENTRY_HPP
#define GIVM_DEFINITION_PROGRAM_ENTRY_HPP

#include <cstddef>

namespace givm
{
    class definition_compile_context;

    namespace detail
    {
        class execution_context;
        class program_input_validator;

        inline constexpr std::size_t null_program_position = 0;
    }

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
#ifndef NDEBUG
        std::size_t debug_index_ = 0;
        std::size_t library_identity_ = 0;
#endif

        friend class definition_compile_context;
        friend class detail::execution_context;
        friend class detail::program_input_validator;
    };
}

#endif
