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

namespace givm_test::definition::source_library
{
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
    struct reaction_bindings
    {
        givm::definition_id<givm::combat_status_view> core;
        givm::definition_id<givm::combat_status_view> field;
        givm::definition_id<givm::summon_view> flame;
        givm::definition_id<givm::attachment_view> frozen;
    };

    struct card_with_reaction_bindings
    {
        using definition_category = givm::card_definition;

        reaction_bindings* bindings;

        constexpr std::string_view name() const noexcept { return "Reaction-aware card"; }

        reaction_bindings compile(givm::definition_compile_context& context) const
        {
            *bindings = { context.dendro_core_id(), context.catalyzing_field_id(),
                context.burning_flame_id(), context.frozen_id() };
            return *bindings;
        }
    };

    struct core_with_field_dependency
    {
        using definition_category = givm::combat_status_view;

        std::string_view source_name;
        std::string_view dependency;

        constexpr std::string_view name() const noexcept { return source_name; }
        constexpr auto combat_status_dependencies() const noexcept { return std::array{ dependency }; }

        givm::definition_id<givm::combat_status_view> compile(givm::definition_compile_context& context) const
        {
            return context.resolve_id<givm::combat_status_view>(dependency);
        }
    };

    struct dynamic_card_source : plain_source<givm::card_definition>
    {
        static constexpr bool is_dynamic = true;

        template<class TView, class TEvent>
        constexpr bool can_handle() const noexcept { return false; }

        template<class TQuery>
        constexpr bool can_query() const noexcept { return false; }
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

    const plain_source<givm::card_definition> conflicting_card{ .source_name = "Card" };
    auto extension = givm_test::make_source_library();
    REQUIRE(extension.add(conflicting_card, summon));

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

    const auto prepared_ids = sources.make_issued_id_map(givm_test::basic_sources, selection);
    const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
    const auto [library, id_map] = compile(sources, givm_test::basic_sources, selection, program, program, givm::compile_mode::normal);
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

    const auto ids = library.make_issued_id_map(givm_test::basic_sources, selection);
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

