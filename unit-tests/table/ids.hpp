#include <givm/table.hpp>

#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>

namespace givm_test::table::ids
{
    TEST_CASE("tag IDs distinguish an absent tag from the first tag index", "[table][id]")
    {
        STATIC_REQUIRE(std::is_trivially_default_constructible_v<givm::tag_id>);
        STATIC_REQUIRE(std::is_trivially_copyable_v<givm::optional_tag_id>);
        STATIC_REQUIRE(sizeof(givm::optional_tag_id) == sizeof(givm::tag_id));
        constexpr givm::optional_tag_id empty;
        STATIC_REQUIRE(not empty);
        givm::tag_id tag;
        tag = 0;
        givm::optional_tag_id chosen = tag;
        REQUIRE(chosen);
        CHECK(chosen.get() == tag);
        chosen = nullptr;
        CHECK_FALSE(chosen);
#ifndef NDEBUG
        CHECK_THROWS_AS(chosen.get(), std::invalid_argument);
#endif
    }

    TEST_CASE("categories map definitions to entity views at compile time", "[table][id]")
    {
        using enum givm::entity_category;
        STATIC_REQUIRE(std::same_as<givm::entity_view<character>, givm::character_view>);
        STATIC_REQUIRE(givm::deck_card_view::category == deck_card);
        STATIC_REQUIRE(givm::definition_category_of<deck_card> == givm::definition_category::card);
        STATIC_REQUIRE(givm::definition_category_of<player> == givm::definition_category::null);
        constexpr auto cards = givm::entity_categories_of<givm::definition_category::card>;
        STATIC_REQUIRE(cards.size() == 2);
        STATIC_REQUIRE(cards[0] == hand_card);
        STATIC_REQUIRE(cards[1] == deck_card);
        constexpr auto statuses = givm::entity_categories_of<givm::definition_category::card_status>;
        STATIC_REQUIRE(statuses.size() == 2);
        STATIC_REQUIRE(statuses[0] == hand_card_status);
        STATIC_REQUIRE(statuses[1] == deck_card_status);
        STATIC_REQUIRE(givm::entity_categories_of<givm::definition_category::history_summary>.empty());
        STATIC_REQUIRE(givm::entity_categories_of<givm::definition_category::null>.empty());
    }

    TEST_CASE("definition IDs preserve wide indices and explicit absence in one word", "[table][id]")
    {
        using enum givm::definition_category;
        using id = givm::variant_definition_id<card, summon>;
        using nullable_id = givm::variant_definition_id<null, card, summon>;
        STATIC_REQUIRE(sizeof(givm::card_definition_id) == sizeof(std::uint64_t));
        STATIC_REQUIRE(sizeof(id) == sizeof(std::uint64_t));
        STATIC_REQUIRE(sizeof(nullable_id) == sizeof(std::uint64_t));
        STATIC_REQUIRE(std::is_trivially_default_constructible_v<givm::card_definition_id>);
        STATIC_REQUIRE(std::is_trivially_default_constructible_v<id>);
        STATIC_REQUIRE(std::is_trivially_copyable_v<nullable_id>);
        constexpr nullable_id constant_empty;
        STATIC_REQUIRE(not constant_empty);
        STATIC_REQUIRE(not std::convertible_to<givm::definition_id<summon>, givm::card_definition_id>);

        givm::card_definition_id card_id;
        card_id = (std::uint64_t{ 1 } << 40) + 7;
        const id value = card_id;
        CHECK(value.category() == card);
        CHECK(value.get<card>() == card_id);
        CHECK(value.holds<givm::card_definition_id>());
        CHECK(value.get<givm::card_definition_id>() == card_id);
        CHECK(value.get_if<card>().get() == card_id);
        CHECK_FALSE(value.get_if<summon>());
        CHECK(id{ value.value() } == value);
        const nullable_id widened = value;
        CHECK(widened.get<card>() == card_id);
        nullable_id absent;
        CHECK_FALSE(absent);
        CHECK(absent.holds<null>());
        CHECK(absent.holds<std::nullptr_t>());
        CHECK(nullable_id{ absent.value() } == absent);
        CHECK(absent.visit([](auto value) { return std::is_same_v<decltype(value), std::nullptr_t>; }));
        absent = card_id;
        CHECK(absent);
        CHECK(absent.get<card>() == card_id);
    }

