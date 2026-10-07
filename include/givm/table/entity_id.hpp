#ifndef GIVM_TABLE_ENTITY_ID_HPP
#define GIVM_TABLE_ENTITY_ID_HPP

#include <concepts>
#include <cstddef>
#include <cstdint>
#ifndef NDEBUG
#include <stdexcept>
#endif
#include <type_traits>
#include <utility>

#include "../enums/entity_category.hpp"
#include "../enums/elemental_reaction.hpp"
#include "../utils/type_list.hpp"

namespace givm
{
    class player_id
    {
    public:
        static constexpr entity_category category = entity_category::player;

        constexpr player_id() noexcept = default;
        constexpr explicit player_id(std::uint32_t index) noexcept : value_{ index } {}

        constexpr std::uint64_t value() const noexcept { return value_; }
        constexpr std::uint32_t index() const noexcept { return static_cast<std::uint32_t>(value_); }

        friend constexpr bool operator==(player_id, player_id) noexcept = default;

    private:
        template<entity_category...>
        friend class variant_entity_id;

        std::uint64_t value_;
    };

    class hand_card_id
    {
        static constexpr unsigned player_shift = detail::entity_id_bit_width - 1;

    public:
        static constexpr entity_category category = entity_category::hand_card;

        constexpr hand_card_id() noexcept = default;

        constexpr hand_card_id(givm::player_id owner, std::uint32_t index)
        : value_{ (owner.value() << player_shift) | index }
        {
#ifndef NDEBUG
            if(owner.index() >= 2)
                throw std::invalid_argument{ "hand_card_id parent or child index exceeds its bit width" };
#endif
        }

        constexpr std::uint64_t value() const noexcept { return value_; }
        constexpr std::uint32_t index() const noexcept { return static_cast<std::uint32_t>(value_); }

        constexpr givm::player_id player_id() const noexcept
        {
            return givm::player_id{ static_cast<std::uint32_t>((value_ >> player_shift) & 1) };
        }

        friend constexpr bool operator==(hand_card_id, hand_card_id) noexcept = default;

    private:
        template<entity_category...>
        friend class variant_entity_id;

        std::uint64_t value_;
    };

    class deck_card_id
    {
        static constexpr unsigned player_shift = detail::entity_id_bit_width - 1;

    public:
        static constexpr entity_category category = entity_category::deck_card;

        constexpr deck_card_id() noexcept = default;

        constexpr deck_card_id(givm::player_id owner, std::uint32_t index)
        : value_{ (owner.value() << player_shift) | index }
        {
#ifndef NDEBUG
            if(owner.index() >= 2)
                throw std::invalid_argument{ "deck_card_id parent or child index exceeds its bit width" };
#endif
        }

        constexpr std::uint64_t value() const noexcept { return value_; }
        constexpr std::uint32_t index() const noexcept { return static_cast<std::uint32_t>(value_); }

        constexpr givm::player_id player_id() const noexcept
        {
            return givm::player_id{ static_cast<std::uint32_t>((value_ >> player_shift) & 1) };
        }

        friend constexpr bool operator==(deck_card_id, deck_card_id) noexcept = default;

    private:
        template<entity_category...>
        friend class variant_entity_id;

        std::uint64_t value_;
    };

    class hand_card_status_id
    {
        static constexpr unsigned player_shift = detail::entity_id_bit_width - 1;
        static constexpr std::uint64_t parent_mask = (std::uint64_t{ 1 } << detail::entity_parent_bit_width) - 1;

    public:
        static constexpr entity_category category = entity_category::hand_card_status;

        constexpr hand_card_status_id() noexcept = default;

        constexpr hand_card_status_id(givm::hand_card_id owner, std::uint32_t index)
        : value_{ (owner.player_id().value() << player_shift) | (std::uint64_t(owner.index()) << 32) | index }
        {
#ifndef NDEBUG
            if(owner.player_id().index() >= 2 || owner.index() > parent_mask)
                throw std::invalid_argument{ "hand_card_status_id parent or child index exceeds its bit width" };
#endif
        }

        constexpr std::uint64_t value() const noexcept { return value_; }
        constexpr std::uint32_t index() const noexcept { return static_cast<std::uint32_t>(value_); }

