#ifndef GIVM_EXECUTOR_INSTRUCTION_HPP
#define GIVM_EXECUTOR_INSTRUCTION_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>

namespace givm
{
    enum class execution_state : std::uint8_t;
    enum class compile_mode : std::uint8_t { normal, observed };
    class definition_library;
    class random_fn;
}

namespace givm::detail
{
    class unrestricted_table;
    class execution_context;

    using execution_position = std::size_t;

    using execute_fn = execution_state (*)(
        const definition_library&, unrestricted_table&, execution_context&, random_fn&
    );

    inline constexpr execution_position entry_position = sizeof(execute_fn);
    inline constexpr std::size_t program_alignment = alignof(execute_fn);

    // Keep the byte buffer aligned for every supported instruction payload.
    // Other element types support allocator rebinding used by debug containers.
    template<class T>
    struct program_allocator
    {
        using value_type = T;
        using is_always_equal = std::true_type;
        template<class U> struct rebind { using other = program_allocator<U>; };

        constexpr program_allocator() noexcept = default;
        template<class U> constexpr program_allocator(const program_allocator<U>&) noexcept {}

        T* allocate(std::size_t count)
        {
            if constexpr(std::is_same_v<T, unsigned char>)
            {
                // operator new implicitly creates the array; vector constructs
                // its elements using the ordinary allocator_traits defaults.
                return static_cast<T*>(::operator new(count, std::align_val_t{ program_alignment }));
            }
            else
            {
                return std::allocator<T>{}.allocate(count);
            }
        }

        void deallocate(T* storage, std::size_t count) noexcept
        {
            if constexpr(std::is_same_v<T, unsigned char>)
            {
                ::operator delete(storage, std::align_val_t{ program_alignment });
            }
            else
            {
                std::allocator<T>{}.deallocate(storage, count);
            }
        }

        template<class U>
        constexpr bool operator==(const program_allocator<U>&) const noexcept { return true; }
    };

    using program_bytes = std::vector<unsigned char, program_allocator<unsigned char>>;

    inline void finalize_program(program_bytes& bytes) noexcept
    {
        if(not bytes.empty())
        {
            // Vector construction/relocation operates on byte objects. At the
            // immutable boundary, memmove implicitly creates the instruction
            // objects while preserving their complete object representations.
            // A self move requires no actual transfer of bytes.
            std::memmove(bytes.data(), bytes.data(), bytes.size());
        }
    }

}

#endif
