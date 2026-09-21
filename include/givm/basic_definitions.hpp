#ifndef GIVM_BASIC_DEFINITIONS_HPP
#define GIVM_BASIC_DEFINITIONS_HPP

#include "definition.hpp"

#include <string_view>

namespace givm::genshin_impact
{
    // These sources currently reserve the official definitions' names and categories.
    // Their effects and entity creation rules remain to be implemented.
    struct dendro_core_3_3_0_source
    {
        using definition_category = combat_status_view;

        struct definition_type{};

        constexpr std::string_view name() const noexcept
        {
            return "dendro_core-3.3.0-genshin_impact";
        }

        constexpr definition_type compile(definition_compile_context&) const noexcept
        {
            return {};
        }
    };

    struct catalyzing_field_3_3_0_source
    {
        using definition_category = combat_status_view;

        struct definition_type{};

        constexpr std::string_view name() const noexcept
        {
            return "catalyzing_field-3.3.0-genshin_impact";
        }

        constexpr definition_type compile(definition_compile_context&) const noexcept
        {
            return {};
        }
    };

    struct catalyzing_field_3_4_0_source
    {
        using definition_category = combat_status_view;

        struct definition_type{};

        constexpr std::string_view name() const noexcept
        {
            return "catalyzing_field-3.4.0-genshin_impact";
        }

        constexpr definition_type compile(definition_compile_context&) const noexcept
        {
            return {};
        }
    };

    struct burning_flame_3_3_0_source
    {
        using definition_category = summon_view;

        struct definition_type{};

        constexpr std::string_view name() const noexcept
        {
            return "burning_flame-3.3.0-genshin_impact";
        }

        constexpr definition_type compile(definition_compile_context&) const noexcept
        {
            return {};
        }
    };

    inline constexpr dendro_core_3_3_0_source dendro_core_3_3_0;
    inline constexpr catalyzing_field_3_3_0_source catalyzing_field_3_3_0;
    inline constexpr catalyzing_field_3_4_0_source catalyzing_field_3_4_0;
    inline constexpr burning_flame_3_3_0_source burning_flame_3_3_0;
}

#endif
