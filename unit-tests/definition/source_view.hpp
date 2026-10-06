#include "../test_source_library.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/definition.hpp>
#include <givm/executor.hpp>
#include <givm/table.hpp>

namespace givm_test::definition::source_view
{
constexpr std::array<std::size_t, 1> draw_positions_1{ 0 };

namespace
{
    struct dependency_observation
    {
        bool handled = false;
        givm::definition_id<givm::support_view> alpha_support;
        givm::definition_id<givm::support_view> beta_support;
        givm::tag_id chosen_tag;
        std::vector<givm::definition_id<givm::support_view>> filtered_supports;
    };

    struct tagged_support_source
    {
        using definition_category = givm::support_view;

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

    struct dependent_card_source
    {
        using definition_category = givm::card_definition;

        struct definition_type
        {
            dependency_observation* observation;
            givm::definition_id<givm::support_view> alpha_support;
            givm::definition_id<givm::support_view> beta_support;
            givm::tag_id chosen_tag;
            std::vector<givm::definition_id<givm::support_view>> filtered_supports;
        };

        dependency_observation* observation;

        constexpr std::string_view name() const noexcept
        {
            return "DependentCard";
        }

        constexpr auto support_dependencies() const noexcept
        {
            return std::array<std::string_view, 2>{ "AlphaSupport", "BetaSupport" };
        }

        constexpr auto tags() const noexcept
        {
            return std::array<std::string_view, 1>{ "chosen" };
        }

        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto self = context.find_definition<givm::card_definition>(name());
            REQUIRE(self);
            CHECK(self->can_handle<givm::round_started, givm::hand_card_view>());
            CHECK_FALSE(self->can_handle<givm::round_started, givm::deck_card_view>());
            CHECK_FALSE(self->has_query<givm::card_initial_state>());
            CHECK(std::ranges::equal(self->dependencies<givm::support_view>(),
                std::array<std::string_view, 2>{ "AlphaSupport", "BetaSupport" }));
            CHECK(self->dependencies<givm::card_definition>().empty());
            return {
                .observation = observation,
                .alpha_support = context.resolve_id<givm::support_view>("AlphaSupport"),
                .beta_support = context.resolve_id<givm::support_view>("BetaSupport"),
                .chosen_tag = context.find_tag("chosen").value(),
                .filtered_supports =
                    context.find_ids_by_tag<givm::support_view>("selected & !excluded")
            };
        }

        static givm::program_entry handle(
            const definition_type& definition,
            givm::round_started&,
            givm::handle_context<givm::hand_card_view>&, std::uint32_t = 0)
        {
            definition.observation->handled = true;
            definition.observation->alpha_support = definition.alpha_support;
            definition.observation->beta_support = definition.beta_support;
            definition.observation->chosen_tag = definition.chosen_tag;
            definition.observation->filtered_supports = definition.filtered_supports;
            return {};
        }
    };

    struct program_observation
    {
        bool event_compiled = false;
        bool onpay_compiled = false;
        givm::definition_id<givm::support_view> resolved_support;
        bool first_event_entry_set = false;
        bool second_event_entry_set = false;
        bool onpay_entry_set = false;
    };

    struct programmed_card_source
    {
        using definition_category = givm::card_definition;

        struct definition_type
        {
            program_observation* observation;
            givm::definition_id<givm::support_view> support;
            givm::program_entry first_entry;
            givm::program_entry second_entry;
        };

        program_observation* observation;

        constexpr std::string_view name() const noexcept
        {
            return "ProgrammedCard";
        }

        constexpr auto support_dependencies() const noexcept
        {
            return std::array<std::string_view, 1>{ "ProgrammedSupport" };
        }

