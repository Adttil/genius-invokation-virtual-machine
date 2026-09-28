#ifndef GIVM_TABLE_HISTORY_SUMMARY_HPP
#define GIVM_TABLE_HISTORY_SUMMARY_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>
#include <utility>
#include <variant>

#include "entity_fwd.hpp"
#include "../utils/debug.hpp"
#include "../utils/type_list.hpp"
#include "../macro_define.hpp"

namespace givm
{
    class definition_compile_context;
    class definition_library;

    struct history_summary_definition {};

    enum class history_value_type
    {
        boolean,
        i8,
        u8,
        i16,
        u16,
        i32,
        u32,
        i64,
        u64,
        f32,
        f64
    };

    namespace detail
    {
        using history_value_types = type_list<bool,
            std::int8_t, std::uint8_t, std::int16_t, std::uint16_t,
            std::int32_t, std::uint32_t, std::int64_t, std::uint64_t, float, double>;

        template<class T>
        concept history_value = requires { history_value_types::index_of<T>(); };

        template<history_value T>
        inline constexpr history_value_type history_value_type_of =
            static_cast<history_value_type>(history_value_types::index_of<T>());

        inline constexpr auto history_value_layouts = []
        {
            std::array<std::pair<std::size_t, std::size_t>, history_value_types::size()> result{};
            history_value_types::each([&]<class T>
            {
                result[history_value_types::index_of<T>()] = { sizeof(T), alignof(T) };
            });
            return result;
        }();

        constexpr std::size_t history_value_size(history_value_type type) noexcept
        {
            const auto index = static_cast<std::size_t>(type);
            return index < history_value_layouts.size() ? history_value_layouts[index].first : 0;
        }

        constexpr std::size_t history_value_alignment(history_value_type type) noexcept
        {
            const auto index = static_cast<std::size_t>(type);
            return index < history_value_layouts.size() ? history_value_layouts[index].second : 0;
        }

        template<class T>
        struct history_key_data
        {
            static_assert(history_value<T>);

            constexpr history_key_data() noexcept = default;

            constexpr history_key_data(std::size_t offset, [[maybe_unused]] std::size_t count) noexcept
            : offset_{ offset }
            {
                GIVM_ASSERT(count == 1);
            }

            constexpr std::size_t offset() const noexcept { return offset_; }
            constexpr std::size_t count() const noexcept { return 1; }

        private:
            std::size_t offset_ = static_cast<std::size_t>(-1);
        };

        template<history_value T>
        struct history_key_data<T[]>
        {
            constexpr history_key_data() noexcept = default;

            constexpr history_key_data(std::size_t offset, std::size_t count) noexcept
            : offset_{ offset }, count_{ count }
            {}

            constexpr std::size_t offset() const noexcept { return offset_; }
            constexpr std::size_t count() const noexcept { return count_; }

        private:
            std::size_t offset_ = static_cast<std::size_t>(-1);
            std::size_t count_ = 0;
        };

        template<class T, class TByte>
        inline decltype(auto) access_history_value(
            TByte* data, [[maybe_unused]] std::size_t size, std::size_t offset,
            [[maybe_unused]] std::size_t count) noexcept
        {
            using value_type = std::remove_extent_t<T>;
            static_assert(history_value<value_type>);
            using element_type = std::conditional_t<std::is_const_v<TByte>, const value_type, value_type>;
            GIVM_ASSERT(offset <= size && count <= (size - offset) / sizeof(value_type));
            GIVM_ASSERT(offset % alignof(value_type) == 0);
            if constexpr(std::is_unbounded_array_v<T>)
            {
                if(count == 0) return std::span<element_type>{};
                return std::span<element_type>{ reinterpret_cast<element_type*>(data + offset), count };
            }
            else
            {
                return *reinterpret_cast<element_type*>(data + offset);
            }
        }
    }

    // Offsets in this key are relative to one summary's region.
    template<class T>
    class history_field_key : private detail::history_key_data<T>
    {
        using base = detail::history_key_data<T>;
        friend class definition_compile_context;
        friend class definition_library;

    public:
        using value_type = T;

        constexpr history_field_key() noexcept = default;
        using base::offset;
        using base::count;

    private:
        constexpr explicit history_field_key(std::size_t offset, std::size_t count = 1) noexcept
        : base{ offset, count }
        {}
    };

    // Offsets in this key are relative to the table's complete history region.
    template<class T>
    class history_value_key : private detail::history_key_data<T>
    {
        using base = detail::history_key_data<T>;
        friend class definition_compile_context;
        friend class definition_library;

    public:
        using value_type = T;

        constexpr history_value_key() noexcept = default;
        using base::offset;
        using base::count;

    private:
        constexpr explicit history_value_key(std::size_t offset, std::size_t count = 1) noexcept
        : base{ offset, count }
        {}
    };

    namespace detail
    {
        template<class... T>
        using history_field_keys_for = std::variant<history_field_key<T>..., history_field_key<T[]>...>;

        template<class... T>
        using history_value_keys_for = std::variant<history_value_key<T>..., history_value_key<T[]>...>;
    }

    using dynamic_history_field = detail::history_value_types::apply<detail::history_field_keys_for>;
    using dynamic_history_value = detail::history_value_types::apply<detail::history_value_keys_for>;

    class history_summary_state
    {
        friend class detail::unrestricted_table;

    public:
        template<class T>
        decltype(auto) operator[](history_field_key<T> key) const noexcept
        {
            return detail::access_history_value<T>(data_, size_, key.offset(), key.count());
        }

    private:
        constexpr history_summary_state(unsigned char* data, std::size_t size) noexcept
        : data_{ data }, size_{ size }
        {}

        unsigned char* data_;
        std::size_t size_;
    };

}

#include "../macro_undef.hpp"
#endif
