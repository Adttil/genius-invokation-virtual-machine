#include "../test_source_library.hpp"

#include <array>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition.hpp>
#include <givm/executor.hpp>

namespace
{
    template<class TDefinition>
    struct plain_source
    {
        using definition_category = TDefinition;

        struct definition_type{};

        std::string_view source_name;
        std::array<std::string_view, 2> source_tags{};
        std::size_t tag_count = 0;

        constexpr std::string_view name() const noexcept
        {
            return source_name;
        }

        constexpr std::span<const std::string_view> tags() const noexcept
        {
            return { source_tags.data(), tag_count };
        }

        constexpr definition_type compile(givm::definition_compile_context&) const noexcept
        {
            return {};
        }
    };

    struct card_with_support_dependency
    {
        using definition_category = givm::card_definition;

        struct definition_type
        {
            givm::definition_id<givm::support_view> support;
        };

        std::string_view source_name;
        std::string_view support_name;

        constexpr std::string_view name() const noexcept
        {
            return source_name;
        }

        constexpr auto support_dependencies() const noexcept
        {
            return std::array{ support_name };
        }

        definition_type compile(givm::definition_compile_context& context) const
        {
            return { .support = context.resolve_id<givm::support_view>(support_name) };
        }
    };

    struct card_with_tag_dependency
    {
        using definition_category = givm::card_definition;

        struct definition_type
        {
            std::vector<givm::definition_id<givm::card_definition>> cards;
        };

        std::string_view source_name;
        std::string_view filter;

        constexpr std::string_view name() const noexcept
        {
            return source_name;
        }

        constexpr auto card_dependencies_by_tag() const noexcept
        {
            return std::array{ filter };
        }

        definition_type compile(givm::definition_compile_context& context) const
        {
            return {
                .cards = context.resolve_ids_by_tag<givm::card_definition>(filter)
            };
        }
    };

    struct core_with_support_dependency
    {
        using definition_category = givm::combat_status_view;

        struct definition_type
        {
            givm::definition_id<givm::support_view> support;
        };

        std::string_view source_name;
        std::string_view support_name;

        constexpr std::string_view name() const noexcept
        {
            return source_name;
        }

        constexpr auto support_dependencies() const noexcept
        {
            return std::array{ support_name };
        }

        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_id<givm::support_view>(support_name) };
        }
    };
}

TEST_CASE("definition_source_library adds a dependent batch atomically", "[source_library]")
{
    const card_with_support_dependency card{ "Card", "Support" };
    const plain_source<givm::support_view> support{ .source_name = "Support" };

    auto library = givm_test::make_source_library();
    CHECK_FALSE(library.add(card));
    CHECK_FALSE(library.has<givm::card_definition>("Card"));

    REQUIRE(library.add(card, support));
    CHECK(library.has<givm::card_definition>("Card"));
    CHECK(library.has<givm::support_view>("Support"));

    auto duplicate_batch = givm_test::make_source_library();
    const plain_source<givm::card_definition> duplicate{ .source_name = "Card" };
    CHECK_FALSE(duplicate_batch.add(card, duplicate, support));
    CHECK_FALSE(duplicate_batch.has<givm::card_definition>("Card"));
    CHECK_FALSE(duplicate_batch.has<givm::support_view>("Support"));
}

TEST_CASE("definition_source_library rejects a conflicting library without partial merge", "[source_library]")
{
    const plain_source<givm::card_definition> card{ .source_name = "Card" };
    const plain_source<givm::support_view> support{ .source_name = "Support" };
    const plain_source<givm::summon_view> summon{ .source_name = "Summon" };

    auto base = givm_test::make_source_library();
    REQUIRE(base.add(card, support));

    auto extension = givm_test::make_source_library();
    REQUIRE(extension.add(card, summon));

    CHECK_FALSE(base.add(extension));
    CHECK(base.has<givm::card_definition>("Card"));
    CHECK(base.has<givm::support_view>("Support"));
    CHECK_FALSE(base.has<givm::summon_view>("Summon"));
}