        definition_type compile(givm::definition_compile_context& context) const
        {
            observation->event_compiled = true;
            const auto support = context.resolve_id<givm::support_view>("ProgrammedSupport");
            observation->resolved_support = support;

            const auto first_entry = context.add_program(std::tuple{
                givm::shuffle_deck{ .player = givm::player_id{ 0 } },
                givm::shuffle_deck{ .player = givm::player_id{ 0 } }
            });
            const auto second_entry = context.add_program(
                std::vector{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } }
            );
            observation->first_event_entry_set = bool{ first_entry };
            observation->second_event_entry_set = bool{ second_entry };
            return {
                .observation = observation,
                .support = support,
                .first_entry = first_entry,
                .second_entry = second_entry
            };
        }
    };

    struct programmed_support_source
    {
        using definition_category = givm::support_view;

        struct definition_type
        {
            program_observation* observation;
            givm::program_entry onpay_entry;
        };

        program_observation* observation;

        constexpr std::string_view name() const noexcept
        {
            return "ProgrammedSupport";
        }

        definition_type compile(givm::definition_compile_context& context) const
        {
            observation->onpay_compiled = true;
            using instruction_type = givm::any_command;
            const auto entry = context.add_program(std::vector{
                instruction_type{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } },
                instruction_type{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } }
            });
            observation->onpay_entry_set = bool{ entry };
            return { .observation = observation, .onpay_entry = entry };
        }
    };

    struct undeclared_dependency_source
    {
        using definition_category = givm::card_definition;

        struct definition_type{};

        constexpr std::string_view name() const noexcept
        {
            return "UndeclaredDependency";
        }

        definition_type compile(givm::definition_compile_context& context) const noexcept
        {
            (void)context.resolve_id<givm::support_view>("MissingDeclaration");
            return {};
        }
    };

    struct selectable_handler_source
    {
        using definition_category = givm::support_view;
        static constexpr bool is_dynamic = true;

        struct definition_type{};

        std::string_view source_name;
        bool enabled;
        std::size_t* capability_checks = nullptr;

        constexpr std::string_view name() const noexcept
        {
            return source_name;
        }

        constexpr definition_type compile(givm::definition_compile_context&) const noexcept
        {
            return {};
        }

        template<class TView, class TEvent>
        constexpr bool can_handle() const noexcept
        {
            if(capability_checks) ++*capability_checks;
            return enabled && std::is_same_v<TView, givm::support_view>
                && std::is_same_v<TEvent, givm::round_started>;
        }

        template<class TQuery>
        constexpr bool can_query() const noexcept
        {
            if(capability_checks) ++*capability_checks;
            return false;
        }

        static givm::program_entry handle(
            const definition_type&,
            givm::round_started&,
            givm::handle_context<givm::support_view>&, std::uint32_t = 0)
        {
            return {};
        }

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

    struct static_handler_source
    {
        using definition_category = givm::support_view;

        struct definition_type{};

        std::string_view source_name;
        std::uint32_t* capability_checks;

        std::string_view name() const noexcept { return source_name; }

        definition_type compile(givm::definition_compile_context&) const noexcept { return {}; }

        template<class TView, class TEvent>
        bool can_handle() const noexcept
        {
            ++*capability_checks;
            return false;
        }

        static givm::program_entry handle(const definition_type&,
            givm::round_started&, givm::handle_context<givm::support_view>&, std::uint32_t = 0)
        {
            return {};
        }
    };

    struct explicitly_static_handler_source : static_handler_source
    {
        static constexpr bool is_dynamic = false;
    };

    struct metadata_observation
    {
        std::array<std::size_t, 3> declaration_reads{};
        std::size_t layout_reads = 0;
        std::size_t compile_reads = 0;

        void inspect(const givm::definition_compile_context& context) const
        {
            const auto definitions = context.definitions<givm::support_view>();
            STATIC_REQUIRE(std::ranges::sized_range<decltype(definitions)>);
            STATIC_REQUIRE(std::same_as<std::ranges::range_value_t<decltype(definitions)>,
                givm::definition_compile_context::definition_view<givm::support_view>>);
            REQUIRE(definitions.size() == 4);
            for(const auto definition : definitions)
            {
                CHECK(bool(context[definition.id()].name() == definition.name()));
                const auto found = context.find_definition<givm::support_view>(definition.name());
                REQUIRE(found);
                CHECK(found->id() == definition.id());
                CHECK_FALSE(definition.can_handle<givm::damage_effect>());
                CHECK_FALSE(definition.can_handle<givm::round_started, givm::hand_card_view>());
                CHECK_FALSE(definition.has_query<givm::card_initial_state>());
            }

            std::vector<std::string_view> active;
            for(const auto definition : definitions | std::views::filter([](const auto& definition)
            {
                return definition.has_tag("eligible")
                    && definition.template can_handle<givm::round_started>()
                    && definition.template has_query<givm::support_state_limit>();
            }))
            {
                active.push_back(definition.name());
                CHECK(std::ranges::equal(definition.tags(),
                    std::array<std::string_view, 2>{ "metadata", "eligible" }));
            }
            std::ranges::sort(active);
            CHECK(bool(active == std::vector<std::string_view>{ "DynamicEnabled", "StaticCustom" }));
            CHECK_FALSE(context.find_definition<givm::support_view>("Unknown"));
            CHECK_FALSE(context.find_definition<givm::card_definition>("StaticCustom"));
            CHECK(context.find_tag("metadata"));
            CHECK_FALSE(context.find_tag("Unknown"));

            const auto defaulted = context.find_definition<givm::support_view>("StaticDefault");
            REQUIRE(defaulted);
            CHECK_FALSE(defaulted->can_handle<givm::round_started>());
            CHECK_FALSE(defaulted->has_query<givm::support_state_limit>());
            CHECK(defaulted->tags().empty());
            CHECK_FALSE(defaulted->has_tag("metadata"));
            const auto disabled = context.find_definition<givm::support_view>("DynamicDisabled");
            REQUIRE(disabled);
            CHECK_FALSE(disabled->can_handle<givm::round_started>());
            CHECK_FALSE(disabled->has_query<givm::support_state_limit>());

            const auto summary = context.find_definition<givm::history_summary_definition>("MetadataObserver");
            REQUIRE(summary);
            CHECK(summary->can_handle<givm::history_summary_initialization>());
            CHECK_FALSE(summary->can_handle<givm::round_started>());
            CHECK_FALSE(summary->has_query<givm::support_state_limit>());
        }
    };

    struct metadata_support_source
    {
        using definition_category = givm::support_view;
        struct definition_type {};
        std::string_view source_name;
        metadata_observation* observation;

        std::string_view name() const noexcept
        {
            ++observation->declaration_reads[0];
            return source_name;
        }
        auto tags() const noexcept
        {
            ++observation->declaration_reads[1];
            return std::array<std::string_view, 2>{ "metadata", "eligible" };
        }
        auto support_dependencies() const noexcept
        {
            ++observation->declaration_reads[2];
            return std::array<std::string_view, 1>{ "StaticDefault" };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            ++observation->compile_reads;
            observation->inspect(context);
            return {};
        }
        static givm::program_entry handle(const definition_type&,
            givm::round_started&, givm::handle_context<givm::support_view>&, std::uint32_t = 0)
        {
            return {};
        }
        static givm::support_state query(const definition_type&, const givm::support_state_limit&)
        {
            return { .count = 7, .round_usages = 2 };
        }
    };

    struct metadata_default_support_source
    {
        using definition_category = givm::support_view;
        struct definition_type {};
        metadata_observation* observation;

        std::string_view name() const noexcept
        {
            ++observation->declaration_reads[0];
            return "StaticDefault";
        }
        auto tags() const noexcept
        {
            ++observation->declaration_reads[1];
            return std::array<std::string_view, 0>{};
        }
        auto support_dependencies() const noexcept
        {
            ++observation->declaration_reads[2];
            return std::array<std::string_view, 0>{};
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            ++observation->compile_reads;
            observation->inspect(context);
            return {};
        }
    };

    struct dynamic_metadata_support_source : metadata_support_source
    {
        static constexpr bool is_dynamic = true;
        bool enabled;

        template<class TView, class TEvent>
        bool can_handle() const noexcept
        {
            return enabled && std::same_as<TView, givm::support_view> && std::same_as<TEvent, givm::round_started>;
        }
        template<class TQuery>
        bool can_query() const noexcept
        {
            return enabled && std::same_as<TQuery, givm::support_state_limit>;
        }

        using metadata_support_source::handle;
        template<class TView, class TEvent>
        static givm::program_entry handle(const definition_type&, TEvent&, givm::handle_context<TView>&, std::uint32_t = 0)
        {
            FAIL("A disabled dynamic handler was invoked");
            std::unreachable();
        }
    };

    struct metadata_summary_source
    {
        using definition_category = givm::history_summary_definition;
        struct definition_type {};
        metadata_observation* observation;

        std::string_view name() const noexcept
        {
            ++observation->declaration_reads[0];
            return "MetadataObserver";
        }
        auto tags() const noexcept
        {
            ++observation->declaration_reads[1];
            return std::array<std::string_view, 0>{};
        }
        auto support_dependencies() const noexcept
        {
            ++observation->declaration_reads[2];
            return std::array<std::string_view, 3>{ "StaticCustom", "DynamicEnabled", "DynamicDisabled" };
        }
        auto layout(const givm::definition_compile_context& context) const
        {
            ++observation->layout_reads;
            observation->inspect(context);
            return givm::history_summary_layout{};
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            ++observation->compile_reads;
            observation->inspect(context);
            return {};
        }
        static void handle(const definition_type&, givm::history_summary_state,
            const givm::history_summary_initialization&, const givm::table&, const givm::definition_library&)
        {}
    };

    struct zero_random
    {
        std::uint32_t operator()() const noexcept
        {
            return 0;
        }
    };
}

