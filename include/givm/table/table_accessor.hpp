#ifndef GIVM_TABLE_TABLE_ACCESSOR_HPP
#define GIVM_TABLE_TABLE_ACCESSOR_HPP

#include "entity_fwd.hpp"
#include "issued_id.hpp"

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
        friend class unrestricted_table;
        template<class>
        friend class basic_player_handle;
        template<class>
        friend class basic_hand_card_handle;
        template<class>
        friend class basic_deck_card_handle;
        template<class>
        friend class basic_hand_card_status_handle;
        template<class>
        friend class basic_deck_card_status_handle;
        template<class>
        friend class basic_support_handle;
        template<class>
        friend class basic_summon_handle;
        template<class>
        friend class basic_combat_status_handle;
        template<class>
        friend class basic_character_handle;
        template<class>
        friend class basic_skill_handle;
        template<class>
        friend class basic_attachment_handle;
        template<class, class, class>
        friend class card_status_range;

        template<class TTag>
        static constexpr issued_id<TTag> make_issued_id(size_t value) noexcept
        {
            return issued_id<TTag>{ value };
        }

        template<class THandle>
        static constexpr THandle make_uninitialized() noexcept
        {
            return THandle{ uninitialized_entity };
        }

        template<class TStorageOwner>
        static constexpr decltype(auto) storage_of(TStorageOwner& owner) noexcept
        {
            return (owner.storage_);
        }
    };
}

#endif
