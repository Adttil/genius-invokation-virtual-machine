#ifndef GIVM_DEFINITION_ISSUED_ID_HPP
#define GIVM_DEFINITION_ISSUED_ID_HPP

#include <cstddef>
#include <string_view>

namespace givm
{
    class issued_id_map;

    template<class TTag>
    class issued_id
    {
        friend class issued_id_map;

    public:
        static constexpr size_t invalid_value = static_cast<size_t>(-1);

        constexpr issued_id() noexcept = default;

        constexpr bool is_valid() const noexcept
        {
            return value_ != invalid_value;
        }

        constexpr explicit operator bool() const noexcept
        {
            return is_valid();
        }

        constexpr size_t value() const noexcept
        {
            return value_;
        }

        constexpr void set_invalid() noexcept
        {
            value_ = invalid_value;
        }

    private:
        constexpr explicit issued_id(size_t value) noexcept
        : value_{ value }
        {}

        size_t value_ = invalid_value;
    };

    template<class TEntityView>
    using definition_id = issued_id<TEntityView>;

    using tag_id = issued_id<std::string_view>;
}

#endif
