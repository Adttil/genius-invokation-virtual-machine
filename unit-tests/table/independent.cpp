#include <givm/table.hpp>

#include <concepts>
#include <cstddef>
#include <ranges>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

using namespace givm;

namespace
{
    template<class T>
    concept exposes_definition_library = requires(const T& value) { value.definition_library(); };

    template<class T>
    concept resolves_definition = requires(const T& value) { value.definition(); };

    template<class T>
    concept erases_entity = requires(T value) { value.erase(); };

    template<class T>
    concept adds_hand_card = requires(T value, definition_id<card_definition> id)
    {
        value.add_hand_card(id, card_state{});
    };

    template<class TId, class TView>
    constexpr bool table_exposes_read_only_view =
        std::same_as<decltype(std::declval<card_table&>()[TId{}]), TView>
        && std::same_as<decltype(std::declval<const card_table&>()[TId{}]), TView>
        && std::is_const_v<std::remove_reference_t<decltype(std::declval<TView>().state())>>
        && not erases_entity<TView>;
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
    table.clean_up();
    const auto copied_player = std::as_const(copy)[player_id{ 0 }];
    CHECK(copy.parameters().hand_limit == 2);
    CHECK(copy.state().round_number == 0);
    CHECK_FALSE(copied_player.state().active_character.has_value());
    CHECK(copied_player.deck_card_count() == 0);
    CHECK(copied_player.characters().empty());
}

TEST_CASE("public table access remains read only through nested views", "[table][public-interface]")
{
    STATIC_REQUIRE(std::same_as<decltype(std::declval<card_table&>().state()), const table_state&>);
    STATIC_REQUIRE(std::same_as<decltype(std::declval<card_table&>().parameters()), const game_parameters&>);
    STATIC_REQUIRE(table_exposes_read_only_view<player_id, player_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<character_id, character_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<skill_id, skill_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<attachment_id, attachment_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<hand_card_id, hand_card_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<deck_card_id, deck_card_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<hand_card_status_id, hand_card_status_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<deck_card_status_id, deck_card_status_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<support_id, support_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<summon_id, summon_view>);
    STATIC_REQUIRE(table_exposes_read_only_view<combat_status_id, combat_status_view>);
    STATIC_REQUIRE(not adds_hand_card<player_view>);

    STATIC_REQUIRE(std::same_as<std::ranges::range_value_t<decltype(std::declval<card_table&>().players())>, player_view>);
    STATIC_REQUIRE(std::same_as<std::ranges::range_value_t<decltype(std::declval<player_view>().characters())>, character_view>);
    STATIC_REQUIRE(std::same_as<std::ranges::range_value_t<decltype(std::declval<character_view>().skills())>, skill_view>);
    STATIC_REQUIRE(std::same_as<std::ranges::range_value_t<decltype(std::declval<character_view>().attachments())>, attachment_view>);
    STATIC_REQUIRE(std::same_as<decltype(std::declval<character_view>().player()), player_view>);
    STATIC_REQUIRE(std::same_as<decltype(std::declval<skill_view>().character()), character_view>);
    STATIC_REQUIRE(std::same_as<decltype(std::declval<hand_card_status_view>().card()), hand_card_view>);
    STATIC_REQUIRE(std::same_as<decltype(std::declval<deck_card_status_view>().card()), deck_card_view>);

}
