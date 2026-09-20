#include <concepts>
#include <cstddef>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition.hpp>

TEST_CASE("program entries preserve null identity", "[definition][program]")
{
    using entry_type = givm::program_entry;
    STATIC_REQUIRE(std::regular<entry_type>);
    STATIC_REQUIRE(not std::constructible_from<entry_type, std::size_t>);

    constexpr entry_type default_entry;
    constexpr auto null_entry = entry_type::null();
    STATIC_REQUIRE(default_entry == null_entry);
    STATIC_REQUIRE(default_entry.is_null());
    STATIC_REQUIRE(not static_cast<bool>(default_entry));
}