TEST_CASE("selected definitions include transitive named dependencies", "[source_library]")
{
    const card_with_support_dependency selected_card{ "Root", "Support" };
    const plain_source<givm::support_view> support{ .source_name = "Support" };
    const plain_source<givm::card_definition> unused{ .source_name = "Unused" };

    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(selected_card, support, unused));

    const std::array card_roots{ std::string_view{ "Root" } };
    givm::definition_selection selection{};
    selection[givm::definition_types::index_of<givm::card_definition>()] = card_roots;

    const auto prepared_ids = sources.make_issued_id_map(selection);
    const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
    const auto [library, id_map] = compile(sources, selection, program, program, givm::compile_mode::normal);
    CHECK(id_map.has<givm::card_definition>("Root"));
    CHECK(id_map.has<givm::support_view>("Support"));
    CHECK_FALSE(id_map.has<givm::card_definition>("Unused"));

    const auto root_id = prepared_ids.get_id<givm::card_definition>("Root");
    const auto support_id = prepared_ids.get_id<givm::support_view>("Support");
    CHECK(root_id == id_map.get_id<givm::card_definition>("Root"));
    CHECK(support_id == id_map.get_id<givm::support_view>("Support"));
    CHECK(bool(library.name(root_id) == "Root"));
    CHECK(bool(library.name(support_id) == "Support"));
}

TEST_CASE("tag dependencies select matching definitions only", "[source_library]")
{
    const card_with_tag_dependency selected_card{ "Root", "selected & !excluded" };
    const plain_source<givm::card_definition> alpha{
        .source_name = "Alpha",
        .source_tags = { "selected", "ordinary" },
        .tag_count = 2
    };
    const plain_source<givm::card_definition> beta{
        .source_name = "Beta",
        .source_tags = { "selected", "excluded" },
        .tag_count = 2
    };
    const plain_source<givm::card_definition> gamma{
        .source_name = "Gamma",
        .source_tags = { "ordinary", {} },
        .tag_count = 1
    };

    auto library = givm_test::make_source_library();
    REQUIRE(library.add(selected_card, alpha, beta, gamma));

    const std::array card_roots{ std::string_view{ "Root" } };
    givm::definition_selection selection{};
    selection[givm::definition_types::index_of<givm::card_definition>()] = card_roots;

    const auto ids = library.make_issued_id_map(selection);
    CHECK(ids.has<givm::card_definition>("Root"));
    CHECK(ids.has<givm::card_definition>("Alpha"));
    CHECK_FALSE(ids.has<givm::card_definition>("Beta"));
    CHECK_FALSE(ids.has<givm::card_definition>("Gamma"));
}

TEST_CASE("issued ids address the definitions produced by compilation", "[source_library]")
{
    const plain_source<givm::card_definition> zulu{
        .source_name = "Zulu",
        .source_tags = { "zeta", {} },
        .tag_count = 1
    };
    const plain_source<givm::card_definition> alpha{
        .source_name = "Alpha",
        .source_tags = { "alpha", {} },
        .tag_count = 1
    };

    for(const bool reverse_order : { false, true })
    {
        CAPTURE(reverse_order);
        auto sources = givm_test::make_source_library();
        REQUIRE((reverse_order ? sources.add(alpha, zulu) : sources.add(zulu, alpha)));

        const auto prepared_ids = sources.make_issued_id_map();
        const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
        const auto [library, id_map] = compile(sources, program, program, givm::compile_mode::normal);
        const auto alpha_id = prepared_ids.get_id<givm::card_definition>("Alpha");
        const auto zulu_id = prepared_ids.get_id<givm::card_definition>("Zulu");
        const auto alpha_tag = prepared_ids.get_tag_id("alpha");
        const auto zeta_tag = prepared_ids.get_tag_id("zeta");

        CHECK(alpha_id == id_map.get_id<givm::card_definition>("Alpha"));
        CHECK(zulu_id == id_map.get_id<givm::card_definition>("Zulu"));
        CHECK(alpha_tag == id_map.get_tag_id("alpha"));
        CHECK(zeta_tag == id_map.get_tag_id("zeta"));
        CHECK(bool(library.name(alpha_id) == "Alpha"));
        CHECK(bool(library.name(zulu_id) == "Zulu"));
        CHECK(bool(library.tag_name(alpha_tag) == "alpha"));
        CHECK(bool(library.tag_name(zeta_tag) == "zeta"));
    }
}