TEST_CASE("definition compile context resolves declared dependencies", "[source_view]")
{
    dependency_observation observation;
    const dependent_card_source card{ .observation = &observation };
    const tagged_support_source alpha{
        .source_name = "AlphaSupport",
        .source_tags = { "selected", "ordinary" },
        .tag_count = 2
    };
    const tagged_support_source beta{
        .source_name = "BetaSupport",
        .source_tags = { "selected", "excluded" },
        .tag_count = 2
    };

    auto source_library = givm_test::make_source_library();
    REQUIRE(source_library.add(card, alpha, beta));
    const auto program = std::tuple{ givm::draw_cards{ .position = 0, .count = 1 }, givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } };
    const auto [library, id_map] = givm_test::require_success(compile(source_library, givm_test::basic_sources, program, program, givm::compile_mode::normal));
    const auto card_id = id_map.get_id<givm::card_definition>(card.name());

    givm::table table{ { .self_player = givm::player_id{ 0 } } };
    load_deck(table, library, givm::linked_deck{ .cards = { card_id } }, {});
    zero_random random_source;
    givm_test::executor_driver executor;
    executor.start(library, table);
    REQUIRE(executor.advance(library, table, random_source) == givm::execution_state::finished);
    REQUIRE(table[givm::player_id{ 0 }].hand_card_count() == 1);
    REQUIRE(observation.handled);
    CHECK(observation.alpha_support.value() == id_map.get_id<givm::support_view>("AlphaSupport").value());
    CHECK(observation.beta_support.value() == id_map.get_id<givm::support_view>("BetaSupport").value());
    CHECK(observation.chosen_tag.value() == id_map.get_tag_id("chosen").value());

    REQUIRE(observation.filtered_supports.size() == 1);
    CHECK(
        observation.filtered_supports.front().value()
        == id_map.get_id<givm::support_view>("AlphaSupport").value()
    );
}

