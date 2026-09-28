#include "../test_source_library.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <givm/givm.hpp>

namespace givm_test::definition::history_compile_errors
{
namespace
{
    template<class T>
    concept has_count_member = requires(T value) { value.count; };

    template<class T>
    concept has_shape_member = requires(T value) { value.is_array; };

    struct layout_source
    {
        using definition_category = givm::history_summary_definition;
        std::string_view source_name;
        givm::history_summary_layout fields;

        std::string_view name() const noexcept { return source_name; }
        const givm::history_summary_layout& layout(const givm::definition_compile_context&) const noexcept
        {
            return fields;
        }
        int compile(givm::definition_compile_context&) const noexcept { return 0; }
    };

    struct premature_access_source
    {
        using definition_category = givm::history_summary_definition;

        std::string_view name() const noexcept { return "PrematureAccess"; }
        auto layout(const givm::definition_compile_context& context) const
        {
            (void)context.history_field<std::uint32_t>("value");
            (void)context.resolve_history_field<std::uint32_t>(name(), "value");
            return givm::history_summary_layout{ givm::history_field<std::uint32_t>("value") };
        }
        int compile(givm::definition_compile_context&) const noexcept { return 0; }
    };

    struct invalid_lookup_source
    {
        using definition_category = givm::card_definition;
        std::vector<std::string_view>* completed;

        std::string_view name() const noexcept { return "AInvalidLookup"; }
        auto history_summary_dependencies() const { return std::array<std::string_view, 1>{ "ValidHistory" }; }
        int compile(givm::definition_compile_context& context) const
        {
            (void)context.resolve_history_field<std::uint64_t>("ValidHistory", "scalar");
            (void)context.resolve_history_field<std::uint32_t>("ValidHistory", "array");
            (void)context.resolve_history_field<std::uint32_t>("ValidHistory", "missing");
            (void)context.history_field<std::uint32_t>("scalar");
            completed->push_back(name());
            return 0;
        }
    };

    struct subsequent_source
    {
        using definition_category = givm::card_definition;
        std::vector<std::string_view>* completed;

        std::string_view name() const noexcept { return "ZSubsequentSource"; }
        int compile(givm::definition_compile_context&) const
        {
            completed->push_back(name());
            return 0;
        }
    };

    struct visited_summary_source
    {
        using definition_category = givm::history_summary_definition;
        using definition_type = std::array<givm::dynamic_history_field, 2>;

        std::string_view name() const noexcept { return "VisitedHistory"; }
        auto layout(const givm::definition_compile_context&) const
        {
            return givm::history_summary_layout{
                givm::history_scalar_field<std::uint32_t>{ "scalar" },
                givm::history_array_field<std::int16_t>{ "array", 2 }
            };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.history_field("scalar"), context.history_field("array") };
        }
        static void handle(const definition_type& fields, givm::history_summary_state state,
            const givm::history_summary_initialization&, const givm::table&, const givm::definition_library&)
        {
            for(const auto& field : fields)
                std::visit([&](auto key)
                {
                    using value_type = typename decltype(key)::value_type;
                    if constexpr(std::is_unbounded_array_v<value_type>)
                        std::ranges::fill(state[key], static_cast<std::remove_extent_t<value_type>>(9));
                    else
                        state[key] = static_cast<value_type>(7);
                }, field);
        }
    };
}