        constexpr givm::player_id player_id() const noexcept
        {
            return givm::player_id{ static_cast<std::uint32_t>((value_ >> player_shift) & 1) };
        }

        constexpr givm::hand_card_id hand_card_id() const noexcept
        {
            return { player_id(), static_cast<std::uint32_t>((value_ >> 32) & parent_mask) };
        }

        friend constexpr bool operator==(hand_card_status_id, hand_card_status_id) noexcept = default;

    private:
        template<entity_category...>
        friend class variant_entity_id;

        std::uint64_t value_;
    };

    class deck_card_status_id
    {
        static constexpr unsigned player_shift = detail::entity_id_bit_width - 1;
        static constexpr std::uint64_t parent_mask = (std::uint64_t{ 1 } << detail::entity_parent_bit_width) - 1;

    public:
        static constexpr entity_category category = entity_category::deck_card_status;

        constexpr deck_card_status_id() noexcept = default;

        constexpr deck_card_status_id(givm::deck_card_id owner, std::uint32_t index)
        : value_{ (owner.player_id().value() << player_shift) | (std::uint64_t(owner.index()) << 32) | index }
        {
#ifndef NDEBUG
            if(owner.player_id().index() >= 2 || owner.index() > parent_mask)
                throw std::invalid_argument{ "deck_card_status_id parent or child index exceeds its bit width" };
#endif
        }

        constexpr std::uint64_t value() const noexcept { return value_; }
        constexpr std::uint32_t index() const noexcept { return static_cast<std::uint32_t>(value_); }

        constexpr givm::player_id player_id() const noexcept
        {
            return givm::player_id{ static_cast<std::uint32_t>((value_ >> player_shift) & 1) };
        }

        constexpr givm::deck_card_id deck_card_id() const noexcept
        {
            return { player_id(), static_cast<std::uint32_t>((value_ >> 32) & parent_mask) };
        }

        friend constexpr bool operator==(deck_card_status_id, deck_card_status_id) noexcept = default;

    private:
        template<entity_category...>
        friend class variant_entity_id;

        std::uint64_t value_;
    };

    class support_id
    {
        static constexpr unsigned player_shift = detail::entity_id_bit_width - 1;

    public:
        static constexpr entity_category category = entity_category::support;

        constexpr support_id() noexcept = default;

        constexpr support_id(givm::player_id owner, std::uint32_t index)
        : value_{ (owner.value() << player_shift) | index }
        {
#ifndef NDEBUG
            if(owner.index() >= 2)
                throw std::invalid_argument{ "support_id parent or child index exceeds its bit width" };
#endif
        }

        constexpr std::uint64_t value() const noexcept { return value_; }
        constexpr std::uint32_t index() const noexcept { return static_cast<std::uint32_t>(value_); }

        constexpr givm::player_id player_id() const noexcept
        {
            return givm::player_id{ static_cast<std::uint32_t>((value_ >> player_shift) & 1) };
        }

        friend constexpr bool operator==(support_id, support_id) noexcept = default;

    private:
        template<entity_category...>
        friend class variant_entity_id;

        std::uint64_t value_;
    };

    class summon_id
    {
        static constexpr unsigned player_shift = detail::entity_id_bit_width - 1;

    public:
        static constexpr entity_category category = entity_category::summon;

        constexpr summon_id() noexcept = default;

        constexpr summon_id(givm::player_id owner, std::uint32_t index)
        : value_{ (owner.value() << player_shift) | index }
        {
#ifndef NDEBUG
            if(owner.index() >= 2)
                throw std::invalid_argument{ "summon_id parent or child index exceeds its bit width" };
#endif
        }

        constexpr std::uint64_t value() const noexcept { return value_; }
        constexpr std::uint32_t index() const noexcept { return static_cast<std::uint32_t>(value_); }

        constexpr givm::player_id player_id() const noexcept
        {
            return givm::player_id{ static_cast<std::uint32_t>((value_ >> player_shift) & 1) };
        }

        friend constexpr bool operator==(summon_id, summon_id) noexcept = default;