TEST_CASE("definition compile context rejects undeclared dependency queries", "[source_view]")
{
    const undeclared_dependency_source source;
    const tagged_support_source support{ .source_name = "MissingDeclaration" };
    auto source_library = givm_test::make_source_library();
    REQUIRE(source_library.add(source, support));
    const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
    const auto result = compile(source_library, givm_test::basic_sources, program, program, givm::compile_mode::normal);
    REQUIRE_FALSE(result);
    REQUIRE(result.error().size() == 1);
    const auto& error = result.error().front();
    CHECK(error.location.stage == givm::compile_stage::definition);
    REQUIRE(error.location.source);
    CHECK(error.location.source->name == std::string{ source.name() });
    const auto* reason = std::get_if<givm::definition_resolution_error>(&error.reason);
    REQUIRE(reason);
    CHECK(reason->cause == givm::definition_resolution_error::reason::undeclared_dependency);
    CHECK(reason->definition.category_index == givm::definition_types::index_of<givm::support_view>());
    CHECK(reason->definition.name == "MissingDeclaration");
}

TEST_CASE("compiled definitions expose only enabled source handlers", "[source_view]")
{
    std::size_t capability_checks = 0;
    const selectable_handler_source enabled{ "Enabled", true, &capability_checks };
    const selectable_handler_source disabled{ "Disabled", false, &capability_checks };
    const givm::definition_source_view<givm::support_view> enabled_view{ enabled };
    const givm::definition_source_view<givm::support_view> disabled_view{ disabled };
    CHECK(bool(enabled_view.name() == "Enabled"));
    CHECK(bool(disabled_view.name() == "Disabled"));
    CHECK(capability_checks == 0);

    auto source_library = givm_test::make_source_library();
    REQUIRE(source_library.add(enabled, disabled));
    const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
    const auto [library, id_map] = givm_test::require_success(compile(source_library, givm_test::basic_sources, program, program, givm::compile_mode::normal));

    CHECK(library[id_map.get_id<givm::support_view>(enabled.name())].can_handle<givm::round_started, givm::support_view>());
    CHECK_FALSE(
        library[id_map.get_id<givm::support_view>(enabled.name())].can_handle<givm::damage_effect, givm::support_view>()
    );
    CHECK_FALSE(
        library[id_map.get_id<givm::support_view>(disabled.name())].can_handle<givm::round_started, givm::support_view>()
    );
    CHECK(capability_checks > 0);
    const auto default_state = givm::query_default(givm::support_state_limit{});
    const auto disabled_state = library[id_map.get_id<givm::support_view>(disabled.name())]
        .query(givm::support_state_limit{});
    CHECK(disabled_state.count == default_state.count);
    CHECK(disabled_state.round_usages == default_state.round_usages);
}