TEST_CASE("history declarations and dynamic keys retain scalar and array types", "[history][compile]")
{
    using scalar = givm::history_scalar_field<std::uint32_t>;
    using array = givm::history_array_field<std::uint32_t>;
    STATIC_REQUIRE_FALSE(has_count_member<scalar>);
    STATIC_REQUIRE_FALSE(has_shape_member<scalar>);
    STATIC_REQUIRE_FALSE(has_shape_member<array>);
    STATIC_REQUIRE_FALSE(std::constructible_from<scalar, std::string, std::size_t>);
    STATIC_REQUIRE(has_count_member<array>);
    STATIC_REQUIRE(std::same_as<decltype(givm::history_field<std::uint32_t>("value")), scalar>);
    STATIC_REQUIRE(std::same_as<decltype(givm::history_array<std::uint32_t>("values", 2)), array>);

    const visited_summary_source summary;
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(summary));
    auto result = compile(sources, givm_test::basic_sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    REQUIRE(result);
    const auto id = result->id_map.get_id<givm::history_summary_definition>(summary.name());
    givm::table table;
    load_deck(table, result->library, {}, {});
    givm_test::executor_driver executor;
    executor.start(result->library, table);

    const auto scalar_key = result->library.history_field(id, "scalar");
    const auto array_key = result->library.history_field(id, "array");
    CHECK(std::holds_alternative<givm::history_value_key<std::uint32_t>>(scalar_key));
    CHECK(std::holds_alternative<givm::history_value_key<std::int16_t[]>>(array_key));
    const auto sum = [&](const givm::dynamic_history_value& field)
    {
        return std::visit([&](auto key) -> std::uint64_t
        {
            if constexpr(std::is_unbounded_array_v<typename decltype(key)::value_type>)
            {
                std::uint64_t value = 0;
                for(const auto item : table[key]) value += static_cast<std::uint64_t>(item);
                return value;
            }
            else
                return static_cast<std::uint64_t>(table[key]);
        }, field);
    };
    CHECK(sum(scalar_key) == 7);
    CHECK(sum(array_key) == 18);
}

TEST_CASE("history layout collects independent errors with original declaration indices", "[history][compile-error]")
{
    constexpr auto huge_count = std::numeric_limits<std::size_t>::max() / sizeof(std::uint64_t) + 1;
    const layout_source summary{ "InvalidLayout", {
        givm::history_field<std::uint8_t>("first"),
        givm::history_field<std::uint8_t>("first"),
        givm::history_field<std::uint8_t>("second"),
        givm::history_field<std::uint8_t>("second"),
        givm::history_field<std::uint8_t>(""),
        givm::history_array<std::uint64_t>("huge", huge_count)
    } };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(summary));
    const auto result = compile(sources, givm_test::basic_sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    REQUIRE_FALSE(result);
    REQUIRE(result.error().size() == 4);
    std::size_t duplicates = 0;
    std::size_t empty_names = 0;
    std::size_t overflows = 0;
    for(const auto& error : result.error())
    {
        CHECK(error.location.stage == givm::compile_stage::history_layout);
        REQUIRE(error.location.source);
        CHECK(error.location.source->name == "InvalidLayout");
        if(const auto* duplicate = std::get_if<givm::history_field_duplicate_name>(&error.reason))
        {
            ++duplicates;
            CHECK((duplicate->field == "first" || duplicate->field == "second"));
            CHECK(duplicate->first_index == (duplicate->field == "first" ? 0 : 2));
            CHECK(duplicate->repeated_index == duplicate->first_index + 1);
        }
        else if(const auto* empty = std::get_if<givm::history_field_empty_name>(&error.reason))
        {
            ++empty_names;
            CHECK(empty->field_index == 4);
        }
        else if(const auto* overflow = std::get_if<givm::history_field_layout_overflow>(&error.reason))
        {
            ++overflows;
            CHECK(overflow->field == "huge");
            CHECK(overflow->field_index == 5);
            CHECK(overflow->count == huge_count);
            CHECK(overflow->element_size == sizeof(std::uint64_t));
        }
        else
            FAIL("unexpected history layout diagnostic");
    }
    CHECK(duplicates == 2);
    CHECK(empty_names == 1);
    CHECK(overflows == 1);
}

