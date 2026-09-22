#ifndef GIVM_TEST_SOURCE_LIBRARY_HPP
#define GIVM_TEST_SOURCE_LIBRARY_HPP

#include <string_view>

#include <givm/definition.hpp>

namespace givm_test
{
    template<class TCategory>
    struct reaction_source
    {
        using definition_category = TCategory;

        struct definition_type{};

        std::string_view source_name;

        constexpr std::string_view name() const noexcept
        {
            return source_name;
        }

        constexpr definition_type compile(givm::definition_compile_context&) const noexcept
        {
            return {};
        }
    };

    inline constexpr reaction_source<givm::combat_status_view> dendro_core{ "TestDendroCore" };
    inline constexpr reaction_source<givm::combat_status_view> catalyzing_field{ "TestCatalyzingField" };
    inline constexpr reaction_source<givm::summon_view> burning_flame{ "TestBurningFlame" };

    inline constexpr reaction_source<givm::attachment_view> frozen{ "TestFrozen" };

    inline givm::definition_source_library make_source_library()
    {
        return { dendro_core, catalyzing_field, burning_flame, frozen };
    }
}

#endif
