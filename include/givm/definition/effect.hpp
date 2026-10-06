#ifndef GIVM_DEFINITION_EFFECT_HPP
#define GIVM_DEFINITION_EFFECT_HPP

#include <cstddef>
#include "../enums/event_category.hpp"

namespace givm
{
    class definition_compile_context;

    namespace detail
    {
        class execution_context;
        class program_input_validator;
        struct program_inputs_builder;

        inline constexpr std::size_t null_effect_position = 0;
    }

    template<event_category Category>
    class effect
    {
    public:
        constexpr effect() noexcept = default;

        [[nodiscard]] static constexpr effect null() noexcept
        {
            return {};
        }

        [[nodiscard]] constexpr bool is_null() const noexcept
        {
            return position_ == detail::null_effect_position;
        }

        [[nodiscard]] constexpr explicit operator bool() const noexcept
        {
            return not is_null();
        }

        friend constexpr bool operator==(effect, effect) noexcept = default;

    private:
        constexpr explicit effect(std::size_t position) noexcept
        : position_{ position }
        {}

        template<event_category Other>
        constexpr explicit effect(effect<Other> other) noexcept
        : position_{ other.position_ }
#ifndef NDEBUG
        , debug_index_{ other.debug_index_ }, library_identity_{ other.library_identity_ }
#endif
        {}

        std::size_t position_ = detail::null_effect_position;
#ifndef NDEBUG
        std::size_t debug_index_ = 0;
        std::size_t library_identity_ = 0;
#endif

        friend class definition_compile_context;
        friend class detail::execution_context;
        friend class detail::program_input_validator;
        template<event_category> friend class effect;
        friend struct detail::program_inputs_builder;
    };

    using normal_effect = effect<event_category::normal>;
    using immediate_effect = effect<event_category::immediate>;
    using preview_effect = effect<event_category::preview>;
}

#endif
