#include <givm/table.hpp>

#include <concepts>
#include <cstddef>
#include <ranges>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

namespace
{
    template<class T>
    concept exposes_definition_library = requires(const T& value) { value.definition_library(); };

    template<class T>
    concept resolves_definition = requires(const T& value) { value.definition(); };

    template<class T>
    concept erases_entity = requires(T value) { value.erase(); };

    template<class T>
    concept adds_hand_card = requires(T value, givm::definition_id<givm::card_definition> id)
    {
        value.add_hand_card(id, givm::card_state{});
    };

    template<class TId, class TView>
    constexpr bool table_exposes_read_only_view =
        std::same_as<decltype(std::declval<givm::table&>()[TId{}]), TView>
        && std::same_as<decltype(std::declval<const givm::table&>()[TId{}]), TView>
        && std::is_const_v<std::remove_reference_t<decltype(std::declval<TView>().state())>>
        && not erases_entity<TView>;
}

TEST_CASE("table constructs and copies without a definition library", "[table][definition-id]")
{
    STATIC_REQUIRE(std::same_as<givm::definition_id<givm::card_definition>, givm::issued_id<givm::card_definition>>);
    STATIC_REQUIRE(std::same_as<givm::definition_id<givm::character_view>, givm::issued_id<givm::character_view>>);
    STATIC_REQUIRE(not std::constructible_from<givm::definition_id<givm::card_definition>, std::size_t>);
    STATIC_REQUIRE(not std::constructible_from<givm::definition_id<givm::character_view>, std::size_t>);
    STATIC_REQUIRE(not givm::definition_id<givm::card_definition>{}.is_valid());

    givm::table table{ givm::game_parameters{ .hand_limit = 2 } };

    STATIC_REQUIRE(not exposes_definition_library<givm::table>);
    STATIC_REQUIRE(not resolves_definition<givm::deck_card_view>);
    STATIC_REQUIRE(not resolves_definition<givm::character_view>);

    auto copy = table;
    table.clean_up();
    const auto copied_player = std::as_const(copy)[givm::player_id{ 0 }];
    CHECK(copy.parameters().hand_limit == 2);
    CHECK(copy.state().round_number == 0);
    CHECK_FALSE(copied_player.state().active_character.has_value());
    CHECK(copied_player.deck_card_count() == 0);
    CHECK(copied_player.characters().empty());
}

TEST_CASE("public table access remains read only through nested views", "[table][public-interface]")
{
    STATIC_REQUIRE(std::same_as<decltype(std::declval<givm::table&>().state()), const givm::table_state&>);
    STATIC_REQUIRE(std::same_as<decltype(std::declval<givm::table&>().parameters()), const givm::game_parameters&>);
    STATIC_REQUIRE(table_exposes_read_only_view<givm::player_id, givm::player_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<givm::character_id, givm::character_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<givm::skill_id, givm::skill_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<givm::attachment_id, givm::attachment_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<givm::hand_card_id, givm::hand_card_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<givm::deck_card_id, givm::deck_card_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<givm::hand_card_status_id, givm::hand_card_status_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<givm::deck_card_status_id, givm::deck_card_status_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<givm::support_id, givm::support_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<givm::summon_id, givm::summon_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<givm::combat_status_id, givm::combat_status_view>);
    STATIC_REQUIRE(not adds_hand_card<givm::player_view>);

    STATIC_REQUIRE(std::same_as<std::ranges::range_value_t<decltype(std::declval<givm::table&>().players())>, givm::player_view>);
    STATIC_REQUIRE(std::same_as<std::ranges::range_value_t<decltype(std::declval<givm::player_view>().characters())>, givm::character_view>);
    STATIC_REQUIRE(std::same_as<std::ranges::range_value_t<decltype(std::declval<givm::character_view>().skills())>, givm::skill_view>);
    STATIC_REQUIRE(std::same_as<std::ranges::range_value_t<decltype(std::declval<givm::character_view>().attachments())>, givm::attachment_view>);
    STATIC_REQUIRE(std::same_as<decltype(std::declval<givm::character_view>().player()), givm::player_view>);
    STATIC_REQUIRE(std::same_as<decltype(std::declval<givm::skill_view>().character()), givm::character_view>);
    STATIC_REQUIRE(std::same_as<decltype(std::declval<givm::hand_card_status_view>().card()), givm::hand_card_view>);
    STATIC_REQUIRE(std::same_as<decltype(std::declval<givm::deck_card_status_view>().card()), givm::deck_card_view>);

}
