#ifndef GIVM_TABLE_TAG_ID_HPP
#define GIVM_TABLE_TAG_ID_HPP

#include <cstddef>
#ifndef NDEBUG
#include <stdexcept>
#endif

namespace givm
{
    class tag_id
    {
    public:
        constexpr tag_id() noexcept = default;

        constexpr explicit tag_id(std::size_t value) : value_{ value }
        {
#ifndef NDEBUG
            if(value == static_cast<std::size_t>(-1))
                throw std::invalid_argument{ "tag ID contains the empty tag value" };
#endif
        }

        constexpr tag_id& operator=(std::size_t value)
        {
            *this = tag_id{ value };
            return *this;
        }

        constexpr std::size_t value() const noexcept { return value_; }

        friend constexpr bool operator==(tag_id, tag_id) noexcept = default;

    private:
        std::size_t value_;
    };

    class optional_tag_id
    {
        static constexpr std::size_t null_value = static_cast<std::size_t>(-1);

    public:
        constexpr optional_tag_id() noexcept = default;
        constexpr optional_tag_id(std::nullptr_t) noexcept {}
        constexpr optional_tag_id(tag_id id) noexcept : value_{ id.value() } {}

        constexpr explicit operator bool() const noexcept { return value_ != null_value; }
        constexpr bool has_value() const noexcept { return static_cast<bool>(*this); }

        constexpr tag_id get() const
        {
#ifndef NDEBUG
            if(not *this) throw std::invalid_argument{ "tag ID is empty" };
#endif
            return tag_id{ value_ };
        }

        constexpr tag_id operator*() const { return get(); }
        constexpr std::size_t value() const noexcept { return value_; }

        friend constexpr bool operator==(optional_tag_id, optional_tag_id) noexcept = default;

    private:
        std::size_t value_ = null_value;
    };
}

#endif
