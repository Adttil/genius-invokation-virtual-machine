#ifndef GIVM_TABLE_DATA_CHARACTER_DATA_HPP
#define GIVM_TABLE_DATA_CHARACTER_DATA_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include "../../enums/element_aura.hpp"
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
        tag_id energy_tag{};
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
            clean_up_datas(attachment_datas);
        }
    };
}

#endif
