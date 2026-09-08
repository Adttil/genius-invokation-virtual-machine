#ifndef GIVM_TABLE_DATA_CHARACTER_DATA_HPP
#define GIVM_TABLE_DATA_CHARACTER_DATA_HPP

#include <algorithm>
#include <cstdint>
#include <vector>

#include "../../definition.hpp"
#include "../../enums/element_aura.hpp"
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
    };

    struct character_data
    {
        definition_id<character_view> definition_id;
        character_state state;
        std::vector<skill_data> skill_datas;
        std::vector<attachment_data> attachment_datas;

        constexpr void clean_up() noexcept
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
            clean_up_datas(skill_datas);
            clean_up_datas(attachment_datas);
        }
    };
}

#endif
