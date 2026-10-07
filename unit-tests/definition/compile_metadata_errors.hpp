#include <cstdint>
#include <array>
#include <cstddef>
#include <string_view>
#include <tuple>
#include <variant>

#include <catch2/catch_test_macros.hpp>
#include <givm/executor.hpp>

#include "../test_source_library.hpp"

namespace givm_test::definition::compile_metadata_errors
{
namespace
{
    struct failed_metadata_source
    {
        static constexpr auto category = givm::definition_category::support;
        struct definition_type { std::size_t* query_calls; };
        bool* finished;
        std::size_t* query_calls;

        constexpr std::string_view name() const noexcept { return "Failed metadata lookup"; }
        constexpr auto tags() const noexcept { return std::array{ std::string_view{ "actual-source-tag" } }; }

        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto missing = context.resolve_id<givm::definition_category::support>("Missing support");
            CHECK_FALSE(missing);
            const auto metadata = context[givm::definition_id<givm::definition_category::support>{ 1 }];
            CHECK_FALSE(metadata.id());
            CHECK(metadata.name().empty());
            CHECK(metadata.tags().empty());
            CHECK(metadata.dependencies<givm::definition_category::card>().empty());
            CHECK_FALSE(metadata.has_tag("actual-source-tag"));
            CHECK_FALSE(metadata.can_handle<givm::round_started>());
            CHECK_FALSE(metadata.has_query<givm::support_state_limit>());

            const auto subsequent = context.resolve_id<givm::definition_category::skill>("Missing skill");
            CHECK_FALSE(subsequent);
            *finished = true;
            return { query_calls };
        }

        static givm::normal_effect handle(const definition_type&,
            givm::round_started&, givm::handle_context<givm::support_view>&, std::uint32_t = 0)
        {
            return {};
        }

        static givm::support_state query(const definition_type& data, const givm::support_state_limit&)
        {
            ++*data.query_calls;
            return { .count = 5, .round_usages = 2 };
        }
    };
}

TEST_CASE("failed hard resolution permits empty metadata inspection and subsequent diagnostics", "[definition][compile][metadata]")
{
    bool finished = false;
    std::size_t query_calls = 0;
    const failed_metadata_source source{ &finished, &query_calls };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(source));
    const auto result = givm::compile(sources, givm_test::basic_sources,
        std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    REQUIRE_FALSE(result);
    CHECK(finished);
    CHECK(query_calls == 0);
    REQUIRE(result.error().size() == 3);
    const auto& errors = result.error();
    const auto* first = std::get_if<givm::definition_resolution_error>(&errors[0].reason);
    const auto* metadata = std::get_if<givm::definition_metadata_error>(&errors[1].reason);
    const auto* subsequent = std::get_if<givm::definition_resolution_error>(&errors[2].reason);
    REQUIRE(first);
    REQUIRE(metadata);
    REQUIRE(subsequent);
    CHECK(first->definition.name == "Missing support");
    CHECK(first->cause == givm::definition_resolution_error::reason::undeclared_dependency);
    CHECK(metadata->category == givm::definition_category::support);
    CHECK(metadata->value == 1);
    CHECK(metadata->count == 1);
    CHECK(subsequent->definition.name == "Missing skill");
    CHECK(subsequent->cause == givm::definition_resolution_error::reason::undeclared_dependency);
    for(const auto& error : errors)
    {
        CHECK(error.location.stage == givm::compile_stage::definition);
        REQUIRE(error.location.source);
        CHECK(error.location.source->name == "Failed metadata lookup");
        CHECK(error.location.source->category == givm::definition_category::support);
    }
}
}
