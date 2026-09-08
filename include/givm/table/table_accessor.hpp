#ifndef GIVM_TABLE_TABLE_ACCESSOR_HPP
#define GIVM_TABLE_TABLE_ACCESSOR_HPP

#include "../definition.hpp"

namespace givm::detail
{
    struct uninitialized_entity_t
    {
        explicit constexpr uninitialized_entity_t() noexcept = default;
    };

    inline constexpr uninitialized_entity_t uninitialized_entity;

    struct table_accessor
    {
    private:
        friend class ::givm::card_table;
        template<class>
        friend class ::givm::player_entity;
        template<class>
        friend class ::givm::hand_card_entity;
        template<class>
        friend class ::givm::deck_card_entity;
        template<class>
        friend class ::givm::hand_card_status_entity;
        template<class>
        friend class ::givm::deck_card_status_entity;
        template<class>
        friend class ::givm::support_entity;
        template<class>
        friend class ::givm::summon_entity;
        template<class>
        friend class ::givm::combat_status_entity;
        template<class>
        friend class ::givm::character_entity;
        template<class>
        friend class ::givm::skill_entity;
        template<class>
        friend class ::givm::attachment_entity;
        template<class, class, class>
        friend class ::givm::card_status_range;

        template<class TEntity>
        static constexpr TEntity make_uninitialized() noexcept
        {
            return TEntity{ uninitialized_entity };
        }

        template<class TStorageOwner>
        static constexpr decltype(auto) storage_of(TStorageOwner& owner) noexcept
        {
            return (owner.storage_);
        }
    };
}

#endif