TEST_CASE("static handler availability depends on the implementation alone", "[source_view]")
{
    std::uint32_t capability_checks = 0;
    const static_handler_source implicit_source{ "ImplicitStatic", &capability_checks };
    const explicitly_static_handler_source explicit_source{ { "ExplicitStatic", &capability_checks } };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(implicit_source, explicit_source));
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal));

    CHECK(library[ids.get_id<givm::support_view>(implicit_source.name())]
        .can_handle<givm::round_started, givm::support_view>());
    CHECK(library[ids.get_id<givm::support_view>(explicit_source.name())]
        .can_handle<givm::round_started, givm::support_view>());
    CHECK_FALSE(library[ids.get_id<givm::support_view>(implicit_source.name())]
        .can_handle<givm::damage_effect, givm::support_view>());
    CHECK(capability_checks == 0);
}

TEST_CASE("definition metadata reuses registered declarations before every layout and compile", "[source_view][metadata]")
{
    const bool selected = GENERATE(false, true);
    metadata_observation observation;
    const metadata_support_source static_custom{ "StaticCustom", &observation };
    const metadata_default_support_source static_default{ &observation };
    const dynamic_metadata_support_source enabled{ { "DynamicEnabled", &observation }, true };
    const dynamic_metadata_support_source disabled{ { "DynamicDisabled", &observation }, false };
    const metadata_summary_source summary{ &observation };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(static_custom, static_default, enabled, disabled, summary));
    const auto registered_reads = observation.declaration_reads;
    for(const auto count : registered_reads) REQUIRE(count != 0);

    const std::array<std::string_view, 1> roots{ "MetadataObserver" };
    givm::definition_selection selection{};
    selection[givm::definition_types::index_of<givm::history_summary_definition>()] = roots;
    const auto [library, ids] = givm_test::require_success(selected
        ? compile(sources, givm_test::basic_sources, selection, std::tuple{}, std::tuple{}, givm::compile_mode::normal)
        : compile(sources, givm_test::basic_sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal));
    CHECK(observation.declaration_reads == registered_reads);
    CHECK(observation.layout_reads == 1);
    CHECK(observation.compile_reads == 5);
    for(const auto name : { "StaticCustom", "DynamicEnabled", "StaticDefault", "DynamicDisabled" })
    {
        CAPTURE(name);
        const bool custom = std::string_view{ name } == "StaticCustom" || std::string_view{ name } == "DynamicEnabled";
        const auto definition = library[ids.get_id<givm::support_view>(name)];
        CHECK(definition.can_handle<givm::round_started, givm::support_view>() == custom);
        const auto limit = definition.query(givm::support_state_limit{});
        const auto expected = custom ? givm::support_state{ .count = 7, .round_usages = 2 }
            : query_default(givm::support_state_limit{});
        CHECK(limit.count == expected.count);
        CHECK(limit.round_usages == expected.round_usages);
    }
}

