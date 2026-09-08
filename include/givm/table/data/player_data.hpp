#ifndef GIVM_TABLE_DATA_PLAYER_DATA_HPP
#define GIVM_TABLE_DATA_PLAYER_DATA_HPP

#include <algorithm>
#include <cstddef>
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
    };

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
                std::erase_if(datas, [](const auto& data){ return not data.definition_id.is_valid(); });
                if constexpr(requires{ (*datas.begin()).clean_up(); })
                {
                    for(auto&& data : datas)
                    {
                        data.clean_up();
                    }
                }
            };
            clean_up_datas(hand_card_datas);

            size_t front = 0;
            size_t back = deck_card_datas.size();
            while(true)
            {
                while(front < back && deck_card_datas[front].definition_id.is_valid())
                {
                    ++front;
                }
                while(front < back && not deck_card_datas[back - 1].definition_id.is_valid())
                {
                    --back;
                }
                if(front == back)
                {
                    break;
                }

                const size_t source = --back;
                deck_card_datas[front] = std::move(deck_card_datas[source]);
                deck_card_datas[source].definition_id.set_invalid();
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
