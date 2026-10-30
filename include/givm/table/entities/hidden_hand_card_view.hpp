#ifndef GIVM_TABLE_ENTITIES_HIDDEN_HAND_CARD_VIEW_HPP
#define GIVM_TABLE_ENTITIES_HIDDEN_HAND_CARD_VIEW_HPP

#include "hand_card_handle.hpp"
#include "../../enums/role.hpp"

namespace givm
{
    template<unsigned Player, role Role> class perspective_player_view;

    class hidden_hand_card_view
    {
    public:
        static constexpr entity_category category = entity_category::hand_card;

        constexpr hand_card_id id() const { return card_.id(); }
        constexpr bool is_valid() const { return card_.is_valid(); }
        constexpr explicit operator bool() const { return is_valid(); }
        constexpr auto statuses() const { return card_.statuses(); }

    private:
        friend class definition_library;
        template<unsigned, role> friend class perspective_player_view;
        constexpr explicit hidden_hand_card_view(hand_card_view card) noexcept : card_{ card } {}
        hand_card_view card_;
    };
}

#endif
