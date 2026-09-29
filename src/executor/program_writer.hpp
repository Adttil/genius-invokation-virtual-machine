#ifndef GIVM_PRIVATE_EXECUTOR_PROGRAM_WRITER_HPP
#define GIVM_PRIVATE_EXECUTOR_PROGRAM_WRITER_HPP

#include <cstddef>
#include <cstring>
#include <type_traits>

#include <givm/executor/instruction.hpp>
#include <givm/utils/debug.hpp>
#include <givm/macro_define.hpp>

namespace givm::detail
{
    constexpr std::size_t align_program_size(std::size_t size) noexcept
    {
        return (size + program_alignment - 1) / program_alignment * program_alignment;
    }

    template<class T>
    inline constexpr std::size_t padded_size = align_program_size(sizeof(T));

    template<std::size_t N, class T>
    inline constexpr std::size_t instruction_extent = N * sizeof(execute_fn) + padded_size<T>;

    class program_writer
    {
    public:
        explicit program_writer(program_bytes& bytes) noexcept : bytes_{ bytes }
        {
            GIVM_ASSERT(bytes_.size() % program_alignment == 0);
        }

        std::size_t position() const noexcept { return bytes_.size(); }

        template<class T>
        std::size_t write(const T& value)
        {
            static_assert(std::is_trivially_copyable_v<T>);
            static_assert(alignof(T) <= program_alignment);
#if defined(__cpp_lib_is_implicit_lifetime)
            static_assert(std::is_implicit_lifetime_v<T>);
#endif
            const auto start = position();
            bytes_.resize(start + padded_size<T>);
            std::memcpy(bytes_.data() + start, &value, sizeof(T));
            return start;
        }

    private:
        program_bytes& bytes_;
    };
}

#include <givm/macro_undef.hpp>
#endif