TEST_CASE("history summaries report combined storage overflow without allocating history", "[history][compile-error]")
{
    constexpr auto count = std::numeric_limits<std::size_t>::max() / 2 + 1;
    const layout_source first{ "FirstHugeHistory", { givm::history_array<std::uint8_t>("bytes", count) } };
    const layout_source second{ "SecondHugeHistory", { givm::history_array<std::uint8_t>("bytes", count) } };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(first, second));
    const auto result = compile(sources, givm_test::basic_sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    REQUIRE_FALSE(result);
    REQUIRE(result.error().size() == 1);
    const auto& error = result.error().front();
    CHECK(error.location.stage == givm::compile_stage::history_layout);
    REQUIRE(error.location.source);
    CHECK(error.location.source->name == "SecondHugeHistory");
    const auto* overflow = std::get_if<givm::history_storage_layout_overflow>(&error.reason);
    REQUIRE(overflow);
    CHECK(overflow->preceding_size == count);
    CHECK(overflow->summary_size == count);
}

TEST_CASE("history fields cannot be resolved while declaring layouts", "[history][compile-error]")
{
    const premature_access_source summary;
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(summary));
    const auto result = compile(sources, givm_test::basic_sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    REQUIRE_FALSE(result);
    REQUIRE(result.error().size() == 2);
    for(const auto& error : result.error())
    {
        CHECK(error.location.stage == givm::compile_stage::history_layout);
        REQUIRE(error.location.source);
        CHECK(error.location.source->name == "PrematureAccess");
        const auto* access = std::get_if<givm::history_field_access_error>(&error.reason);
        REQUIRE(access);
        CHECK(access->cause == givm::history_field_access_error::reason::layouts_unavailable);
        CHECK(access->summary == "PrematureAccess");
        CHECK(access->field == "value");
    }
}

TEST_CASE("invalid history lookups collect errors and allow later definitions to compile", "[history][compile-error]")
{
    std::vector<std::string_view> completed;
    const invalid_lookup_source invalid{ &completed };
    const subsequent_source subsequent{ &completed };
    const layout_source summary{ "ValidHistory", {
        givm::history_field<std::uint32_t>("scalar"), givm::history_array<std::uint32_t>("array", 2)
    } };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(invalid, subsequent, summary));
    const auto result = compile(sources, givm_test::basic_sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    REQUIRE_FALSE(result);
    REQUIRE(result.error().size() == 4);
    CHECK(bool(completed == std::vector<std::string_view>{ "AInvalidLookup", "ZSubsequentSource" }));
    std::size_t mismatches = 0;
    std::size_t missing_fields = 0;
    std::size_t invalid_accesses = 0;
    for(const auto& error : result.error())
    {
        CHECK(error.location.stage == givm::compile_stage::definition);
        REQUIRE(error.location.source);
        CHECK(error.location.source->category_index == givm::definition_types::index_of<givm::card_definition>());
        CHECK(error.location.source->name == "AInvalidLookup");
        if(const auto* mismatch = std::get_if<givm::history_field_type_mismatch>(&error.reason))
        {
            ++mismatches;
            CHECK(mismatch->summary == "ValidHistory");
            CHECK(mismatch->expected_type == (mismatch->field == "scalar" ? "uint64_t" : "uint32_t"));
            CHECK(mismatch->actual_type == (mismatch->field == "scalar" ? "uint32_t" : "uint32_t[]"));
        }
        else if(const auto* missing = std::get_if<givm::history_field_not_found>(&error.reason))
        {
            ++missing_fields;
            CHECK(missing->summary == "ValidHistory");
            CHECK(missing->field == "missing");
        }
        else if(const auto* access = std::get_if<givm::history_field_access_error>(&error.reason))
        {
            ++invalid_accesses;
            CHECK(access->cause == givm::history_field_access_error::reason::no_current_summary);
            CHECK(access->field == "scalar");
        }
        else
            FAIL("unexpected history lookup diagnostic");
    }
    CHECK(mismatches == 2);
    CHECK(missing_fields == 1);
    CHECK(invalid_accesses == 1);
}
}
