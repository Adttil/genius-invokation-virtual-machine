#ifndef GIVM_TABLE_ISSUED_ID_HPP
#define GIVM_TABLE_ISSUED_ID_HPP

#include <cstddef>
#include <string_view>

namespace givm::detail
{
    struct table_accessor;
}

namespace givm
{
    template<class TTag>
    class issued_id
    {
        friend class issued_id_map;
        friend struct detail::table_accessor;

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

        friend constexpr bool operator==(issued_id, issued_id) noexcept = default;

    private:
        constexpr explicit issued_id(size_t value) noexcept
        : value_{ value }
        {}

        size_t value_ = invalid_value;
    };

    struct card_definition{};
    struct status_definition{};

    template<class TEntityView>
    using definition_id = issued_id<TEntityView>;

    using tag_id = issued_id<std::string_view>;
}

#endif
