#include <concepts>
#include <cstddef>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition/source_view.hpp>

using namespace givm;

namespace
{
    struct event_context{};
    struct other_context{};
}

TEST_CASE("program entries preserve their context and named terminal states", "[definition][program]")
{
    using entry_type = program_entry<event_context>;

    STATIC_REQUIRE(std::regular<entry_type>);
    STATIC_REQUIRE(std::is_trivially_copyable_v<entry_type>);
    STATIC_REQUIRE(not std::constructible_from<entry_type, std::size_t>);
    STATIC_REQUIRE(not std::constructible_from<entry_type, program_entry<other_context>>);
    STATIC_REQUIRE(not std::convertible_to<entry_type, program_entry<other_context>>);

    constexpr entry_type default_entry;
    constexpr auto null_entry = entry_type::null();
    constexpr auto player_0_win = entry_type::player_0_win();
    constexpr auto player_1_win = entry_type::player_1_win();
    constexpr auto both_loss = entry_type::both_loss();

    STATIC_REQUIRE(default_entry == null_entry);
    STATIC_REQUIRE(default_entry.is_null());
    STATIC_REQUIRE(not static_cast<bool>(default_entry));

    STATIC_REQUIRE(not player_0_win.is_null());
    STATIC_REQUIRE(not player_1_win.is_null());
    STATIC_REQUIRE(not both_loss.is_null());
    STATIC_REQUIRE(static_cast<bool>(player_0_win));
    STATIC_REQUIRE(player_0_win != player_1_win);
    STATIC_REQUIRE(player_0_win != both_loss);
    STATIC_REQUIRE(player_1_win != both_loss);
}
