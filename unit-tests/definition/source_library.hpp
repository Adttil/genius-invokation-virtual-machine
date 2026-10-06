#include <cstdint>
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
        using definition_category = givm::reaction_view;

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
        std::array<givm::definition_id<givm::reaction_view>, givm::elemental_reaction_count> ids;
    };

    struct card_with_reaction_bindings
    {
        using definition_category = givm::card_definition;
        reaction_bindings* bindings;
        constexpr std::string_view name() const noexcept { return "Reaction-aware card"; }
        reaction_bindings compile(givm::definition_compile_context& context) const
        {
            for(std::size_t i = 0; i != bindings->ids.size(); ++i)
                bindings->ids[i] = context.default_reaction_id(static_cast<givm::elemental_reaction>(i + 1));
            return *bindings;
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
        static givm::program_entry handle(const definition_type&, TEvent&, givm::handle_context<TView>&, std::uint32_t = 0)
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

    givm::definition_source_library library;
    CHECK_FALSE(library.add(card));
    CHECK(library.empty());
    CHECK_FALSE(library.has<givm::card_definition>("Card"));

    REQUIRE(library.add(card, support));
    CHECK_FALSE(library.empty());
    CHECK(library.has<givm::card_definition>("Card"));
    CHECK(library.has<givm::support_view>("Support"));

    givm::definition_source_library duplicate_batch;
    const plain_source<givm::card_definition> duplicate{ .source_name = "Card" };
    CHECK_FALSE(duplicate_batch.add(card, duplicate, support));
    CHECK(duplicate_batch.empty());
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

    const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
    const auto [library, id_map] = givm_test::require_success(compile(sources, givm_test::basic_sources, selection, program, program, givm::compile_mode::normal));
    CHECK(id_map.has<givm::card_definition>("Root"));
    CHECK(id_map.has<givm::support_view>("Support"));
    CHECK_FALSE(id_map.has<givm::card_definition>("Unused"));

    const auto root_id = id_map.get_id<givm::card_definition>("Root");
    const auto support_id = id_map.get_id<givm::support_view>("Support");
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

    const auto [compiled, compiled_ids] = givm_test::require_success(compile(library, givm_test::basic_sources, selection,
        std::tuple{}, std::tuple{}, givm::compile_mode::normal));
    CHECK(compiled_ids.has<givm::card_definition>("Root"));
    CHECK(compiled_ids.has<givm::card_definition>("Alpha") == (card_roots.size() > 1));
    CHECK(compiled_ids.has<givm::card_definition>("Beta") == (card_roots.size() > 1));
    CHECK_FALSE(compiled_ids.has<givm::card_definition>("Gamma"));
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

        const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
        const auto [library, id_map] = givm_test::require_success(compile(sources, givm_test::basic_sources, program, program, givm::compile_mode::normal));
        const auto alpha_id = id_map.get_id<givm::card_definition>("Alpha");
        const auto zulu_id = id_map.get_id<givm::card_definition>("Zulu");
        const auto alpha_tag = id_map.get_tag_id("alpha");
        const auto zeta_tag = id_map.get_tag_id("zeta");

        CHECK(bool(library.name(alpha_id) == "Alpha"));
        CHECK(bool(library.name(zulu_id) == "Zulu"));
        CHECK(bool(library.tag_name(alpha_tag) == "alpha"));
        CHECK(bool(library.tag_name(zeta_tag) == "zeta"));
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
    REQUIRE(base.add(givm_test::make_source_library()));
    const auto [library, ids] = givm_test::require_success(compile(base, givm_test::basic_sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal));
    CHECK(ids.definition_count<givm::support_view>() == 1);
    CHECK(ids.definition_count<givm::card_definition>() == 2);
    CHECK(bool(library.name(ids.get_id<givm::card_definition>(first.name())) == first.name()));
    CHECK(bool(library.name(ids.get_id<givm::card_definition>(second.name())) == second.name()));
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
    REQUIRE(sources.add(givm_test::make_source_library()));
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

TEST_CASE("default reaction names resolve existing sources and survive compiled copies", "[source_library][reactions]")
{
    const plain_source<givm::reaction_view> ordinary{ .source_name = "Ordinary reaction" };
    const plain_source<givm::reaction_view> alternative{ .source_name = "Alternative reaction" };
    reaction_bindings observed;
    const card_with_reaction_bindings card{ &observed };
    givm::definition_source_library sources;
    REQUIRE(sources.add(ordinary, alternative, card));
    givm::reaction_definition_names names{ ordinary.name() };
    names[givm::elemental_reaction::bloom] = alternative.name();
    const auto [library, ids] = givm_test::require_success(compile(sources, names,
        std::tuple{}, std::tuple{}, givm::compile_mode::normal));
    for(std::size_t i = 0; i != givm::elemental_reaction_count; ++i)
    {
        const auto slot = static_cast<givm::elemental_reaction>(i + 1);
        CHECK(library.default_reaction_id(slot) == ids.get_id<givm::reaction_view>(names[slot]));
        CHECK(observed.ids[i] == library.default_reaction_id(slot));
    }
    auto copied = library;
    auto moved = std::move(copied);
    CHECK(moved.default_reaction_id(givm::elemental_reaction::bloom)
        == ids.get_id<givm::reaction_view>(alternative.name()));
    CHECK(std::ranges::distance(sources.source_views<givm::reaction_view>()) == 2);
}

TEST_CASE("reaction selection requires registered names and dependency closure", "[source_library][reactions]")
{
    const core_with_support_dependency reaction{ "Dependent reaction", "Reaction support" };
    const plain_source<givm::support_view> support{ .source_name = "Reaction support" };
    const plain_source<givm::card_definition> unused{ .source_name = "Unused card" };
    givm::reaction_definition_names names{ reaction.name() };
    givm::definition_source_library missing;
    REQUIRE_FALSE(compile(missing, names, std::tuple{}, std::tuple{}, givm::compile_mode::normal));
    CHECK(missing.empty());
    REQUIRE_FALSE(missing.add(reaction));
    const auto missing_closure = compile(missing, names, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    REQUIRE_FALSE(missing_closure);
    CHECK(missing_closure.error().front().location.stage == givm::compile_stage::source_selection);
    REQUIRE(missing.add(reaction, support, unused));
    const auto [library, ids] = givm_test::require_success(compile(missing, names, givm::definition_selection{},
        std::tuple{}, std::tuple{}, givm::compile_mode::normal));
    CHECK(ids.has<givm::reaction_view>(reaction.name()));
    CHECK(ids.has<givm::support_view>(support.name()));
    CHECK_FALSE(ids.has<givm::card_definition>(unused.name()));
}

TEST_CASE("one source collection selects different registered reaction versions", "[source_library][reactions]")
{
    const plain_source<givm::reaction_view> first{ .source_name = "Reaction 1" };
    const plain_source<givm::reaction_view> second{ .source_name = "Reaction 2" };
    givm::definition_source_library sources;
    REQUIRE(sources.add(first, second));
    for(const bool use_second : { false, true, false })
    {
        givm::reaction_definition_names names{ use_second ? second.name() : first.name() };
        const auto [library, ids] = givm_test::require_success(compile(sources, names, givm::definition_selection{},
            std::tuple{}, std::tuple{}, givm::compile_mode::normal));
        CHECK(bool(library.name(library.default_reaction_id(givm::elemental_reaction::melt)) == names[givm::elemental_reaction::melt]));
        CHECK_FALSE(ids.has<givm::reaction_view>(use_second ? first.name() : second.name()));
    }
    CHECK(std::ranges::distance(sources.source_views<givm::reaction_view>()) == 2);
}

}
