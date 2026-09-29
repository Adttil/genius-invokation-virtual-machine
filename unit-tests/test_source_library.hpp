#ifndef GIVM_TEST_SOURCE_LIBRARY_HPP
#define GIVM_TEST_SOURCE_LIBRARY_HPP

#include <string_view>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition.hpp>

namespace givm_test
{
    template<class TResult>
    auto require_success(TResult&& result)
    {
        if(not result)
        {
            INFO(error_string(result.error()));
            REQUIRE(result.has_value());
        }
        return std::move(*result);
    }

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

    inline constexpr reaction_source<givm::combat_status_view> shield{ "TestShield" };

    inline constexpr givm::basic_definition_sources basic_sources{
        dendro_core, catalyzing_field, burning_flame, frozen, shield };

    inline givm::definition_source_library make_source_library()
    {
        return {};
    }
}

#endif
