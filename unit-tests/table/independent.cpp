#include <givm/table.hpp>

#if defined(GIVM_DEFINITION_HPP) || defined(GIVM_DEFINITION_ISSUED_ID_HPP) \
    || defined(GIVM_DEFINITION_SOURCE_VIEW_HPP) || defined(GIVM_DEFINITION_LIBRARY_HPP) \
    || defined(GIVM_EXECUTOR_HPP) || defined(GIVM_EXECUTOR_EXECUTOR_HPP)
#error "table.hpp must not include definition or executor headers"
#endif

#include <concepts>
#include <cstddef>
#include <utility>

#include <catch2/catch_test_macros.hpp>

using namespace givm;

namespace
{
    template<class T>
    concept exposes_definition_library = requires(const T& value) { value.definition_library(); };

    template<class T>
    concept resolves_definition = requires(const T& value) { value.definition(); };
}

TEST_CASE("table constructs and copies without a definition library", "[table][definition-id]")
{
    STATIC_REQUIRE(std::same_as<definition_id<card_definition>, issued_id<card_definition>>);
    STATIC_REQUIRE(std::same_as<definition_id<character_view>, issued_id<character_view>>);
    STATIC_REQUIRE(not std::constructible_from<definition_id<card_definition>, std::size_t>);
    STATIC_REQUIRE(not std::constructible_from<definition_id<character_view>, std::size_t>);
    STATIC_REQUIRE(not definition_id<card_definition>{}.is_valid());

    const linked_deck deck{};
    card_table table{ game_parameters{ .hand_limit = 2 } };
    table.load_deck(player_id{ 0 }, deck);

    STATIC_REQUIRE(not exposes_definition_library<card_table>);
    STATIC_REQUIRE(not resolves_definition<deck_card_view>);
    STATIC_REQUIRE(not resolves_definition<character_view>);

    auto copy = table;
    table.state().round_number = 1;
    const auto copied_player = std::as_const(copy)[player_id{ 0 }];
    CHECK(copy.parameters().hand_limit == 2);
    CHECK(copy.state().round_number == 0);
    CHECK_FALSE(copied_player.state().active_character.has_value());
    CHECK(copied_player.deck_card_count() == 0);
    CHECK(copied_player.characters().empty());
}