        const auto prepared_ids = sources.make_issued_id_map(givm_test::basic_sources);
        const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
        const auto [library, id_map] = compile(sources, givm_test::basic_sources, program, program, givm::compile_mode::normal);
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

TEST_CASE("basic definitions are supplied at compile time and survive compiled library copies", "[source_library][reactions]")
{
    const plain_source<givm::combat_status_view> core{ .source_name = "Z custom core" };
    const plain_source<givm::combat_status_view> field{ .source_name = "A custom field" };
    const plain_source<givm::summon_view> flame{ .source_name = "Custom flame" };
    const plain_source<givm::attachment_view> frozen{ .source_name = "Custom frozen" };
    const givm::basic_definition_sources basics{ core, field, flame, frozen };
    const givm::definition_source_library sources;
    const auto [library, ids] = compile(sources, basics, givm::definition_selection{},
        std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    const auto check = [&](const givm::definition_library& compiled)
    {
        CHECK(compiled.dendro_core_id() == ids.get_id<givm::combat_status_view>(core.name()));
        CHECK(compiled.catalyzing_field_id() == ids.get_id<givm::combat_status_view>(field.name()));
        CHECK(compiled.burning_flame_id() == ids.get_id<givm::summon_view>(flame.name()));
        CHECK(compiled.frozen_id() == ids.get_id<givm::attachment_view>(frozen.name()));
        CHECK(bool(compiled.name(compiled.dendro_core_id()) == core.name()));
        CHECK(bool(compiled.name(compiled.catalyzing_field_id()) == field.name()));
        CHECK(bool(compiled.name(compiled.burning_flame_id()) == flame.name()));
        CHECK(bool(compiled.name(compiled.frozen_id()) == frozen.name()));
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
    CHECK_FALSE(sources.has<givm::combat_status_view>(core.name()));
    CHECK_FALSE(sources.has<givm::combat_status_view>(field.name()));
    CHECK_FALSE(sources.has<givm::summon_view>(flame.name()));
    CHECK_FALSE(sources.has<givm::attachment_view>(frozen.name()));
}

TEST_CASE("partial compilation includes basic dependencies before issuing ids", "[source_library][reactions]")
{
    const core_with_support_dependency core{ "Dependent core", "Reaction support" };
    const plain_source<givm::combat_status_view> field{ .source_name = "Field" };
    const plain_source<givm::summon_view> flame{ .source_name = "Flame" };
    const plain_source<givm::support_view> support{ .source_name = "Reaction support" };
    const plain_source<givm::card_definition> unused{ .source_name = "Unused card" };
    const givm::basic_definition_sources basics{ core, field, flame, givm_test::frozen };
    const givm::definition_source_library missing;
    CHECK_THROWS_AS(missing.make_issued_id_map(basics), std::invalid_argument);
    CHECK_THROWS_AS(compile(missing, basics, std::tuple{}, std::tuple{}, givm::compile_mode::normal), std::invalid_argument);
    const givm::definition_source_library sources{ support, unused };
    const auto prepared = sources.make_issued_id_map(basics, givm::definition_selection{});
    const auto [library, ids] = compile(sources, basics, givm::definition_selection{},
        std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    CHECK(ids.has<givm::combat_status_view>(core.name()));
    CHECK(ids.has<givm::combat_status_view>(field.name()));
    CHECK(ids.has<givm::summon_view>(flame.name()));
    CHECK(ids.has<givm::support_view>(support.name()));
    CHECK_FALSE(ids.has<givm::card_definition>(unused.name()));
    CHECK(prepared.get_id<givm::support_view>(support.name()) == ids.get_id<givm::support_view>(support.name()));
    CHECK_FALSE(prepared.has<givm::card_definition>(unused.name()));
    CHECK(library.dendro_core_id() == prepared.get_id<givm::combat_status_view>(core.name()));
    CHECK_FALSE(sources.has<givm::combat_status_view>(core.name()));
}

TEST_CASE("basic definitions can depend on each other in the same compile configuration", "[source_library][reactions]")
{
    const core_with_field_dependency core{ "Core", "Field" };
    const core_with_field_dependency field{ "Field", "Core" };
    const givm::basic_definition_sources basics{ core, field, givm_test::burning_flame, givm_test::frozen };
    const givm::definition_source_library sources;
    const auto prepared = sources.make_issued_id_map(basics, givm::definition_selection{});
    const auto [library, ids] = compile(sources, basics, givm::definition_selection{},
        std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    CHECK(library.dendro_core_id() == prepared.get_id<givm::combat_status_view>(core.name()));
    CHECK(library.catalyzing_field_id() == ids.get_id<givm::combat_status_view>(field.name()));
}

TEST_CASE("one source library can compile different basic versions without retaining bindings", "[source_library][reactions]")
{
    const plain_source<givm::combat_status_view> core{ .source_name = "Core" };
    const plain_source<givm::combat_status_view> old_field{ .source_name = "Field-1" };
    const plain_source<givm::combat_status_view> new_field{ .source_name = "Field-2" };
    const plain_source<givm::summon_view> flame{ .source_name = "Flame" };
    const plain_source<givm::attachment_view> old_frozen{ .source_name = "Frozen-1" };
    const plain_source<givm::attachment_view> new_frozen{ .source_name = "Frozen-2" };
    reaction_bindings observed;
    const card_with_reaction_bindings card{ &observed };
    const givm::definition_source_library original{ card };
    auto copied = original;
    auto sources = std::move(copied);
    givm::definition_source_library assigned;
    assigned = sources;
    sources = std::move(assigned);

    for(const bool use_new : { false, true, false })
    {
        CAPTURE(use_new);
        const givm::basic_definition_sources basics{
            core, use_new ? new_field : old_field, flame, use_new ? new_frozen : old_frozen };
        const auto prepared = sources.make_issued_id_map(basics);
        const auto [library, ids] = compile(sources, basics, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
        CHECK(observed.core == library.dendro_core_id());
        CHECK(observed.field == library.catalyzing_field_id());
        CHECK(observed.flame == library.burning_flame_id());
        CHECK(observed.frozen == library.frozen_id());
        CHECK(observed.field == prepared.get_id<givm::combat_status_view>(basics.catalyzing_field.name()));
        CHECK(observed.frozen == prepared.get_id<givm::attachment_view>(basics.frozen.name()));
        CHECK_FALSE(ids.has<givm::combat_status_view>((use_new ? old_field : new_field).name()));
        CHECK_FALSE(ids.has<givm::attachment_view>((use_new ? old_frozen : new_frozen).name()));
        CHECK_FALSE(sources.has<givm::combat_status_view>(old_field.name()));
        CHECK_FALSE(sources.has<givm::combat_status_view>(new_field.name()));
        CHECK_FALSE(sources.has<givm::attachment_view>(old_frozen.name()));
        CHECK_FALSE(sources.has<givm::attachment_view>(new_frozen.name()));
    }
}

TEST_CASE("source library merging reuses ordinary shared dependencies and permits self merge", "[source_library]")
{
    const plain_source<givm::support_view> support{ .source_name = "Shared support" };
    const card_with_support_dependency first{ "First card", support.name() };
    const card_with_support_dependency second{ "Second card", support.name() };
    givm::definition_source_library base{ first, support };
    const givm::definition_source_library extension{ second, support };
    REQUIRE(base.add(extension));
    REQUIRE(base.add(extension));
    REQUIRE(base.add(base));
    const auto [library, ids] = compile(base, givm_test::basic_sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    CHECK(ids.definition_count<givm::support_view>() == 1);
    CHECK(ids.definition_count<givm::card_definition>() == 2);
    CHECK(bool(library.name(ids.get_id<givm::card_definition>(first.name())) == first.name()));
    CHECK(bool(library.name(ids.get_id<givm::card_definition>(second.name())) == second.name()));
}

TEST_CASE("basic injection reuses identical sources and rejects different sources with the same name", "[source_library][reactions]")
{
    const plain_source<givm::combat_status_view> core{ .source_name = "Core" };
    const plain_source<givm::combat_status_view> field{ .source_name = "Field" };
    const plain_source<givm::combat_status_view> different_field{ .source_name = "Field" };
    const givm::definition_source_library sources{ field };
    const givm::basic_definition_sources matching{ core, field, givm_test::burning_flame, givm_test::frozen };
    const givm::basic_definition_sources conflicting{ core, different_field, givm_test::burning_flame, givm_test::frozen };
    const auto prepared = sources.make_issued_id_map(matching);
    const auto [library, ids] = compile(sources, matching, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    CHECK(ids.definition_count<givm::combat_status_view>() == 2);
    CHECK(library.catalyzing_field_id() == prepared.get_id<givm::combat_status_view>(field.name()));
    CHECK_THROWS_AS(sources.make_issued_id_map(conflicting), std::invalid_argument);
    CHECK_THROWS_AS(compile(sources, conflicting, std::tuple{}, std::tuple{}, givm::compile_mode::normal), std::invalid_argument);
    CHECK_FALSE(sources.has<givm::combat_status_view>(core.name()));
    CHECK(sources.has<givm::combat_status_view>(field.name()));
}

TEST_CASE("dynamic source instances can be shared but same-named distinct instances conflict", "[source_library]")
{
    const dynamic_card_source dynamic{ { .source_name = "Dynamic card" } };
    const dynamic_card_source other_dynamic{ { .source_name = "Dynamic card" } };
    const plain_source<givm::summon_view> summon{ .source_name = "Unmerged summon" };
    givm::definition_source_library sources{ dynamic };
    const givm::definition_source_library shared{ dynamic };
    const givm::definition_source_library conflicting{ other_dynamic, summon };
    REQUIRE(sources.add(shared));
    const auto [library, ids] = compile(sources, givm_test::basic_sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    CHECK(ids.definition_count<givm::card_definition>() == 1);
    CHECK(bool(library.name(ids.get_id<givm::card_definition>(dynamic.name())) == dynamic.name()));
    CHECK_FALSE(sources.add(conflicting));
    CHECK_FALSE(sources.has<givm::summon_view>(summon.name()));
    const givm::definition_source_library different_source_type{
        static_cast<const plain_source<givm::card_definition>&>(dynamic) };
    CHECK_FALSE(sources.add(different_source_type));
}
}