    private:
        template<entity_category...>
        friend class variant_entity_id;

        std::uint64_t value_;
    };

    class combat_status_id
    {
        static constexpr unsigned player_shift = detail::entity_id_bit_width - 1;

    public:
        static constexpr entity_category category = entity_category::combat_status;

        constexpr combat_status_id() noexcept = default;

        constexpr combat_status_id(givm::player_id owner, std::uint32_t index)
        : value_{ (owner.value() << player_shift) | index }
        {
#ifndef NDEBUG
            if(owner.index() >= 2)
                throw std::invalid_argument{ "combat_status_id parent or child index exceeds its bit width" };
#endif
        }

        constexpr std::uint64_t value() const noexcept { return value_; }
        constexpr std::uint32_t index() const noexcept { return static_cast<std::uint32_t>(value_); }

        constexpr givm::player_id player_id() const noexcept
        {
            return givm::player_id{ static_cast<std::uint32_t>((value_ >> player_shift) & 1) };
        }

        friend constexpr bool operator==(combat_status_id, combat_status_id) noexcept = default;

    private:
        template<entity_category...>
        friend class variant_entity_id;

        std::uint64_t value_;
    };

    class character_id
    {
        static constexpr unsigned player_shift = detail::entity_id_bit_width - 1;

    public:
        static constexpr entity_category category = entity_category::character;

        constexpr character_id() noexcept = default;

        constexpr character_id(givm::player_id owner, std::uint32_t index)
        : value_{ (owner.value() << player_shift) | index }
        {
#ifndef NDEBUG
            if(owner.index() >= 2)
                throw std::invalid_argument{ "character_id parent or child index exceeds its bit width" };
#endif
        }

        constexpr std::uint64_t value() const noexcept { return value_; }
        constexpr std::uint32_t index() const noexcept { return static_cast<std::uint32_t>(value_); }

        constexpr givm::player_id player_id() const noexcept
        {
            return givm::player_id{ static_cast<std::uint32_t>((value_ >> player_shift) & 1) };
        }

        friend constexpr bool operator==(character_id, character_id) noexcept = default;

    private:
        template<entity_category...>
        friend class variant_entity_id;

        std::uint64_t value_;
    };

    class skill_id
    {
        static constexpr unsigned player_shift = detail::entity_id_bit_width - 1;
        static constexpr std::uint64_t parent_mask = (std::uint64_t{ 1 } << detail::entity_parent_bit_width) - 1;

    public:
        static constexpr entity_category category = entity_category::skill;

        constexpr skill_id() noexcept = default;

        constexpr skill_id(givm::character_id owner, std::uint32_t index)
        : value_{ (owner.player_id().value() << player_shift) | (std::uint64_t(owner.index()) << 32) | index }
        {
#ifndef NDEBUG
            if(owner.player_id().index() >= 2 || owner.index() > parent_mask)
                throw std::invalid_argument{ "skill_id parent or child index exceeds its bit width" };
#endif
        }

        constexpr std::uint64_t value() const noexcept { return value_; }
        constexpr std::uint32_t index() const noexcept { return static_cast<std::uint32_t>(value_); }

        constexpr givm::player_id player_id() const noexcept
        {
            return givm::player_id{ static_cast<std::uint32_t>((value_ >> player_shift) & 1) };
        }

        constexpr givm::character_id character_id() const noexcept
        {
            return { player_id(), static_cast<std::uint32_t>((value_ >> 32) & parent_mask) };
        }

        friend constexpr bool operator==(skill_id, skill_id) noexcept = default;

    private:
        template<entity_category...>
        friend class variant_entity_id;

        std::uint64_t value_;
    };

    class attachment_id
    {
        static constexpr unsigned player_shift = detail::entity_id_bit_width - 1;
        static constexpr std::uint64_t parent_mask = (std::uint64_t{ 1 } << detail::entity_parent_bit_width) - 1;

    public:
        static constexpr entity_category category = entity_category::attachment;

        constexpr attachment_id() noexcept = default;

