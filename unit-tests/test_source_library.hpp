#ifndef GIVM_TEST_SOURCE_LIBRARY_HPP
#define GIVM_TEST_SOURCE_LIBRARY_HPP

#include <string_view>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition.hpp>
#include <givm/basic_definitions.hpp>

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

    template<givm::definition_category TCategory>
    struct reaction_source
    {
        static constexpr auto category = TCategory;

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

    inline constexpr reaction_source<givm::definition_category::combat_status> dendro_core{ "TestDendroCore" };
    inline constexpr reaction_source<givm::definition_category::combat_status> catalyzing_field{ "TestCatalyzingField" };
    inline constexpr reaction_source<givm::definition_category::summon> burning_flame{ "TestBurningFlame" };

    inline constexpr reaction_source<givm::definition_category::attachment> frozen{ "TestFrozen" };

    inline constexpr reaction_source<givm::definition_category::combat_status> shield{ "TestShield" };

    template<givm::elemental_reaction Slot>
    struct default_reaction_source : givm::genshin_impact::reaction_3_3_0_source<Slot>
    {
        using base = givm::genshin_impact::reaction_3_3_0_source<Slot>;
        using definition_type = typename base::definition_type;
        std::string_view generated_name;

        constexpr default_reaction_source(std::string_view generated = {})
            : base{ givm::genshin_impact::reaction_names_3_3_0[Slot] },
              generated_name{ generated } {}

        constexpr auto combat_status_dependencies() const noexcept
        {
            if constexpr(Slot == givm::elemental_reaction::bloom || Slot == givm::elemental_reaction::quicken
                || Slot >= givm::elemental_reaction::crystallize_cryo) return std::array{ generated_name };
            else return std::array<std::string_view, 0>{};
        }

        constexpr auto summon_dependencies() const noexcept
        {
            if constexpr(Slot == givm::elemental_reaction::burning) return std::array{ generated_name };
            else return std::array<std::string_view, 0>{};
        }

        constexpr auto attachment_dependencies() const noexcept
        {
            if constexpr(Slot == givm::elemental_reaction::frozen) return std::array{ generated_name };
            else return std::array<std::string_view, 0>{};
        }

        definition_type compile(givm::definition_compile_context& context) const
        {
            if constexpr(Slot == givm::elemental_reaction::bloom || Slot == givm::elemental_reaction::quicken
                || Slot >= givm::elemental_reaction::crystallize_cryo)
                return { context.add_immediate_effect(std::tuple{ givm::generate_combat_status{
                    .definition = context.resolve_id<givm::definition_category::combat_status>(generated_name),
                    .state = Slot >= givm::elemental_reaction::crystallize_cryo
                        ? givm::combat_status_state{ 1, 0 }
                        : givm::combat_status_state{ Slot == givm::elemental_reaction::bloom ? 1u : 2u, 0 } } }), {} };
            else if constexpr(Slot == givm::elemental_reaction::burning)
                return { context.add_immediate_effect(std::tuple{ givm::summon{
                    .definition = context.resolve_id<givm::definition_category::summon>(generated_name), .state = { 1, 1 } } }), {} };
            else if constexpr(Slot == givm::elemental_reaction::frozen)
                return { context.add_immediate_effect(std::tuple{ givm::attach{} }),
                    context.resolve_id<givm::definition_category::attachment>(generated_name) };
            else return base::compile(context);
        }
    };

    constexpr auto default_reactions(std::string_view core, std::string_view field,
        std::string_view flame, std::string_view freeze, std::string_view guard)
    {
        using enum givm::elemental_reaction;
        return std::tuple{
            default_reaction_source<melt>{}, default_reaction_source<vaporize>{},
            default_reaction_source<overloaded>{}, default_reaction_source<superconduct>{},
            default_reaction_source<electro_charged>{}, default_reaction_source<frozen>{ freeze },
            default_reaction_source<burning>{ flame }, default_reaction_source<bloom>{ core },
            default_reaction_source<quicken>{ field }, default_reaction_source<swirl_cryo>{},
            default_reaction_source<swirl_hydro>{}, default_reaction_source<swirl_pyro>{},
            default_reaction_source<swirl_electro>{}, default_reaction_source<crystallize_cryo>{ guard },
            default_reaction_source<crystallize_hydro>{ guard }, default_reaction_source<crystallize_pyro>{ guard },
            default_reaction_source<crystallize_electro>{ guard } };
    }

    inline constexpr auto basic_reactions = default_reactions(
        dendro_core.name(), catalyzing_field.name(), burning_flame.name(), frozen.name(), shield.name());
    inline constexpr auto basic_sources = givm::genshin_impact::reaction_names_3_3_0;

    inline givm::definition_source_library make_source_library()
    {
        givm::definition_source_library sources;
        REQUIRE(sources.add(dendro_core, catalyzing_field, burning_flame, frozen, shield));
        std::apply([&](const auto&... source) { REQUIRE(sources.add(source...)); }, basic_reactions);
        return sources;
    }
}

#endif
