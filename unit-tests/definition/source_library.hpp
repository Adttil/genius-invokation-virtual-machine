#include "../test_source_library.hpp"

#include <array>
#include <cstddef>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>
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

    struct card_with_soft_filter
    {
        using definition_category = givm::card_definition;

        struct definition_type
        {
            std::vector<givm::definition_id<givm::card_definition>> cards;
        };

        std::string_view source_name;
        std::string_view filter;
        std::vector<givm::definition_id<givm::card_definition>>* matches;

        constexpr std::string_view name() const noexcept
        {
            return source_name;
        }

        constexpr auto tags() const noexcept
        {
            return std::array<std::string_view, 2>{ "selected", "excluded" };
        }

        definition_type compile(givm::definition_compile_context& context) const
        {
            *matches = context.find_ids_by_tag<givm::card_definition>(filter);
            CHECK(context.find_definition<givm::card_definition>("Alpha").has_value() == not matches->empty());
            CHECK_FALSE(context.find_definition<givm::card_definition>("Gamma"));
            CHECK(context.definitions<givm::card_definition>().size() == (matches->empty() ? 1 : 3));
            return { .cards = *matches };
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

        template<class TView, class TEvent>
        static givm::program_entry handle(const definition_type&, const TView&, TEvent&, givm::handle_context&)
        {
            FAIL("A disabled dynamic handler was invoked");
            std::unreachable();
        }

        template<class TQuery>
        static TQuery::result_t query(const definition_type&, const TQuery&)
        {
            FAIL("A disabled dynamic query was invoked");
            std::unreachable();
        }
    };

    template<class TCategory>
    struct source_with_dependencies
    {
        using definition_category = TCategory;
        struct definition_type {};

        std::string_view source_name;
        std::span<const std::string_view> cards{};
        std::span<const std::string_view> supports{};
        std::span<const std::string_view> summons{};
        std::size_t* declaration_reads = nullptr;

        std::string_view name() const noexcept { return source_name; }
        auto card_dependencies() const noexcept { return cards; }
        auto support_dependencies() const noexcept
        {
            if(declaration_reads) ++*declaration_reads;
            return supports;
        }
        auto summon_dependencies() const noexcept { return summons; }
        definition_type compile(givm::definition_compile_context&) const noexcept { return {}; }
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

    const auto prepared_ids = givm_test::require_success(sources.make_issued_id_map(givm_test::basic_sources, selection));
    const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
    const auto [library, id_map] = givm_test::require_success(compile(sources, givm_test::basic_sources, selection, program, program, givm::compile_mode::normal));
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

TEST_CASE("soft tag filters query selected definitions without expanding the closure", "[source_library]")
{
    std::vector<givm::definition_id<givm::card_definition>> matches;
    const card_with_soft_filter selected_card{ "Root", "selected & !excluded", &matches };
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

    std::vector<std::string_view> card_roots{ "Root" };
    SECTION("unselected matching definitions remain unavailable") {}
    SECTION("explicitly selected definitions can be filtered")
    {
        card_roots.insert(card_roots.end(), { "Alpha", "Beta" });
    }
    givm::definition_selection selection{};
    selection[givm::definition_types::index_of<givm::card_definition>()] = card_roots;

    const auto ids = givm_test::require_success(library.make_issued_id_map(givm_test::basic_sources, selection));
    CHECK(ids.has<givm::card_definition>("Root"));
    CHECK(ids.has<givm::card_definition>("Alpha") == (card_roots.size() > 1));
    CHECK(ids.has<givm::card_definition>("Beta") == (card_roots.size() > 1));
    CHECK_FALSE(ids.has<givm::card_definition>("Gamma"));
    const auto [compiled, compiled_ids] = givm_test::require_success(compile(library, givm_test::basic_sources, selection,
        std::tuple{}, std::tuple{}, givm::compile_mode::normal));
    if(card_roots.size() == 1)
    {
        CHECK(matches.empty());
    }
    else
    {
        REQUIRE(matches.size() == 1);
        CHECK(matches.front() == compiled_ids.get_id<givm::card_definition>("Alpha"));
    }
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

        const auto prepared_ids = givm_test::require_success(sources.make_issued_id_map(givm_test::basic_sources));
        const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
        const auto [library, id_map] = givm_test::require_success(compile(sources, givm_test::basic_sources, program, program, givm::compile_mode::normal));
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
    const auto [library, ids] = givm_test::require_success(compile(sources, basics, givm::definition_selection{},
        std::tuple{}, std::tuple{}, givm::compile_mode::normal));
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
    const auto missing_ids = missing.make_issued_id_map(basics);
    REQUIRE_FALSE(missing_ids);
    REQUIRE(missing_ids.error().size() == 1);
    const auto* missing_dependency = std::get_if<givm::source_missing_dependency>(&missing_ids.error().front());
    REQUIRE(missing_dependency);
    CHECK(missing_dependency->source.name == std::string{ core.name() });
    CHECK(missing_dependency->dependency.name == std::string{ support.name() });
    const auto missing_library = compile(missing, basics, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    REQUIRE_FALSE(missing_library);
    REQUIRE(missing_library.error().size() == 2);
    CHECK(missing_library.error().front().location.stage == givm::compile_stage::source_selection);
    CHECK(std::holds_alternative<givm::source_missing_dependency>(missing_library.error().front().reason));
    const auto& resolution = missing_library.error()[1];
    CHECK(resolution.location.stage == givm::compile_stage::definition);
    REQUIRE(resolution.location.source);
    CHECK(resolution.location.source->name == std::string{ core.name() });
    const auto* unresolved = std::get_if<givm::definition_resolution_error>(&resolution.reason);
    REQUIRE(unresolved);
    CHECK(unresolved->cause == givm::definition_resolution_error::reason::not_found);
    CHECK(unresolved->definition.name == std::string{ support.name() });
    givm::definition_source_library sources;
    REQUIRE(sources.add(support, unused));
    const auto prepared = givm_test::require_success(sources.make_issued_id_map(basics, givm::definition_selection{}));
    const auto [library, ids] = givm_test::require_success(compile(sources, basics, givm::definition_selection{},
        std::tuple{}, std::tuple{}, givm::compile_mode::normal));
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
    const auto prepared = givm_test::require_success(sources.make_issued_id_map(basics, givm::definition_selection{}));
    const auto [library, ids] = givm_test::require_success(compile(sources, basics, givm::definition_selection{},
        std::tuple{}, std::tuple{}, givm::compile_mode::normal));
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
    givm::definition_source_library original;
    REQUIRE(original.add(card));
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
        const auto prepared = givm_test::require_success(sources.make_issued_id_map(basics));
        const auto [library, ids] = givm_test::require_success(compile(sources, basics, std::tuple{}, std::tuple{}, givm::compile_mode::normal));
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
    givm::definition_source_library base;
    REQUIRE(base.add(first, support));
    givm::definition_source_library extension;
    REQUIRE(extension.add(second, support));
    REQUIRE(base.add(extension));
    REQUIRE(base.add(extension));
    REQUIRE(base.add(base));
    const auto [library, ids] = givm_test::require_success(compile(base, givm_test::basic_sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal));
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
    givm::definition_source_library sources;
    REQUIRE(sources.add(field));
    const givm::basic_definition_sources matching{ core, field, givm_test::burning_flame, givm_test::frozen };
    const givm::basic_definition_sources conflicting{ core, different_field, givm_test::burning_flame, givm_test::frozen };
    const auto prepared = givm_test::require_success(sources.make_issued_id_map(matching));
    const auto [library, ids] = givm_test::require_success(compile(sources, matching, std::tuple{}, std::tuple{}, givm::compile_mode::normal));
    CHECK(ids.definition_count<givm::combat_status_view>() == 2);
    CHECK(library.catalyzing_field_id() == prepared.get_id<givm::combat_status_view>(field.name()));
    const auto conflicting_ids = sources.make_issued_id_map(conflicting);
    REQUIRE_FALSE(conflicting_ids);
    REQUIRE(conflicting_ids.error().size() == 1);
    const auto* conflict = std::get_if<givm::source_conflict>(&conflicting_ids.error().front());
    REQUIRE(conflict);
    CHECK(conflict->definition.name == std::string{ field.name() });
    CHECK(conflict->cause == givm::source_conflict::reason::different_object);
    const auto conflicting_library = compile(sources, conflicting, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    REQUIRE_FALSE(conflicting_library);
    REQUIRE(conflicting_library.error().size() == 1);
    CHECK(conflicting_library.error().front().location.stage == givm::compile_stage::source_selection);
    CHECK(std::holds_alternative<givm::source_conflict>(conflicting_library.error().front().reason));
    CHECK_FALSE(sources.has<givm::combat_status_view>(core.name()));
    CHECK(sources.has<givm::combat_status_view>(field.name()));
}

TEST_CASE("dynamic source instances can be shared but same-named distinct instances conflict", "[source_library]")
{
    const dynamic_card_source dynamic{ { .source_name = "Dynamic card" } };
    const dynamic_card_source other_dynamic{ { .source_name = "Dynamic card" } };
    const plain_source<givm::summon_view> summon{ .source_name = "Unmerged summon" };
    givm::definition_source_library sources;
    REQUIRE(sources.add(dynamic));
    REQUIRE(sources.add(dynamic, dynamic));
    givm::definition_source_library shared;
    REQUIRE(shared.add(dynamic));
    givm::definition_source_library conflicting;
    REQUIRE(conflicting.add(other_dynamic, summon));
    REQUIRE(sources.add(shared));
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal));
    CHECK(ids.definition_count<givm::card_definition>() == 1);
    CHECK(bool(library.name(ids.get_id<givm::card_definition>(dynamic.name())) == dynamic.name()));
    const auto object_result = sources.add(conflicting);
    REQUIRE_FALSE(object_result);
    REQUIRE(object_result.error().size() == 1);
    CHECK(object_result.error()[0].cause == givm::source_conflict::reason::different_object);
    CHECK(object_result.error()[0].definition.category_index == givm::definition_types::index_of<givm::card_definition>());
    CHECK(object_result.error()[0].definition.name == std::string{ dynamic.name() });
    CHECK_FALSE(object_result.error()[0].first_input_index);
    CHECK_FALSE(object_result.error()[0].second_input_index);
    CHECK_FALSE(sources.has<givm::summon_view>(summon.name()));
    givm::definition_source_library different_source_type;
    REQUIRE(different_source_type.add(static_cast<const plain_source<givm::card_definition>&>(dynamic)));
    const auto type_result = sources.add(different_source_type);
    REQUIRE_FALSE(type_result);
    REQUIRE(type_result.error().size() == 1);
    CHECK(type_result.error()[0].cause == givm::source_conflict::reason::different_type);
    CHECK_FALSE(type_result.error()[0].first_input_index);
    CHECK_FALSE(type_result.error()[0].second_input_index);
}

TEST_CASE("source addition reports all conflicts then unique missing dependencies without mutation", "[source_library]")
{
    constexpr auto card_category = givm::definition_types::index_of<givm::card_definition>();
    constexpr auto status_category = givm::definition_types::index_of<givm::combat_status_view>();
    constexpr auto support_category = givm::definition_types::index_of<givm::support_view>();
    constexpr auto summon_category = givm::definition_types::index_of<givm::summon_view>();
    const plain_source<givm::card_definition> existing{ .source_name = "Existing card" };
    const plain_source<givm::card_definition> conflicting_existing{ .source_name = "Existing card" };
    const std::array missing_cards{ std::string_view{ "Missing card A" }, std::string_view{ "Missing card A" },
        std::string_view{ "Missing card B" } };
    const std::array missing_supports{ std::string_view{ "Missing support" } };
    const std::array missing_summons{ std::string_view{ "Missing summon" } };
    const std::array conflicting_missing{ std::string_view{ "Conflict missing" } };
    std::size_t reads = 0;
    const source_with_dependencies<givm::combat_status_view> first{
        .source_name = "First source", .cards = missing_cards, .supports = missing_supports, .declaration_reads = &reads };
    const source_with_dependencies<givm::card_definition> second{
        .source_name = "Second source", .summons = missing_summons };
    const plain_source<givm::support_view> batch_support{ .source_name = "Batch support" };
    const source_with_dependencies<givm::support_view> conflicting_support{
        .source_name = "Batch support", .summons = conflicting_missing };
    const std::array named_card{ existing.name() };
    const std::array named_support{ batch_support.name() };
    const source_with_dependencies<givm::summon_view> names_exist{
        .source_name = "Names exist", .cards = named_card, .supports = named_support };
    givm::definition_source_library library;
    REQUIRE(library.add(existing));
    const auto result = library.add(first, conflicting_existing, second, batch_support, conflicting_support, first, names_exist);
    REQUIRE_FALSE(result);
    const auto& errors = result.error();
    REQUIRE(errors.size() == 7);
    const auto* existing_conflict = std::get_if<givm::source_conflict>(&errors[0]);
    REQUIRE(existing_conflict);
    CHECK(existing_conflict->definition.category_index == card_category);
    CHECK(existing_conflict->definition.name == std::string{ existing.name() });
    CHECK(existing_conflict->cause == givm::source_conflict::reason::different_object);
    CHECK_FALSE(existing_conflict->first_input_index);
    CHECK(existing_conflict->second_input_index == 1);
    const auto* batch_conflict = std::get_if<givm::source_conflict>(&errors[1]);
    REQUIRE(batch_conflict);
    CHECK(batch_conflict->definition.category_index == support_category);
    CHECK(batch_conflict->definition.name == std::string{ batch_support.name() });
    CHECK(batch_conflict->cause == givm::source_conflict::reason::different_type);
    CHECK(batch_conflict->first_input_index == 3);
    CHECK(batch_conflict->second_input_index == 4);
    const std::array expected_sources{ status_category, status_category, status_category, card_category, support_category };
    const std::array expected_inputs{ std::size_t{ 0 }, std::size_t{ 0 }, std::size_t{ 0 }, std::size_t{ 2 }, std::size_t{ 4 } };
    const std::array expected_categories{ card_category, card_category, support_category, summon_category, summon_category };
    const std::array expected_names{ "Missing card A", "Missing card B", "Missing support", "Missing summon", "Conflict missing" };
    const std::array expected_source_names{ "First source", "First source", "First source", "Second source", "Batch support" };
    for(std::size_t index = 0; index != expected_names.size(); ++index)
    {
        const auto* missing = std::get_if<givm::source_missing_dependency>(&errors[index + 2]);
        REQUIRE(missing);
        CHECK(missing->source.category_index == expected_sources[index]);
        CHECK(missing->source.name == expected_source_names[index]);
        CHECK(missing->input_index == expected_inputs[index]);
        CHECK(missing->dependency.category_index == expected_categories[index]);
        CHECK(missing->dependency.name == expected_names[index]);
    }
    CHECK(reads == 1);
    CHECK(library.has<givm::card_definition>(existing.name()));
    CHECK_FALSE(library.has<givm::card_definition>(second.name()));
    CHECK_FALSE(library.has<givm::combat_status_view>(first.name()));
    CHECK_FALSE(library.has<givm::support_view>(batch_support.name()));
    CHECK_FALSE(library.has<givm::summon_view>(names_exist.name()));
}

TEST_CASE("identical input sources are reused without rereading their declarations", "[source_library]")
{
    const plain_source<givm::support_view> support{ .source_name = "Support" };
    const std::array dependency{ support.name() };
    std::size_t reads = 0;
    const source_with_dependencies<givm::card_definition> source{
        .source_name = "Card", .supports = dependency, .declaration_reads = &reads };
    givm::definition_source_library library;
    REQUIRE(library.add());
    REQUIRE(library.add(source, support, source));
    CHECK(reads == 1);
    REQUIRE(library.add(source));
    REQUIRE(library.add(source, source, support));
    CHECK(reads == 1);
    CHECK(std::ranges::distance(library.source_views<givm::card_definition>()) == 1);
    CHECK(std::ranges::distance(library.source_views<givm::support_view>()) == 1);
}

TEST_CASE("named dependencies allow self references and cycles in one batch", "[source_library]")
{
    const std::array self_name{ std::string_view{ "Self" } };
    const source_with_dependencies<givm::card_definition> self{ .source_name = "Self", .cards = self_name };
    const std::array first_dependencies{ std::string_view{ "Second" } };
    const std::array second_dependencies{ std::string_view{ "First" } };
    const source_with_dependencies<givm::card_definition> first{ .source_name = "First", .cards = first_dependencies };
    const source_with_dependencies<givm::card_definition> second{ .source_name = "Second", .cards = second_dependencies };
    givm::definition_source_library library;
    REQUIRE(library.add(self));
    REQUIRE(library.add(first, second));
    CHECK(std::ranges::distance(library.source_views<givm::card_definition>()) == 3);
}

TEST_CASE("merge returns every conflict in category and registration order without dependency checks", "[source_library]")
{
    const plain_source<givm::card_definition> first{ .source_name = "First" };
    const plain_source<givm::card_definition> second{ .source_name = "Second" };
    const plain_source<givm::support_view> support{ .source_name = "Support" };
    const plain_source<givm::card_definition> other_first{ .source_name = "First" };
    const plain_source<givm::card_definition> other_second{ .source_name = "Second" };
    const plain_source<givm::support_view> other_support{ .source_name = "Support" };
    const std::array dependency{ support.name() };
    std::size_t reads = 0;
    const source_with_dependencies<givm::summon_view> fresh{
        .source_name = "Fresh", .supports = dependency, .declaration_reads = &reads };
    givm::definition_source_library library;
    givm::definition_source_library extension;
    REQUIRE(library.add(first, second, support));
    REQUIRE(extension.add(other_support, other_second, fresh, other_first));
    CHECK(reads == 1);
    const auto result = library.add(extension);
    REQUIRE_FALSE(result);
    const auto& errors = result.error();
    REQUIRE(errors.size() == 3);
    const std::array expected_names{ "Second", "First", "Support" };
    const std::array expected_categories{
        givm::definition_types::index_of<givm::card_definition>(),
        givm::definition_types::index_of<givm::card_definition>(),
        givm::definition_types::index_of<givm::support_view>() };
    for(std::size_t index = 0; index != errors.size(); ++index)
    {
        CHECK(errors[index].definition.name == expected_names[index]);
        CHECK(errors[index].definition.category_index == expected_categories[index]);
        CHECK(errors[index].cause == givm::source_conflict::reason::different_object);
        CHECK_FALSE(errors[index].first_input_index);
        CHECK_FALSE(errors[index].second_input_index);
    }
    CHECK(reads == 1);
    CHECK_FALSE(library.has<givm::summon_view>(fresh.name()));
}

TEST_CASE("source addition errors own names from rejected sources and their dependencies", "[source_library]")
{
    givm::definition_source_library library;
    const auto result = [&]
    {
        std::string first_name = "Duplicated source";
        std::string second_name = first_name;
        std::string missing_name = "Missing dependency";
        const std::array dependency{ std::string_view{ missing_name } };
        const source_with_dependencies<givm::card_definition> first{
            .source_name = first_name, .supports = dependency };
        const plain_source<givm::card_definition> second{ .source_name = second_name };
        auto failure = library.add(first, second);
        first_name.assign(first_name.size(), 'x');
        second_name.assign(second_name.size(), 'y');
        missing_name.assign(missing_name.size(), 'z');
        return failure;
    }();
    REQUIRE_FALSE(result);
    REQUIRE(result.error().size() == 2);
    const auto* conflict = std::get_if<givm::source_conflict>(&result.error()[0]);
    const auto* missing = std::get_if<givm::source_missing_dependency>(&result.error()[1]);
    REQUIRE(conflict);
    REQUIRE(missing);
    CHECK(conflict->definition.name == "Duplicated source");
    CHECK(missing->source.name == "Duplicated source");
    CHECK(missing->dependency.name == "Missing dependency");
    CHECK_FALSE(library.has<givm::card_definition>("Duplicated source"));
}

TEST_CASE("source library factory accepts empty input and builds deduplicated dependency batches", "[source_library]")
{
    const auto empty = givm::make_definition_source_library();
    REQUIRE(empty);
    CHECK(std::ranges::distance(empty->source_views<givm::card_definition>()) == 0);
    CHECK(std::ranges::distance(empty->source_views<givm::support_view>()) == 0);

    const card_with_support_dependency card{ "Factory card", "Factory support" };
    const plain_source<givm::support_view> support{ .source_name = "Factory support" };
    const auto result = givm::make_definition_source_library(card, support, card, support);
    REQUIRE(result);
    CHECK(result->has<givm::card_definition>(card.name()));
    CHECK(result->has<givm::support_view>(support.name()));
    CHECK(std::ranges::distance(result->source_views<givm::card_definition>()) == 1);
    CHECK(std::ranges::distance(result->source_views<givm::support_view>()) == 1);
}

TEST_CASE("source library factory returns all batch errors without a partial library", "[source_library]")
{
    const card_with_support_dependency card{ "Factory conflict", "Missing support" };
    const plain_source<givm::card_definition> conflicting_card{ .source_name = "Factory conflict" };
    const std::array dependencies{ std::string_view{ "Missing card" } };
    const source_with_dependencies<givm::summon_view> summon{
        .source_name = "Factory summon", .cards = dependencies };
    const plain_source<givm::support_view> accepted_support{ .source_name = "Accepted support" };
    const auto result = givm::make_definition_source_library(card, conflicting_card, summon, accepted_support);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().size() == 3);
    const auto* conflict = std::get_if<givm::source_conflict>(&result.error()[0]);
    const auto* first_missing = std::get_if<givm::source_missing_dependency>(&result.error()[1]);
    const auto* second_missing = std::get_if<givm::source_missing_dependency>(&result.error()[2]);
    REQUIRE(conflict);
    REQUIRE(first_missing);
    REQUIRE(second_missing);
    CHECK(conflict->definition.name == "Factory conflict");
    CHECK(conflict->cause == givm::source_conflict::reason::different_type);
    CHECK(conflict->first_input_index == 0);
    CHECK(conflict->second_input_index == 1);
    CHECK(first_missing->source.name == "Factory conflict");
    CHECK(first_missing->input_index == 0);
    CHECK(first_missing->dependency.name == "Missing support");
    CHECK(second_missing->source.name == "Factory summon");
    CHECK(second_missing->input_index == 2);
    CHECK(second_missing->dependency.name == "Missing card");
}

TEST_CASE("empty source error collections format as empty text", "[source_library]")
{
    CHECK(givm::error_string(std::vector<givm::source_add_error>{}).empty());
    CHECK(givm::error_string(std::vector<givm::source_conflict>{}).empty());
}

TEST_CASE("source addition errors format reasons categories names and input positions in order", "[source_library]")
{
    const std::vector<givm::source_add_error> errors{
        givm::source_conflict{
            .definition = { givm::definition_types::index_of<givm::card_definition>(), "Repeated card" },
            .cause = givm::source_conflict::reason::different_object,
            .second_input_index = 0 },
        givm::source_conflict{
            .definition = { givm::definition_types::index_of<givm::support_view>(), "Repeated support" },
            .cause = givm::source_conflict::reason::different_type,
            .first_input_index = 1,
            .second_input_index = 2 },
        givm::source_missing_dependency{
            .source = { givm::definition_types::index_of<givm::combat_status_view>(), "Dependent status" },
            .input_index = 3,
            .dependency = { givm::definition_types::index_of<givm::skill_view>(), "Needed skill" } }
    };
    const auto message = givm::error_string(errors);
    REQUIRE_FALSE(message.empty());
    const auto first = message.find(R"(source conflict (different_object): card_definition "Repeated card")");
    const auto second = message.find(R"(source conflict (different_type): support_view "Repeated support")");
    const auto third = message.find(R"(missing dependency: combat_status_view "Dependent status")");
    REQUIRE(first != std::string::npos);
    REQUIRE(second != std::string::npos);
    REQUIRE(third != std::string::npos);
    CHECK(first < second);
    CHECK(second < third);
    CHECK(message.find("first: receiver library; second: input[0]") != std::string::npos);
    CHECK(message.find("first: input[1]; second: input[2]") != std::string::npos);
    CHECK(message.find(R"((input[3]) requires skill_view "Needed skill")") != std::string::npos);
    CHECK(std::ranges::count(message, '\n') == 2);
    CHECK(message.back() != '\n');

    const auto single = givm::error_string(std::vector<givm::source_add_error>{ errors[2] });
    CHECK(single.find("Dependent status") != std::string::npos);
    CHECK(single.find("Needed skill") != std::string::npos);
    CHECK(single.find('\n') == std::string::npos);
}

TEST_CASE("source merge errors format receiving and incoming library positions", "[source_library]")
{
    const std::vector<givm::source_conflict> errors{
        {
            .definition = { givm::definition_types::index_of<givm::summon_view>(), "First summon" },
            .cause = givm::source_conflict::reason::different_object },
        {
            .definition = { givm::definition_types::index_of<givm::attachment_view>(), "Second attachment" },
            .cause = givm::source_conflict::reason::different_type }
    };
    const auto single = givm::error_string(std::vector<givm::source_conflict>{ errors[0] });
    CHECK(single.find(R"(summon_view "First summon")") != std::string::npos);
    CHECK(single.find("different_object") != std::string::npos);
    CHECK(single.find("first: receiver library; second: incoming library") != std::string::npos);
    CHECK(single.find("input[") == std::string::npos);
    CHECK(single.find('\n') == std::string::npos);
    const auto message = givm::error_string(errors);
    REQUIRE_FALSE(message.empty());
    const auto first = message.find(R"(summon_view "First summon")");
    const auto second = message.find(R"(attachment_view "Second attachment")");
    REQUIRE(first != std::string::npos);
    REQUIRE(second != std::string::npos);
    CHECK(first < second);
    CHECK(message.find("different_type") != std::string::npos);
    CHECK(std::ranges::count(message, '\n') == 1);
    CHECK(message.back() != '\n');
}

TEST_CASE("source error formatting keeps escaped definition names on one line", "[source_library]")
{
    const std::string name = "First\"\\\n\r\tLast";
    const std::vector<givm::source_conflict> errors{
        {
            .definition = { givm::definition_types::index_of<givm::card_definition>(), name },
            .cause = givm::source_conflict::reason::different_object }
    };
    const auto message = givm::error_string(errors);
    CHECK(message.find(R"(First\"\\\n\r\tLast)") != std::string::npos);
    CHECK(message.find(char{ 10 }) == std::string::npos);
    CHECK(message.find(char{ 13 }) == std::string::npos);
    CHECK(message.find(char{ 9 }) == std::string::npos);
}

}