        constexpr attachment_id(givm::character_id owner, std::uint32_t index)
        : value_{ (owner.player_id().value() << player_shift) | (std::uint64_t(owner.index()) << 32) | index }
        {
#ifndef NDEBUG
            if(owner.player_id().index() >= 2 || owner.index() > parent_mask)
                throw std::invalid_argument{ "attachment_id parent or child index exceeds its bit width" };
#endif
        }

        constexpr std::uint64_t value() const noexcept { return value_; }
        constexpr std::uint32_t index() const noexcept { return static_cast<std::uint32_t>(value_); }

        constexpr givm::player_id player_id() const noexcept
        {
            return givm::player_id{ static_cast<std::uint32_t>((value_ >> player_shift) & 1) };
        }

        constexpr givm::character_id character_id() const noexcept
        {
            return { player_id(), static_cast<std::uint32_t>((value_ >> 32) & parent_mask) };
        }

        friend constexpr bool operator==(attachment_id, attachment_id) noexcept = default;

    private:
        template<entity_category...>
        friend class variant_entity_id;

        std::uint64_t value_;
    };

    class reaction_id
    {
        static constexpr unsigned player_shift = detail::entity_id_bit_width - 1;

    public:
        static constexpr entity_category category = entity_category::reaction;

        constexpr reaction_id() noexcept = default;

        constexpr reaction_id(givm::player_id player, elemental_reaction slot)
        : value_{ (player.value() << player_shift) | std::uint64_t(slot) }
        {
#ifndef NDEBUG
            if(player.index() >= 2 || slot == elemental_reaction::none)
                throw std::invalid_argument{ "reaction_id contains an invalid player or empty reaction slot" };
#endif
        }

        constexpr std::uint64_t value() const noexcept { return value_; }
        constexpr std::uint32_t index() const noexcept { return static_cast<std::uint32_t>(value_); }

        constexpr givm::player_id player_id() const noexcept
        {
            return givm::player_id{ static_cast<std::uint32_t>((value_ >> player_shift) & 1) };
        }

        constexpr elemental_reaction slot() const noexcept
        {
            return static_cast<elemental_reaction>(index());
        }

        friend constexpr bool operator==(reaction_id, reaction_id) noexcept = default;

    private:
        template<entity_category...>
        friend class variant_entity_id;

        std::uint64_t value_;
    };

    namespace detail
    {
        using entity_ids = type_list<player_id, hand_card_id, deck_card_id, hand_card_status_id, deck_card_status_id, support_id, summon_id, combat_status_id, character_id, skill_id, attachment_id, reaction_id>;
    }

    template<entity_category Category> requires (Category < entity_category::null)
    using entity_id = detail::entity_ids::type_at<static_cast<std::size_t>(Category)>;

    constexpr player_id other_player(player_id player) noexcept
    {
        return player_id{ player.index() ^ 1u };
    }

    template<entity_category... Categories>
    class variant_entity_id
    {
        static_assert(sizeof...(Categories) != 0);
        static_assert(((Categories <= entity_category::null) && ...));
        static constexpr bool nullable = ((Categories == entity_category::null) || ...);
        static constexpr std::size_t non_null_count = ((Categories != entity_category::null ? 1u : 0u) + ...);
        template<entity_category Category>
        static constexpr bool contains = ((Category == Categories) || ...);
        static constexpr std::uint64_t payload_mask = (std::uint64_t{ 1 } << detail::entity_id_bit_width) - 1;
        static constexpr std::uint64_t null_value =
            std::uint64_t(entity_category::null) << detail::entity_id_bit_width;

    public:
        constexpr variant_entity_id() noexcept = default;
        constexpr variant_entity_id(std::nullptr_t) noexcept requires nullable : storage_{ null_value } {}

        template<class TId> requires (std::same_as<TId, entity_id<TId::category>> && contains<TId::category>)
        constexpr variant_entity_id(TId id) noexcept
        : storage_{ id.value() | (std::uint64_t(TId::category) << detail::entity_id_bit_width) }
        {}

        template<entity_category... Other> requires ((contains<Other>) && ...)
        constexpr variant_entity_id(variant_entity_id<Other...> id) noexcept : storage_{ id.value() } {}

