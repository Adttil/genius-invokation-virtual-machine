#include <array>
#include <cstddef>
#include <span>
#include <string_view>
#include <tuple>
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
}

TEST_CASE("definition_source_library adds a dependent batch atomically", "[source_library]")
{
    const card_with_support_dependency card{ "Card", "Support" };
    const plain_source<givm::support_view> support{ .source_name = "Support" };

    givm::definition_source_library library;
    CHECK_FALSE(library.add(card));
    CHECK_FALSE(library.has<givm::card_definition>("Card"));

    REQUIRE(library.add(card, support));
    CHECK(library.has<givm::card_definition>("Card"));
    CHECK(library.has<givm::support_view>("Support"));

    givm::definition_source_library duplicate_batch;
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

    givm::definition_source_library base;
    REQUIRE(base.add(card, support));

    givm::definition_source_library extension;
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

    givm::definition_source_library sources;
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

    givm::definition_source_library library;
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
        givm::definition_source_library sources;
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
