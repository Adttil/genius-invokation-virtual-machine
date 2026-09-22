#ifndef GIVM_TABLE_DATA_PLAYER_DATA_HPP
#define GIVM_TABLE_DATA_PLAYER_DATA_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <ranges>
#include <vector>

#include "../../enums/elemental_dice.hpp"
#include "../../utils/debug.hpp"
#include "../entity_id.hpp"
#include "card_data.hpp"
#include "character_data.hpp"
#include "combat_status_data.hpp"
#include "summon_data.hpp"
#include "support_data.hpp"

#include "../../macro_define.hpp"

namespace givm
{
    struct player_state
    {
        dice_counts dice;
        std::optional<character_id> active_character;
        std::uint32_t hand_limit = 10;
        std::uint32_t summon_limit = 4;
        std::uint32_t support_limit = 4;
    };
}

namespace givm::detail
{
    struct player_data
    {
        player_state state;

        std::vector<card_data> hand_card_datas;
        std::vector<card_data> deck_card_datas;
        std::vector<size_t> deck_card_order;
        std::vector<summon_data> summon_datas;
        std::vector<support_data> support_datas;
        std::vector<combat_status_data> combat_status_datas;
        std::vector<character_data> character_datas;

        void clean_up() noexcept
        {
            constexpr auto clean_up_datas = [](auto& datas)
            {
                std::erase_if(datas, [](const auto& data)
                {
                    constexpr size_t erased_mask = size_t{ 1 } << (std::numeric_limits<size_t>::digits - 1);
                    return (data.definition_and_flags & erased_mask) != 0;
                });
                if constexpr(requires{ (*datas.begin()).clean_up(); })
                {
                    for(auto&& data : datas)
                    {
                        data.clean_up();
                    }
                }
            };
            clean_up_datas(hand_card_datas);

            constexpr size_t deck_card_erased_mask = size_t{ 1 } << (std::numeric_limits<size_t>::digits - 1);
            size_t front = 0;
            size_t back = deck_card_datas.size();
            while(true)
            {
                while(front < back && (deck_card_datas[front].definition_and_flags & deck_card_erased_mask) == 0)
                {
                    ++front;
                }
                while(front < back && (deck_card_datas[back - 1].definition_and_flags & deck_card_erased_mask) != 0)
                {
                    --back;
                }
                if(front == back)
                {
                    break;
                }

                const size_t source = --back;
                deck_card_datas[front] = std::move(deck_card_datas[source]);
                deck_card_datas[source].definition_and_flags = static_cast<size_t>(-1);
                deck_card_datas[source].first_status = invalid_status_index;
                deck_card_datas[source].last_status = invalid_status_index;

                const auto order = std::ranges::find(deck_card_order, source);
                GIVM_ASSERT(order != deck_card_order.end());
                *order = front;
                ++front;
            }
            deck_card_datas.resize(front);
            clean_up_datas(summon_datas);
            clean_up_datas(support_datas);
            clean_up_datas(combat_status_datas);
            clean_up_datas(character_datas);
        }
    };
}

#include "../../macro_undef.hpp"
#endif