        constexpr std::uint64_t value() const noexcept { return storage_.value; }
        constexpr entity_category category() const noexcept
        {
            return static_cast<entity_category>(storage_.value >> detail::entity_id_bit_width);
        }
        constexpr explicit operator bool() const noexcept requires nullable
        {
            return category() != entity_category::null;
        }

        constexpr bool has_value() const noexcept requires nullable { return static_cast<bool>(*this); }

        template<entity_category Category> requires contains<Category>
        constexpr bool holds() const noexcept { return category() == Category; }

        template<class TId> requires (
            (std::same_as<TId, std::nullptr_t> && nullable)
            || (std::same_as<TId, entity_id<TId::category>> && contains<TId::category>))
        constexpr bool holds() const noexcept
        {
            if constexpr(std::same_as<TId, std::nullptr_t>) return holds<entity_category::null>();
            else return holds<TId::category>();
        }

        template<entity_category Category> requires (Category != entity_category::null && contains<Category>)
        constexpr entity_id<Category> get() const
        {
#ifndef NDEBUG
            if(not holds<Category>()) throw std::invalid_argument{ "entity ID category does not match get" };
#endif
            entity_id<Category> result;
            result.value_ = storage_.value & payload_mask;
            return result;
        }

        template<class TId> requires (std::same_as<TId, entity_id<TId::category>> && contains<TId::category>)
        constexpr TId get() const { return get<TId::category>(); }

        constexpr auto get() const requires (non_null_count == 1)
        {
            constexpr auto category = []
            {
                entity_category result = entity_category::null;
                ((Categories != entity_category::null ? result = Categories : result), ...);
                return result;
            }();
            return get<category>();
        }

        constexpr auto operator*() const requires (nullable && non_null_count == 1) { return get(); }

        template<entity_category Category> requires (Category != entity_category::null && contains<Category>)
        constexpr variant_entity_id<entity_category::null, Category> get_if() const noexcept
        {
            if(not holds<Category>()) return nullptr;
            return get<Category>();
        }

        constexpr givm::player_id player_id() const noexcept requires (not nullable)
        {
            return visit([](auto id) -> givm::player_id
            {
                if constexpr(std::same_as<decltype(id), givm::player_id>) return id;
                else return id.player_id();
            });
        }

        template<class F>
        constexpr decltype(auto) visit(F&& fn) const
        {
            return visit_impl<Categories...>((F&&)fn);
        }

        friend constexpr bool operator==(variant_entity_id left, variant_entity_id right) noexcept
        {
            return left.value() == right.value();
        }

    private:
        template<entity_category First, entity_category... Rest, class F>
        constexpr decltype(auto) visit_impl(F&& fn) const
        {
            if constexpr(sizeof...(Rest) != 0)
            {
                if(not holds<First>()) return visit_impl<Rest...>((F&&)fn);
            }
            if constexpr(First == entity_category::null) return ((F&&)fn)(nullptr);
            else return ((F&&)fn)(get<First>());
        }

        struct initialized_word { std::uint64_t value = null_value; };
        struct uninitialized_word { std::uint64_t value; };
        std::conditional_t<nullable, initialized_word, uninitialized_word> storage_;
    };

    template<entity_category Category>
    using optional_entity_id = variant_entity_id<entity_category::null, Category>;

    using optional_player_id = optional_entity_id<entity_category::player>;
    using optional_hand_card_id = optional_entity_id<entity_category::hand_card>;
    using optional_deck_card_id = optional_entity_id<entity_category::deck_card>;
    using optional_hand_card_status_id = optional_entity_id<entity_category::hand_card_status>;
    using optional_deck_card_status_id = optional_entity_id<entity_category::deck_card_status>;
    using optional_support_id = optional_entity_id<entity_category::support>;
    using optional_summon_id = optional_entity_id<entity_category::summon>;
    using optional_combat_status_id = optional_entity_id<entity_category::combat_status>;
    using optional_character_id = optional_entity_id<entity_category::character>;
    using optional_skill_id = optional_entity_id<entity_category::skill>;
    using optional_attachment_id = optional_entity_id<entity_category::attachment>;
    using optional_reaction_id = optional_entity_id<entity_category::reaction>;
}

#endif
