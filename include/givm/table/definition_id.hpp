#ifndef GIVM_TABLE_DEFINITION_ID_HPP
#define GIVM_TABLE_DEFINITION_ID_HPP

#include <cstddef>
#include <cstdint>
#ifndef NDEBUG
#include <stdexcept>
#endif
#include <type_traits>
#include <utility>

#include "../enums/definition_category.hpp"

namespace givm
{
    template<definition_category Category>
    class definition_id
    {
        static_assert(Category < definition_category::null);

    public:
        static constexpr definition_category category = Category;

        constexpr definition_id() noexcept = default;

        constexpr explicit definition_id(std::uint64_t value)
        : value_{ value }
        {
#ifndef NDEBUG
            if(value > detail::definition_index_mask)
                throw std::invalid_argument{ "definition ID index exceeds its bit width" };
#endif
        }

        constexpr definition_id& operator=(std::uint64_t value)
        {
            *this = definition_id{ value };
            return *this;
        }

        constexpr std::uint64_t value() const noexcept { return value_; }

        friend constexpr bool operator==(definition_id, definition_id) noexcept = default;

    private:
        std::uint64_t value_;
    };

    template<definition_category... Categories>
    class variant_definition_id
    {
        static_assert(sizeof...(Categories) != 0);
        static_assert(((Categories <= definition_category::null) && ...));
        static constexpr bool nullable = ((Categories == definition_category::null) || ...);
        static constexpr std::size_t non_null_count = ((Categories != definition_category::null ? 1u : 0u) + ...);
        template<definition_category Category>
        static constexpr bool contains = ((Category == Categories) || ...);
        static constexpr std::uint64_t null_value =
            std::uint64_t(definition_category::null) << detail::definition_id_bit_width;

    public:
        constexpr variant_definition_id() noexcept = default;
        constexpr variant_definition_id(std::nullptr_t) noexcept requires nullable : storage_{ null_value } {}

        template<definition_category Category> requires ((Category == Categories) || ...)
        constexpr variant_definition_id(definition_id<Category> id) noexcept
        : storage_{ id.value() | (std::uint64_t(Category) << detail::definition_id_bit_width) }
        {}

        template<definition_category... Other> requires ((contains<Other>) && ...)
        constexpr variant_definition_id(variant_definition_id<Other...> id) noexcept
        : storage_{ id.value() }
        {}

        constexpr explicit variant_definition_id(std::uint64_t value) : storage_{ value }
        {
#ifndef NDEBUG
            if(not ((category() == Categories) || ...) || (category() == definition_category::null && value != null_value))
                throw std::invalid_argument{ "definition ID has an invalid category or null representation" };
#endif
        }

        constexpr variant_definition_id& operator=(std::uint64_t value)
        {
            *this = variant_definition_id{ value };
            return *this;
        }

        constexpr std::uint64_t value() const noexcept { return storage_.value; }
        constexpr definition_category category() const noexcept
        {
            return static_cast<definition_category>(storage_.value >> detail::definition_id_bit_width);
        }
        constexpr explicit operator bool() const noexcept requires nullable
        {
            return category() != definition_category::null;
        }

        constexpr bool has_value() const noexcept requires nullable { return static_cast<bool>(*this); }

        template<definition_category Category> requires ((Category == Categories) || ...)
        constexpr bool holds() const noexcept { return category() == Category; }

        template<class TId> requires (
            (std::is_same_v<TId, std::nullptr_t> && nullable)
            || (std::is_same_v<TId, definition_id<TId::category>> && contains<TId::category>))
        constexpr bool holds() const noexcept
        {
            if constexpr(std::is_same_v<TId, std::nullptr_t>) return holds<definition_category::null>();
            else return holds<TId::category>();
        }

        template<definition_category Category>
            requires (Category != definition_category::null && ((Category == Categories) || ...))
        constexpr definition_id<Category> get() const
        {
#ifndef NDEBUG
            if(not holds<Category>()) throw std::invalid_argument{ "definition ID category does not match get" };
#endif
            return definition_id<Category>{ storage_.value & detail::definition_index_mask };
        }

        template<class TId> requires (std::is_same_v<TId, definition_id<TId::category>> && contains<TId::category>)
        constexpr TId get() const { return get<TId::category>(); }

        constexpr auto get() const requires (non_null_count == 1)
        {
            constexpr auto category = []
            {
                definition_category result = definition_category::null;
                ((Categories != definition_category::null ? result = Categories : result), ...);
                return result;
            }();
            return get<category>();
        }

        constexpr auto operator*() const requires (nullable && non_null_count == 1) { return get(); }

        template<definition_category Category>
            requires (Category != definition_category::null && ((Category == Categories) || ...))
        constexpr variant_definition_id<definition_category::null, Category> get_if() const noexcept
        {
            if(not holds<Category>()) return nullptr;
            return definition_id<Category>{ storage_.value & detail::definition_index_mask };
        }

        template<class F>
        constexpr decltype(auto) visit(F&& fn) const
        {
            return visit_impl<Categories...>((F&&)fn);
        }

        friend constexpr bool operator==(variant_definition_id left, variant_definition_id right) noexcept
        {
            return left.value() == right.value();
        }

    private:
        template<definition_category First, definition_category... Rest, class F>
        constexpr decltype(auto) visit_impl(F&& fn) const
        {
            if constexpr(sizeof...(Rest) != 0)
            {
                if(not holds<First>()) return visit_impl<Rest...>((F&&)fn);
            }
            if constexpr(First == definition_category::null) return ((F&&)fn)(nullptr);
            else return ((F&&)fn)(get<First>());
        }

        struct initialized_word { std::uint64_t value = null_value; };
        struct uninitialized_word { std::uint64_t value; };
        std::conditional_t<nullable, initialized_word, uninitialized_word> storage_;
    };

    template<definition_category Category>
    using optional_definition_id = variant_definition_id<definition_category::null, Category>;

    using card_definition_id = definition_id<definition_category::card>;
    using optional_card_definition_id = optional_definition_id<definition_category::card>;
    using card_status_definition_id = definition_id<definition_category::card_status>;
    using optional_card_status_definition_id = optional_definition_id<definition_category::card_status>;
    using support_definition_id = definition_id<definition_category::support>;
    using optional_support_definition_id = optional_definition_id<definition_category::support>;
    using summon_definition_id = definition_id<definition_category::summon>;
    using optional_summon_definition_id = optional_definition_id<definition_category::summon>;
    using combat_status_definition_id = definition_id<definition_category::combat_status>;
    using optional_combat_status_definition_id = optional_definition_id<definition_category::combat_status>;
    using character_definition_id = definition_id<definition_category::character>;
    using optional_character_definition_id = optional_definition_id<definition_category::character>;
    using skill_definition_id = definition_id<definition_category::skill>;
    using optional_skill_definition_id = optional_definition_id<definition_category::skill>;
    using attachment_definition_id = definition_id<definition_category::attachment>;
    using optional_attachment_definition_id = optional_definition_id<definition_category::attachment>;
    using history_summary_definition_id = definition_id<definition_category::history_summary>;
    using optional_history_summary_definition_id = optional_definition_id<definition_category::history_summary>;
    using reaction_definition_id = definition_id<definition_category::reaction>;
    using optional_reaction_definition_id = optional_definition_id<definition_category::reaction>;
}

#endif