    TEST_CASE("entity IDs preserve parent and full child indices in one word", "[table][id]")
    {
        using enum givm::entity_category;
        using id = givm::variant_entity_id<skill, hand_card_status, deck_card_status, summon>;
        STATIC_REQUIRE(sizeof(givm::player_id) == sizeof(std::uint64_t));
        STATIC_REQUIRE(sizeof(givm::character_id) == sizeof(std::uint64_t));
        STATIC_REQUIRE(sizeof(givm::skill_id) == sizeof(std::uint64_t));
        STATIC_REQUIRE(sizeof(givm::hand_card_status_id) == sizeof(std::uint64_t));
        STATIC_REQUIRE(sizeof(id) == sizeof(std::uint64_t));
        STATIC_REQUIRE(sizeof(givm::optional_character_id) == sizeof(std::uint64_t));
        STATIC_REQUIRE(std::is_trivially_default_constructible_v<givm::skill_id>);
        STATIC_REQUIRE(std::is_trivially_copyable_v<id>);
        STATIC_REQUIRE_FALSE(std::is_constructible_v<givm::skill_id, std::uint64_t>);
        STATIC_REQUIRE_FALSE(std::is_constructible_v<id, std::uint64_t>);
        STATIC_REQUIRE_FALSE(std::is_assignable_v<givm::skill_id&, std::uint64_t>);
        STATIC_REQUIRE_FALSE(std::is_assignable_v<id&, std::uint64_t>);
        constexpr givm::optional_character_id constant_empty;
        STATIC_REQUIRE(not constant_empty);
        constexpr auto index = std::numeric_limits<std::uint32_t>::max();
        constexpr givm::player_id owner{ 1 };
        constexpr givm::character_id parent{ owner, 37 };
        constexpr givm::skill_id child{ parent, index };
        STATIC_REQUIRE(child.character_id() == parent);
        STATIC_REQUIRE(child.player_id() == owner);
        STATIC_REQUIRE(child.index() == index);
        constexpr givm::hand_card_id hand_owner{ owner, 123 };
        constexpr givm::deck_card_id deck_owner{ owner, 456 };
        const givm::hand_card_status_id hand_status{ hand_owner, index };
        const givm::deck_card_status_id deck_status{ deck_owner, index };
        CHECK(hand_status.hand_card_id() == hand_owner);
        CHECK(deck_status.deck_card_id() == deck_owner);
        CHECK(hand_status.index() == index);
        CHECK(deck_status.index() == index);
        for(const id value : { id{ child }, id{ hand_status }, id{ deck_status } })
        {
            CHECK(value.player_id() == owner);
            CHECK(value.visit([](auto id) { return id.index(); }) == index);
        }
        const id value = child;
        CHECK(value.get<skill>() == child);
        CHECK(value.holds<givm::skill_id>());
        CHECK(value.get<givm::skill_id>() == child);
        CHECK(value.get<givm::skill_id>().value() == child.value());
        CHECK(value.get_if<skill>().get() == child);
        CHECK_FALSE(value.get_if<summon>());
        givm::optional_character_id active;
        CHECK_FALSE(active);
        CHECK(active.holds<std::nullptr_t>());
        active = parent;
        CHECK(active.get() == parent);
        CHECK(*active == parent);
        active = nullptr;
        CHECK_FALSE(active);
    }

#ifndef NDEBUG
    TEST_CASE("debug ID access rejects incompatible categories and representations", "[table][id][debug]")
    {
        using enum givm::definition_category;
        const givm::variant_definition_id<card, summon> value = givm::card_definition_id{ 7 };
        CHECK_THROWS_AS(value.get<summon>(), std::invalid_argument);
        const givm::optional_character_id empty{};
        CHECK_THROWS_AS(empty.get(), std::invalid_argument);
        CHECK_THROWS_AS(givm::card_definition_id{ std::numeric_limits<std::uint64_t>::max() }, std::invalid_argument);
        const givm::character_id parent{ givm::player_id{ 0 }, std::numeric_limits<std::uint32_t>::max() };
        CHECK_THROWS_AS((givm::skill_id{ parent, 0 }), std::invalid_argument);
        CHECK_THROWS_AS(value.get<givm::summon_definition_id>(), std::invalid_argument);
        CHECK_THROWS_AS(empty.get<givm::character_id>(), std::invalid_argument);
    }
#endif
}
