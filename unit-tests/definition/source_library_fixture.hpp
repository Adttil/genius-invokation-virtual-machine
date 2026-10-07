#ifndef GIVM_UNIT_TESTS_DEFINITION_SOURCE_LIBRARY_FIXTURE_HPP
#define GIVM_UNIT_TESTS_DEFINITION_SOURCE_LIBRARY_FIXTURE_HPP

#include <array>
#include <string_view>

#include <givm/definition_source_interface.hpp>

namespace givm_test::definition::source_library_linkage
{
    template<int Identity>
    struct card_source
    {
        static constexpr auto category = givm::definition_category::card;
        struct definition_type {};

        std::string_view source_name;

        constexpr std::string_view name() const noexcept { return source_name; }
        constexpr auto tags() const noexcept { return std::array<std::string_view, 1>{ "linkage_fixture" }; }
        constexpr definition_type compile(givm::definition_compile_context&) const noexcept { return {}; }
    };

    inline constexpr card_source<0> shared_card{ "SharedCard" };
    inline constexpr card_source<0> provider_card{ "ProviderCard" };
    inline constexpr card_source<0> peer_card{ "PeerCard" };
    inline constexpr card_source<1> conflicting_card{ "SharedCard" };
    inline constexpr card_source<0> rejected_card{ "RejectedCard" };
}

#endif
