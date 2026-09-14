#include <concepts>
#include <cstddef>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition.hpp>

using namespace givm;

namespace
{
    struct event_context{};
    struct other_context{};
}

TEST_CASE("program entries preserve their context and null identity", "[definition][program]")
{
    using entry_type = program_entry<event_context>;

    STATIC_REQUIRE(std::regular<entry_type>);
    STATIC_REQUIRE(not std::constructible_from<entry_type, std::size_t>);
    STATIC_REQUIRE(not std::constructible_from<entry_type, program_entry<other_context>>);
    STATIC_REQUIRE(not std::convertible_to<entry_type, program_entry<other_context>>);

    constexpr entry_type default_entry;
    constexpr auto null_entry = entry_type::null();

    STATIC_REQUIRE(default_entry == null_entry);
    STATIC_REQUIRE(default_entry.is_null());
    STATIC_REQUIRE(not static_cast<bool>(default_entry));
}