TEST_CASE("reaction bindings use the supplied definitions and survive library copies", "[source_library][reactions]")
{
    const plain_source<givm::combat_status_view> core{ .source_name = "Z custom core" };
    const plain_source<givm::combat_status_view> field{ .source_name = "A custom field" };
    const plain_source<givm::summon_view> flame{ .source_name = "Custom flame" };
    const givm::definition_source_library original{ core, field, flame };
    auto copied_sources = original;
    auto moved_sources = std::move(copied_sources);
    auto assigned_sources = givm_test::make_source_library();
    assigned_sources = original;
    auto move_assigned_sources = givm_test::make_source_library();
    move_assigned_sources = std::move(assigned_sources);

    const std::array<const givm::definition_source_library*, 3> source_libraries{
        &original, &moved_sources, &move_assigned_sources
    };
    for(const auto* sources : source_libraries)
    {
        const auto [library, ids] = compile(*sources, givm::definition_selection{},
            std::tuple{}, std::tuple{}, givm::compile_mode::normal);
        const auto check = [&](const givm::definition_library& compiled)
        {
            CHECK(compiled.dendro_core_id() == ids.get_id<givm::combat_status_view>(core.name()));
            CHECK(compiled.catalyzing_field_id() == ids.get_id<givm::combat_status_view>(field.name()));
            CHECK(compiled.burning_flame_id() == ids.get_id<givm::summon_view>(flame.name()));
            CHECK(bool(compiled.name(compiled.dendro_core_id()) == core.name()));
            CHECK(bool(compiled.name(compiled.catalyzing_field_id()) == field.name()));
            CHECK(bool(compiled.name(compiled.burning_flame_id()) == flame.name()));
        };
        check(library);
        auto copied = library;
        check(copied);
        auto moved = std::move(copied);
        check(moved);
        copied = library;
        check(copied);
        moved = std::move(copied);
        check(moved);
    }
}

