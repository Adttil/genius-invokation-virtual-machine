#include "../test_source_library.hpp"

#include <givm/definition.hpp>

#include <algorithm>
#include <concepts>
#include <initializer_list>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace
{
    template<class TCategory>
    struct named_source
    {
        using definition_category = TCategory;

        struct definition_type{};

        std::string_view source_name;

        std::string_view name() const noexcept
        {
            return source_name;
        }

        definition_type compile(givm::definition_compile_context&) const
        {
            return {};
        }
    };

    template<class TCategory>
    void check_sources(
        const givm::definition_source_library& sources,
        std::initializer_list<std::string_view> expected_names
    )
    {
        const auto views = sources.source_views<TCategory>();
        STATIC_REQUIRE(std::ranges::input_range<decltype(views)>);
        STATIC_REQUIRE(std::same_as<
            std::ranges::range_value_t<decltype(views)>,
            givm::definition_source_view<TCategory>
        >);

        std::vector<std::string_view> names;
        for(const auto view : views)
        {
            names.push_back(view.name());
            CHECK(sources.has<TCategory>(view.name()));
        }
        std::ranges::sort(names);
        CHECK(bool(names == std::vector<std::string_view>{ expected_names }));
    }
}

TEST_CASE("program entries can be constructed without executor headers", "[definition][program]")
{
    using entry_type = givm::program_entry;
    const entry_type entry{};

    CHECK(entry.is_null());
}

TEST_CASE("query parameters and defaults are available without executor headers", "[definition][query]")
{
    STATIC_REQUIRE(std::same_as<givm::character_initial_state::result_t, givm::character_state>);
    STATIC_REQUIRE(std::same_as<givm::card_initial_state::result_t, givm::card_state>);
    STATIC_REQUIRE(std::same_as<givm::card_state_modification::result_t, void>);
    STATIC_REQUIRE(std::same_as<decltype(givm::card_state_modification::status), const givm::status_state&>);
    STATIC_REQUIRE(std::same_as<givm::card_target_validation::result_t, givm::target_validation>);
    CHECK(query_default(givm::character_initial_state{}).health == 0);
    CHECK(query_default(givm::card_initial_state{}).cost.speed == givm::action_speed::fast);
}

TEST_CASE("definition sources can be registered and enumerated without executor headers", "[definition][source_library]")
{
    STATIC_REQUIRE_FALSE(std::is_default_constructible_v<givm::definition_source_library>);
    const named_source<givm::card_definition> card{ "Card" };
    const named_source<givm::card_definition> another_card{ "Another card" };
    const named_source<givm::status_definition> status{ "Status" };
    const named_source<givm::support_view> support{ "Support" };
    const named_source<givm::summon_view> summon{ "Summon" };
    const named_source<givm::combat_status_view> combat_status{ "Combat status" };
    const named_source<givm::character_view> character{ "Character" };
    const named_source<givm::skill_view> skill{ "Skill" };
    const named_source<givm::attachment_view> attachment{ "Attachment" };

    auto sources = givm_test::make_source_library();
    check_sources<givm::card_definition>(sources, {});
    REQUIRE(sources.add(card, status, support, summon, combat_status, character, skill, attachment));
    REQUIRE(sources.add(another_card));

    check_sources<givm::card_definition>(sources, { "Another card", "Card" });
    check_sources<givm::status_definition>(sources, { "Status" });
    check_sources<givm::support_view>(sources, { "Support" });
    check_sources<givm::summon_view>(sources, { "Summon", "TestBurningFlame" });
    check_sources<givm::combat_status_view>(sources, { "Combat status", "TestCatalyzingField", "TestDendroCore" });
    check_sources<givm::character_view>(sources, { "Character" });
    check_sources<givm::skill_view>(sources, { "Skill" });
    check_sources<givm::attachment_view>(sources, { "Attachment", "TestFrozen" });

    const auto ids = sources.make_issued_id_map();
    CHECK(ids.has<givm::card_definition>("Another card"));
    CHECK(ids.has<givm::character_view>("Character"));
    CHECK(ids.has<givm::attachment_view>("Attachment"));
}

TEST_CASE("reaction definitions can be bound without executor headers", "[definition][source_library]")
{
    const named_source<givm::combat_status_view> core{ "Custom core" };
    const named_source<givm::combat_status_view> field{ "Custom field" };
    const named_source<givm::summon_view> flame{ "Custom flame" };
    const givm::definition_source_library sources{ core, field, flame, givm_test::frozen };

    check_sources<givm::combat_status_view>(sources, { "Custom core", "Custom field" });
    check_sources<givm::summon_view>(sources, { "Custom flame" });
    const auto ids = sources.make_issued_id_map(givm::definition_selection{});
    CHECK(ids.has<givm::combat_status_view>(core.name()));
    CHECK(ids.has<givm::combat_status_view>(field.name()));
    CHECK(ids.has<givm::summon_view>(flame.name()));
}