TEST_CASE("definition compile context accepts heterogeneous tuples and homogeneous ranges", "[source_view]")
{
    program_observation observation;
    const programmed_card_source card{ &observation };
    const programmed_support_source support{ &observation };

    auto source_library = givm_test::make_source_library();
    REQUIRE(source_library.add(card, support));
    const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
    const auto [library, id_map] = givm_test::require_success(compile(source_library, givm_test::basic_sources, program, program, givm::compile_mode::normal));

    CHECK(observation.event_compiled);
    CHECK(observation.onpay_compiled);
    CHECK(
        observation.resolved_support.value()
        == id_map.get_id<givm::support_view>(support.name()).value()
    );
    CHECK(observation.first_event_entry_set);
    CHECK(observation.second_event_entry_set);
    CHECK(observation.onpay_entry_set);
}

TEST_CASE("root programs reject commands that consume invocation inputs", "[source_view][program-input]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool runtime_commands = GENERATE(false, true);
    const bool initialization = GENERATE(false, true);
    CAPTURE(mode, runtime_commands, initialization);
    const auto sources = givm_test::make_source_library();
    const auto valid = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
    const auto check_result = [&](const auto& result)
    {
        REQUIRE_FALSE(result);
        REQUIRE(result.error().size() == 1);
        const auto& error = result.error().front();
        CHECK(error.location.stage == givm::compile_stage::program);
        CHECK(error.location.program == (initialization ? givm::program_kind::initialization : givm::program_kind::round));
        CHECK(error.location.command_index == 0);
        CHECK_FALSE(error.location.source);
        const auto* reason = std::get_if<givm::set_active_character::error_type>(&error.reason);
        REQUIRE(reason);
        CHECK(reason->cause == givm::set_active_character::error_type::reason::dynamic_input_in_root);
    };
    const auto check = [&](const auto& invalid)
    {
        if(initialization)
            check_result(compile(sources, givm_test::basic_sources, invalid, valid, mode));
        else
            check_result(compile(sources, givm_test::basic_sources, valid, invalid, mode));
    };
    if(runtime_commands)
        check(std::vector<givm::any_command>{ givm::set_active_character{} });
    else
        check(std::tuple{ givm::set_active_character{} });
}
}
