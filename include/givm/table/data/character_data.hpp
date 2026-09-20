#ifndef GIVM_TABLE_DATA_CHARACTER_DATA_HPP
#define GIVM_TABLE_DATA_CHARACTER_DATA_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

#include "../../enums/element.hpp"
#include "../../enums/element_aura.hpp"
#include "../../enums/equipment_type.hpp"
#include "../../enums/weapon_type.hpp"
#include "../issued_id.hpp"
#include "attachment_data.hpp"
#include "skill_data.hpp"

namespace givm
{
    struct character_state
    {
        std::uint32_t max_health;
        std::uint32_t max_energy;
        std::uint32_t health;
        std::uint32_t energy;
        element_aura aura = element_aura::none;
        weapon_type_mask allowed_weapon_types;
        tag_id energy_tag{};
        givm::element element = givm::element::none;
    };
}

namespace givm::detail
{
    struct character_data
    {
        size_t definition_and_flags = static_cast<size_t>(-1);
        character_state state;
        std::vector<skill_data> skill_datas;
        std::vector<attachment_data> attachment_datas;
        std::array<size_t, static_cast<size_t>(equipment_type::none)> equipment_indices = []
        {
            std::array<size_t, static_cast<size_t>(equipment_type::none)> result;
            result.fill(static_cast<size_t>(-1));
            return result;
        }();

        constexpr void clean_up() noexcept
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
            clean_up_datas(skill_datas);
            size_t next_index = 0;
            for(size_t index = 0; index != attachment_datas.size(); ++index)
            {
                constexpr size_t erased_mask = size_t{ 1 } << (std::numeric_limits<size_t>::digits - 1);
                if((attachment_datas[index].definition_and_flags & erased_mask) != 0)
                {
                    continue;
                }
                if(index != next_index)
                {
                    for(auto& equipment_index : equipment_indices)
                    {
                        if(equipment_index == index)
                        {
                            equipment_index = next_index;
                            break;
                        }
                    }
                    attachment_datas[next_index] = std::move(attachment_datas[index]);
                }
                ++next_index;
            }
            attachment_datas.resize(next_index);
        }
    };
}

#endif