TEST_CASE("partial compilation keeps reaction dependencies and excludes unrelated definitions", "[source_library][reactions]")
{
    const core_with_support_dependency core{ "Dependent core", "Reaction support" };
    const plain_source<givm::combat_status_view> field{ .source_name = "Field" };
    const plain_source<givm::summon_view> flame{ .source_name = "Flame" };
    const plain_source<givm::support_view> support{ .source_name = "Reaction support" };
    const plain_source<givm::card_definition> unused{ .source_name = "Unused card" };

    CHECK_THROWS_AS((givm::definition_source_library{ core, field, flame }), std::invalid_argument);
    const givm::definition_source_library sources{ core, field, flame, support, unused };
    const auto prepared = sources.make_issued_id_map(givm::definition_selection{});
    const auto [library, ids] = compile(sources, givm::definition_selection{},
        std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    CHECK(ids.has<givm::combat_status_view>(core.name()));
    CHECK(ids.has<givm::combat_status_view>(field.name()));
    CHECK(ids.has<givm::summon_view>(flame.name()));
    CHECK(ids.has<givm::support_view>(support.name()));
    CHECK_FALSE(ids.has<givm::card_definition>(unused.name()));
    CHECK(prepared.has<givm::support_view>(support.name()));
    CHECK_FALSE(prepared.has<givm::card_definition>(unused.name()));
    CHECK(library.dendro_core_id() == prepared.get_id<givm::combat_status_view>(core.name()));
}

TEST_CASE("merging reaction versions preserves the receiving library bindings", "[source_library][reactions]")
{
    const plain_source<givm::combat_status_view> core{ .source_name = "Core" };
    const plain_source<givm::combat_status_view> old_field{ .source_name = "Field-1" };
    const plain_source<givm::combat_status_view> new_field{ .source_name = "Field-2" };
    const plain_source<givm::summon_view> flame{ .source_name = "Flame" };
    const plain_source<givm::card_definition> card{ .source_name = "Extension card" };
    givm::definition_source_library base{ core, old_field, flame };
    const givm::definition_source_library extension{ core, new_field, flame, card };
    REQUIRE(base.add(extension));

    const auto [library, ids] = compile(base, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    CHECK(ids.has<givm::combat_status_view>(old_field.name()));
    CHECK(ids.has<givm::combat_status_view>(new_field.name()));
    CHECK(ids.has<givm::card_definition>(card.name()));
    CHECK(library.catalyzing_field_id() == ids.get_id<givm::combat_status_view>(old_field.name()));
    CHECK(library.dendro_core_id() == ids.get_id<givm::combat_status_view>(core.name()));
    CHECK(library.burning_flame_id() == ids.get_id<givm::summon_view>(flame.name()));

    const auto [selected, selected_ids] = compile(base, givm::definition_selection{},
        std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    CHECK(selected_ids.has<givm::combat_status_view>(old_field.name()));
    CHECK_FALSE(selected_ids.has<givm::combat_status_view>(new_field.name()));
    CHECK_FALSE(selected_ids.has<givm::card_definition>(card.name()));
    CHECK(selected.catalyzing_field_id() == selected_ids.get_id<givm::combat_status_view>(old_field.name()));

    const auto [other_library, other_ids] = compile(extension, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    CHECK(other_library.catalyzing_field_id() == other_ids.get_id<givm::combat_status_view>(new_field.name()));
}

TEST_CASE("a shared reaction source can also be an ordinary definition in a merged library", "[source_library][reactions]")
{
    const plain_source<givm::combat_status_view> core{ .source_name = "Core" };
    const plain_source<givm::combat_status_view> first_field{ .source_name = "First field" };
    const plain_source<givm::combat_status_view> second_field{ .source_name = "Second field" };
    const plain_source<givm::summon_view> flame{ .source_name = "Flame" };
    const givm::definition_source_library first{ core, first_field, flame };
    const givm::definition_source_library second{ core, second_field, flame, first_field };
    auto first_receives = first;
    auto second_receives = second;
    REQUIRE(first_receives.add(second));
    REQUIRE(second_receives.add(first));

    const auto [first_library, first_ids] = compile(first_receives, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    const auto [second_library, second_ids] = compile(second_receives, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    CHECK(first_library.catalyzing_field_id() == first_ids.get_id<givm::combat_status_view>(first_field.name()));
    CHECK(second_library.catalyzing_field_id() == second_ids.get_id<givm::combat_status_view>(second_field.name()));
}

TEST_CASE("distinct reaction source objects with the same name remain conflicting", "[source_library][reactions]")
{
    const plain_source<givm::combat_status_view> core{ .source_name = "Core" };
    const plain_source<givm::combat_status_view> field{ .source_name = "Field" };
    const plain_source<givm::combat_status_view> different_field{ .source_name = "Field" };
    const plain_source<givm::summon_view> flame{ .source_name = "Flame" };
    const plain_source<givm::card_definition> card{ .source_name = "Unmerged card" };
    givm::definition_source_library base{ core, field, flame };
    const givm::definition_source_library extension{ core, different_field, flame, card };

    CHECK_FALSE(base.add(extension));
    CHECK_FALSE(base.has<givm::card_definition>(card.name()));
    CHECK_THROWS_AS((givm::definition_source_library{ core, core, flame }), std::invalid_argument);
    CHECK_THROWS_AS((givm::definition_source_library{ core, field, flame, different_field }), std::invalid_argument);
}
