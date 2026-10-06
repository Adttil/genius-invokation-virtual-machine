#include <array>
#include <concepts>
#include <cstdint>
#include <tuple>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/definition_source.hpp>
#include <givm/compile.hpp>

#include "../test_source_library.hpp"

namespace givm_test::executor::effects
{
namespace
{
    template<class T>
    concept can_defer = requires(T value) { givm::defer_invoke(value); givm::fixed_defer_invoke(value); };

    template<class T>
    concept has_random = requires(const T& context) { context.random(); };

    template<class TContext, class TEffect>
    concept can_invoke = requires(TContext& context, TEffect value) { context.invoke(value); };

    static_assert(can_defer<givm::normal_effect> && can_defer<givm::preview_effect>);
    static_assert(not can_defer<givm::immediate_effect>);
    static_assert(not std::convertible_to<givm::normal_effect, givm::immediate_effect>);
    static_assert(not std::convertible_to<givm::preview_effect, givm::normal_effect>);
    static_assert(has_random<givm::normal_handle_context<givm::skill_view>>);
    static_assert(has_random<givm::immediate_handle_context<givm::skill_view>>);
    static_assert(not has_random<givm::preview_handle_context<givm::skill_view>>);
    static_assert(can_invoke<givm::preview_handle_context<givm::skill_view>, givm::preview_effect>);
    static_assert(not can_invoke<givm::preview_handle_context<givm::skill_view>, givm::normal_effect>);
    static_assert(not can_invoke<givm::normal_handle_context<givm::skill_view>, givm::immediate_effect>);
    static_assert(std::same_as<givm::handle_fn_t<givm::skill_view, givm::cost_of_switch>,
        givm::preview_effect (*)(const givm::definition_data&, givm::cost_of_switch&,
            givm::preview_handle_context<givm::skill_view>&)>);
    static_assert(std::same_as<givm::handle_fn_t<givm::skill_view, givm::damage_calculation>,
        givm::immediate_effect (*)(const givm::definition_data&, givm::damage_calculation&,
            givm::immediate_handle_context<givm::skill_view>&, std::uint32_t)>);

    template<givm::event_category Category>
    struct source
    {
        using definition_category = givm::skill_view;
        std::array<givm::effect<Category>, 4>* results;
        std::string_view name() const { return "EffectForms"; }

        auto compile(givm::definition_compile_context& context) const
        {
            const std::array<givm::any_command, 1> commands{ givm::modify_energy{} };
            (*results)[0] = context.add_effect<Category>(std::span<const givm::any_command>{ commands });
            (*results)[1] = context.add_effect<Category>(std::tuple{ givm::modify_energy{} });
            (*results)[2] = context.add_effect<Category>(givm::modify_energy{});
            if constexpr(Category == givm::event_category::normal)
                (*results)[3] = context.add_normal_effect(std::span<const givm::any_command>{ commands });
            else if constexpr(Category == givm::event_category::immediate)
                (*results)[3] = context.add_immediate_effect(std::span<const givm::any_command>{ commands });
            else (*results)[3] = context.add_preview_effect(std::span<const givm::any_command>{ commands });
            return *results;
        }
    };
}

TEST_CASE("effect registration preserves categories for spans sequences and individual commands", "[effect][compile][linkage]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto check = [&]<givm::event_category Category>()
    {
        std::array<givm::effect<Category>, 4> entries;
        source<Category> definition{ &entries };
        auto sources = givm_test::make_source_library();
        REQUIRE(sources.add(definition));
        REQUIRE(givm::compile(sources, givm_test::basic_sources, std::tuple{}, std::tuple{}, mode));
        for(const auto entry : entries) CHECK(entry);
    };
    check.template operator()<givm::event_category::normal>();
    check.template operator()<givm::event_category::immediate>();
    check.template operator()<givm::event_category::preview>();
}
}
