#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../test_source_library.hpp"
#include "../table/test_passive_skill.hpp"
#include "test_character_source.hpp"

namespace givm_test::executor::history_summaries
{
constexpr std::array<std::size_t, 1> draw_positions_1{ 0 };

namespace
{
    constexpr std::string_view mixed_name = "MixedHistory";
    constexpr std::string_view magic_name = "DistinctGeneratedCardHistory";

    struct plain_card_source
    {
        using definition_category = givm::card_definition;
        struct definition_type {};
        std::string_view source_name;

        std::string_view name() const { return source_name; }
        definition_type compile(givm::definition_compile_context&) const { return {}; }
        static givm::program_entry handle(const definition_type&,
            givm::card_effect&, givm::handle_context<givm::hand_card_view>&, std::uint32_t = 0)
        {
            return {};
        }
    };

    struct mixed_summary_source
    {
        using definition_category = givm::history_summary_definition;
        struct definition_type
        {
            std::size_t* initializations;
            givm::history_field_key<std::uint8_t> small;
            givm::history_field_key<std::uint64_t> wide;
            givm::history_field_key<std::uint32_t[]> counts;
            givm::history_field_key<std::uint64_t[]> empty;
            givm::history_field_key<std::uint16_t[]> cards;
        };

        std::size_t* initializations;
        std::string_view source_name = mixed_name;

        std::string_view name() const { return source_name; }
        auto layout(const givm::definition_compile_context& context) const
        {
            return givm::history_summary_layout{
                givm::history_field<std::uint8_t>("small"),
                givm::history_field<std::uint64_t>("wide"),
                givm::history_array<std::uint32_t>("counts", 2),
                givm::history_array<std::uint64_t>("empty", 0),
                givm::history_array<std::uint16_t>("cards", context.definition_count<givm::card_definition>())
            };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { initializations, context.history_field<std::uint8_t>("small"),
                context.history_field<std::uint64_t>("wide"), context.history_field<std::uint32_t[]>("counts"),
                context.history_field<std::uint64_t[]>("empty"), context.history_field<std::uint16_t[]>("cards") };
        }
        static void handle(const definition_type& data, givm::history_summary_state state,
            const givm::history_summary_initialization&, const givm::table& table, const givm::definition_library&)
        {
            ++*data.initializations;
            CHECK(state[data.empty].empty());
            state[data.small] = 7;
            state[data.wide] = 0xFEDCBA9876543210ull;
            for(const auto player : table.players())
                state[data.counts][player.id().index] = static_cast<std::uint32_t>(player.deck_card_count());
            for(auto& value : state[data.cards]) value = 31;
        }
        static void handle(const definition_type& data, givm::history_summary_state state,
            const givm::round_started&, const givm::table&, const givm::definition_library&)
        {
            ++state[data.wide];
            ++state[data.counts][0];
        }
    };

    struct deferred_summary_source
    {
        using definition_category = givm::history_summary_definition;
        struct definition_type
        {
            givm::history_field_key<std::uint32_t> value;
            std::size_t* recordings;
        };
        std::size_t* recordings;

        std::string_view name() const { return "DeferredHistory"; }
        auto layout(const givm::definition_compile_context&) const
        {
            return givm::history_summary_layout{ givm::history_field<std::uint32_t>("value") };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.history_field<std::uint32_t>("value"), recordings };
        }
        static void handle(const definition_type& data, givm::history_summary_state state,
            const givm::round_started&, const givm::table&, const givm::definition_library&)
        {
            ++*data.recordings;
            state[data.value] = 37;
        }
    };

    struct dependent_card_source
    {
        using definition_category = givm::card_definition;
        struct definition_type { givm::history_value_key<std::uint64_t> value; };
        std::string_view source_name = "HistoryConsumer";
        std::string_view summary_name = mixed_name;

        std::string_view name() const { return source_name; }
        auto history_summary_dependencies() const { return std::array{ summary_name }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_history_field<std::uint64_t>(summary_name, "wide") };
        }
    };

    struct dynamic_summary_source
    {
        using definition_category = givm::history_summary_definition;
        static constexpr bool is_dynamic = true;
        struct definition_type { givm::dynamic_history_field hits; };

        std::string_view source_name;
        bool enabled;
        std::size_t payload_count;
        std::size_t* capability_checks;

        std::string_view name() const { return source_name; }
        auto layout(const givm::definition_compile_context&) const
        {
            return givm::history_summary_layout{
                givm::history_array<std::uint64_t>("payload", payload_count),
                givm::history_field<std::uint32_t>("hits")
            };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.history_field("hits") };
        }
        template<class TEvent>
        bool can_handle() const
        {
            ++*capability_checks;
            return std::same_as<TEvent, givm::history_summary_initialization>
                || (enabled && std::same_as<TEvent, givm::round_started>);
        }
        template<class TEvent>
        static void handle(const definition_type& data, givm::history_summary_state state,
            const TEvent&, const givm::table&, const givm::definition_library&)
        {
            if constexpr(std::same_as<TEvent, givm::history_summary_initialization>)
                state[std::get<givm::history_field_key<std::uint32_t>>(data.hits)] = 0;
            else
                ++state[std::get<givm::history_field_key<std::uint32_t>>(data.hits)];
        }
    };

    struct history_observer_source
    {
        using definition_category = givm::card_definition;
        struct definition_type
        {
            givm::history_value_key<std::uint32_t> hits;
            givm::program_entry pause;
            std::vector<std::uint32_t>* observed;
        };
        std::vector<std::uint32_t>* observed;

        std::string_view name() const { return "HistoryObserver"; }
        auto history_summary_dependencies() const { return std::array{ std::string_view{ "DynamicEnabled" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { std::get<givm::history_value_key<std::uint32_t>>(context.resolve_history_field("DynamicEnabled", "hits")),
                context.add_program(std::tuple{ givm::replace_cards{ givm::player_id{ 0 } } }), observed };
        }
        static givm::program_entry handle(const definition_type& data,
            givm::round_started&, givm::handle_context<givm::deck_card_view>& context, std::uint32_t = 0)
        {
            const auto hits = context.table()[data.hits];
            data.observed->push_back(hits);
            return hits == 1 ? context.invoke(data.pause) : givm::program_entry{};
        }
    };

    template<class T>
    struct scalar_summary_source
    {
        using definition_category = givm::history_summary_definition;
        struct definition_type
        {
            givm::history_field_key<T> field;
            T initial_value;
        };
        T initial_value;

        std::string_view name() const { return "ScalarHistory"; }
        auto layout(const givm::definition_compile_context&) const
        {
            return givm::history_summary_layout{ givm::history_field<T>("value") };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.history_field<T>("value"), initial_value };
        }
        static void handle(const definition_type& data, givm::history_summary_state state,
            const givm::history_summary_initialization&, const givm::table&, const givm::definition_library&)
        {
            state[data.field] = data.initial_value;
        }
    };

    struct distinct_card_summary_source
    {
        using definition_category = givm::history_summary_definition;
        struct definition_type
        {
            std::size_t words;
            givm::history_field_key<std::uint32_t[]> counts;
            givm::history_field_key<std::uint64_t[]> seen;
        };

        std::string_view name() const { return magic_name; }
        auto layout(const givm::definition_compile_context& context) const
        {
            const auto words = (context.definition_count<givm::card_definition>() + 63) / 64;
            return givm::history_summary_layout{
                givm::history_array<std::uint32_t>("counts", 2),
                givm::history_array<std::uint64_t>("seen", words * 2)
            };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { (context.definition_count<givm::card_definition>() + 63) / 64,
                context.history_field<std::uint32_t[]>("counts"), context.history_field<std::uint64_t[]>("seen") };
        }
        static void handle(const definition_type& data, givm::history_summary_state state,
            const givm::history_summary_initialization&, const givm::table& table, const givm::definition_library&)
        {
            for(auto& value : state[data.counts]) value = 0;
            for(auto& value : state[data.seen]) value = 0;
            for(const auto player : table.players())
                for(const auto card : player.deck_cards())
                {
                    const auto index = card.definition_id().value();
                    state[data.seen][player.id().index * data.words + index / 64] |= std::uint64_t{ 1 } << (index % 64);
                }
        }
        static void handle(const definition_type& data, givm::history_summary_state state,
            const givm::card_played& event, const givm::table&, const givm::definition_library&)
        {
            const auto player = event.card.player_id.index;
            const auto index = event.definition_id.value();
            auto& word = state[data.seen][player * data.words + index / 64];
            const auto mask = std::uint64_t{ 1 } << (index % 64);
            if((word & mask) == 0)
            {
                word |= mask;
                ++state[data.counts][player];
            }
        }
    };

    struct future_card_source
    {
        using definition_category = givm::card_definition;
        struct definition_type { givm::history_value_key<std::uint32_t[]> counts; };

        std::string_view name() const { return "FutureCard"; }
        auto history_summary_dependencies() const { return std::array{ magic_name }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_history_field<std::uint32_t[]>(magic_name, "counts") };
        }
        static givm::target_validation query(const definition_type& data, const givm::card_target_validation& query)
        {
            return query.table[data.counts][query.card.id().player_id.index] != 0
                ? givm::target_validation::valid_complete : givm::target_validation::invalid;
        }
        static givm::program_entry handle(const definition_type&,
            givm::card_effect&, givm::handle_context<givm::hand_card_view>&, std::uint32_t = 0)
        {
            return {};
        }
    };

    struct generation_driver_source
    {
        using definition_category = givm::card_definition;
        struct definition_type
        {
            givm::definition_id<givm::card_definition> first;
            givm::history_value_key<std::uint32_t[]> counts;
            givm::program_entry generate;
            givm::program_entry future;
            std::vector<std::uint32_t>* observed;
        };
        std::vector<std::uint32_t>* observed;

        std::string_view name() const { return "GenerationDriver"; }
        auto card_dependencies() const
        {
            return std::array{ std::string_view{ "GeneratedA" }, std::string_view{ "GeneratedB" },
                std::string_view{ "FutureCard" } };
        }
        auto history_summary_dependencies() const { return std::array{ magic_name }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto first = context.resolve_id<givm::card_definition>("GeneratedA");
            const auto second = context.resolve_id<givm::card_definition>("GeneratedB");
            const auto future = context.resolve_id<givm::card_definition>("FutureCard");
            return { first, context.resolve_history_field<std::uint32_t[]>(magic_name, "counts"),
                context.add_program(std::tuple{
                    givm::create_hand_card{ .definition = first }, givm::create_hand_card{ .definition = first },
                    givm::create_hand_card{ .definition = second } }),
                context.add_program(std::tuple{ givm::create_hand_card{ .definition = future } }), observed };
        }
        static givm::program_entry handle(const definition_type& data,
            givm::round_started&, givm::handle_context<givm::deck_card_view>& context, std::uint32_t = 0)
        {
            return context.invoke(data.generate);
        }
        static givm::program_entry handle(const definition_type& data,
            givm::card_played& event, givm::handle_context<givm::deck_card_view>& context, std::uint32_t = 0)
        {
            data.observed->push_back(context.table()[data.counts][event.card.player_id.index]);
            return event.definition_id == data.first ? context.invoke(data.future) : givm::program_entry{};
        }
    };

    givm::execution_state advance(givm_test::executor_driver& executor, const givm::definition_library& library, givm::table& table)
    {
        auto random = [] { return std::uint32_t{ 0 }; };
        for(;;)
        {
            const auto state = executor.advance(library, table, random);
            if(state == givm::execution_state::action_selection || state == givm::execution_state::card_selection
                || state == givm::execution_state::finished)
                return state;
        }
    }

    enum class field_error { missing, type, shape, undeclared, missing_summary };

    struct invalid_consumer_source
    {
        using definition_category = givm::card_definition;
        struct definition_type {};
        field_error error;
        std::string_view name() const { return "InvalidHistoryConsumer"; }
        auto history_summary_dependencies() const
        {
            static constexpr std::array dependencies{ mixed_name };
            return error == field_error::undeclared ? std::span<const std::string_view>{}
                : std::span<const std::string_view>{ dependencies };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            switch(error)
            {
            case field_error::missing: (void)context.resolve_history_field<std::uint32_t>(mixed_name, "missing"); break;
            case field_error::type: (void)context.resolve_history_field<std::uint32_t>(mixed_name, "wide"); break;
            case field_error::shape: (void)context.resolve_history_field<std::uint32_t>(mixed_name, "counts"); break;
            case field_error::undeclared: (void)context.resolve_history_field<std::uint64_t>(mixed_name, "wide"); break;
            case field_error::missing_summary: (void)context.resolve_history_field<std::uint64_t>("Missing", "wide"); break;
            }
            return {};
        }
    };

    struct defeat_summary_source
    {
        using definition_category = givm::history_summary_definition;
        struct definition_type { givm::history_field_key<std::uint32_t[]> counts; };

        std::string_view name() const { return "DefeatHistory"; }
        auto layout(const givm::definition_compile_context&) const
        {
            return givm::history_summary_layout{ givm::history_array<std::uint32_t>("counts", 2) };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.history_field<std::uint32_t[]>("counts") };
        }
        static void handle(const definition_type& data, givm::history_summary_state state,
            const givm::history_summary_initialization&, const givm::table&, const givm::definition_library&)
        {
            for(auto& value : state[data.counts]) value = 0;
        }
        static void handle(const definition_type& data, givm::history_summary_state state,
            const givm::after_damage& event, const givm::table&, const givm::definition_library&)
        {
            if(event.defeated) ++state[data.counts][event.target.player_id.index];
        }
    };

    struct defeat_observer_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            givm::history_value_key<std::uint32_t[]> counts;
            givm::program_entry revive;
            bool should_revive;
            std::vector<std::uint32_t>* observed;
        };
        bool should_revive;
        std::vector<std::uint32_t>* observed;

        std::string_view name() const { return "DefeatHistoryObserver"; }
        auto history_summary_dependencies() const { return std::array{ std::string_view{ "DefeatHistory" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_history_field<std::uint32_t[]>("DefeatHistory", "counts"),
                context.add_program(std::tuple{ givm::heal{} }), should_revive, observed };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data,
            givm::character_will_be_defeated& event, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity().character();
            CHECK(context.table()[data.counts][event.target.player_id.index] == 0);
            if(not data.should_revive) return {};
            return context.invoke(data.revive, givm::heal_input{ std::array{ givm::heal_input::item{ .source = self.id(), .target = event.target, .value = 2 , .kind = givm::healing_kind::prevent_defeat} } });
        }
        static givm::program_entry handle(const definition_type& data,
            givm::after_damage& event, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            if(event.defeated) data.observed->push_back(context.table()[data.counts][event.target.player_id.index]);
            return {};
        }
    };
}

TEST_CASE("starting initializes history after both decks and copies preserve initialized state", "[history_summary]")
{
    std::size_t initializations = 0;
    const mixed_summary_source summary{ &initializations };
    const plain_card_source first{ "FirstCard" }, second{ "SecondCard" };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(summary, first, second));
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, givm::compile_mode::normal));
    const auto id = ids.get_id<givm::history_summary_definition>(mixed_name);
    const auto small = library.history_field<std::uint8_t>(id, "small");
    const auto wide = library.history_field<std::uint64_t>(id, "wide");
    const auto counts = library.history_field<std::uint32_t[]>(id, "counts");
    const auto cards = library.history_field<std::uint16_t[]>(id, "cards");
    const auto empty = library.history_field<std::uint64_t[]>(id, "empty");
    givm::table table;
    load_deck(table, library, { .cards = { ids.get_id<givm::card_definition>(first.name()) } },
        { .cards = { ids.get_id<givm::card_definition>(second.name()), ids.get_id<givm::card_definition>(second.name()) } });
    CHECK(initializations == 0);
    givm_test::executor_driver executor;
    executor.start(library, table);
    CHECK(initializations == 1);
    CHECK(table[small] == 7);
    CHECK(table[wide] == 0xFEDCBA9876543210ull);
    CHECK(table[counts][0] == 1);
    CHECK(table[counts][1] == 2);
    REQUIRE(table[cards].size() == 2);
    CHECK(table[cards][0] == 31);
    CHECK(table[cards][1] == 31);
    CHECK(table[empty].empty());
    STATIC_REQUIRE(std::same_as<decltype(table[wide]), const std::uint64_t&>);
    STATIC_REQUIRE(std::same_as<decltype(table[counts]), std::span<const std::uint32_t>>);

    auto branch = table;
    auto copied_library = library;
    auto branch_executor = executor;
    REQUIRE(advance(branch_executor, copied_library, branch) == givm::execution_state::finished);
    CHECK(branch[wide] == 0xFEDCBA9876543211ull);
    CHECK(branch[counts][0] == 2);
    CHECK(table[wide] == 0xFEDCBA9876543210ull);
    CHECK(table[counts][0] == 1);
    CHECK(initializations == 1);
    givm::table another;
    load_deck(another, copied_library, {}, {});
    CHECK(initializations == 1);
    givm_test::executor_driver another_executor;
    another_executor.start(copied_library, another);
    CHECK(initializations == 2);
    CHECK(another[wide] == 0xFEDCBA9876543210ull);
    CHECK(another[counts][0] == 0);
    CHECK(another[counts][1] == 0);

#ifndef NDEBUG
    CHECK_THROWS_AS(library.history_field<std::uint32_t>(id, "wide"), givm::history_access_error);
    CHECK_THROWS_AS(library.history_field<std::uint32_t>(id, "counts"), givm::history_access_error);
    CHECK_THROWS_AS(library.history_field<std::uint32_t>(id, "missing"), givm::history_access_error);
#endif
}

TEST_CASE("history without initialization writes fields before reading them", "[history_summary]")
{
    std::size_t recordings = 0;
    const deferred_summary_source summary{ &recordings };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(summary));
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, givm::compile_mode::normal));
    givm::table table;
    load_deck(table, library, {}, {});
    givm_test::executor_driver executor;
    executor.start(library, table);
    CHECK(recordings == 0);
    REQUIRE(advance(executor, library, table) == givm::execution_state::finished);
    CHECK(recordings == 1);
    const auto value = library.history_field<std::uint32_t>(
        ids.get_id<givm::history_summary_definition>(summary.name()), "value");
    CHECK(table[value] == 37);
}

TEST_CASE("history dependencies select only needed summaries and validate field contracts", "[history_summary]")
{
    std::size_t selected_initializations = 0, unused_initializations = 0;
    const mixed_summary_source summary{ &selected_initializations }, unused{ &unused_initializations, "UnusedHistory" };
    const dependent_card_source consumer;
    const plain_card_source unrelated{ "UnrelatedCard" };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(consumer, summary, unused, unrelated));
    const std::array roots{ consumer.name() };
    givm::definition_selection selection{};
    selection[givm::definition_types::index_of<givm::card_definition>()] = roots;
    const auto program = std::tuple{ givm::settle{}, givm::end_game{ givm::game_result::both_loss } };
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources, selection, program, std::tuple{}, givm::compile_mode::normal));
    CHECK(ids.has<givm::history_summary_definition>(mixed_name));
    CHECK_FALSE(ids.has<givm::history_summary_definition>(unused.name()));
    CHECK_FALSE(ids.has<givm::card_definition>(unrelated.name()));
    givm::table table;
    load_deck(table, library, {}, {});
    CHECK(selected_initializations == 0);
    givm_test::executor_driver executor;
    executor.start(library, table);
    CHECK(selected_initializations == 1);
    CHECK(unused_initializations == 0);
    const auto id = ids.get_id<givm::history_summary_definition>(mixed_name);
    CHECK(table[library.history_field<std::uint16_t[]>(id, "cards")].size() == 1);

    const auto error = GENERATE(field_error::missing, field_error::type, field_error::shape,
        field_error::undeclared, field_error::missing_summary);
    const invalid_consumer_source invalid{ error };
    auto invalid_sources = givm_test::make_source_library();
    REQUIRE(invalid_sources.add(invalid, summary));
    const auto result = compile(invalid_sources, givm_test::basic_sources, program, std::tuple{}, givm::compile_mode::normal);
    REQUIRE_FALSE(result);
    REQUIRE(result.error().size() == 1);
    const auto& diagnostic = result.error().front();
    CHECK(diagnostic.location.stage == givm::compile_stage::definition);
    REQUIRE(diagnostic.location.source);
    CHECK(diagnostic.location.source->name == std::string{ invalid.name() });
    if(error == field_error::missing)
    {
        const auto* reason = std::get_if<givm::history_field_not_found>(&diagnostic.reason);
        REQUIRE(reason);
        CHECK(reason->summary == std::string{ mixed_name });
        CHECK(reason->field == "missing");
    }
    else if(error == field_error::type || error == field_error::shape)
    {
        const auto* reason = std::get_if<givm::history_field_type_mismatch>(&diagnostic.reason);
        REQUIRE(reason);
        CHECK(reason->summary == std::string{ mixed_name });
        CHECK(reason->expected_type == "uint32_t");
        CHECK(reason->actual_type == (error == field_error::type ? "uint64_t" : "uint32_t[]"));
    }
    else
    {
        const auto* reason = std::get_if<givm::definition_resolution_error>(&diagnostic.reason);
        REQUIRE(reason);
        CHECK(reason->cause == givm::definition_resolution_error::reason::undeclared_dependency);
        CHECK(reason->definition.name == std::string{ error == field_error::undeclared ? mixed_name : "Missing" });
    }
}

TEST_CASE("dynamic history adapters update once after resumable ordinary responses", "[history_summary]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    std::size_t capability_checks = 0;
    const dynamic_summary_source enabled{ "DynamicEnabled", true, 3, &capability_checks };
    const dynamic_summary_source disabled{ "DynamicDisabled", false, 17, &capability_checks };
    std::vector<std::uint32_t> observed;
    const history_observer_source observer{ &observed };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(observer, enabled, disabled));
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, mode));
    REQUIRE(capability_checks != 0);
    const auto compiled_checks = capability_checks;
    const auto enabled_id = ids.get_id<givm::history_summary_definition>(enabled.name());
    const auto disabled_id = ids.get_id<givm::history_summary_definition>(disabled.name());
    const auto hits = library.history_field<std::uint32_t>(enabled_id, "hits");
    givm::table table;
    load_deck(table, library, { .cards = { ids.get_id<givm::card_definition>(observer.name()) } }, {});
    givm_test::executor_driver executor;
    executor.start(library, table);
    CHECK(table[hits] == 0);
    CHECK(table[library.history_field<std::uint64_t[]>(enabled_id, "payload")].size() == 3);
    CHECK(table[library.history_field<std::uint64_t[]>(disabled_id, "payload")].size() == 17);
    REQUIRE(advance(executor, library, table) == givm::execution_state::card_selection);
    CHECK(table[hits] == 1);
    CHECK(observed == std::vector<std::uint32_t>{ 1 });
    auto branch = table;
    auto branch_executor = executor;
    branch_executor.submitted(branch_executor.view_in<givm::execution_state::card_selection>().select(library, branch, givm_test::zero_random, {}));
    REQUIRE(advance(branch_executor, library, branch) == givm::execution_state::finished);
    CHECK(branch[hits] == 2);
    CHECK(table[hits] == 1);
    CHECK(observed == std::vector<std::uint32_t>{ 1, 2 });
    CHECK(branch[library.history_field<std::uint32_t>(disabled_id, "hits")] == 0);
    CHECK(capability_checks == compiled_checks);
}

TEST_CASE("history copy assignment and moves preserve different runtime layouts", "[history_summary]")
{
    std::size_t capability_checks = 0;
    const auto make_library = [&capability_checks](std::size_t payload_count)
    {
        const dynamic_summary_source summary{ "DynamicHistory", true, payload_count, &capability_checks };
        auto sources = givm_test::make_source_library();
        REQUIRE(sources.add(summary));
        return givm_test::require_success(compile(sources, givm_test::basic_sources,
            std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } },
            std::tuple{}, givm::compile_mode::normal));
    };
    const auto [small_library, small_ids] = make_library(1);
    const auto [large_library, large_ids] = make_library(33);
    const auto small_id = small_ids.get_id<givm::history_summary_definition>("DynamicHistory");
    const auto large_id = large_ids.get_id<givm::history_summary_definition>("DynamicHistory");
    const auto small_hits = small_library.history_field<std::uint32_t>(small_id, "hits");
    const auto large_hits = large_library.history_field<std::uint32_t>(large_id, "hits");
    givm::table small, large;
    load_deck(small, small_library, {}, {});
    load_deck(large, large_library, {}, {});
    givm_test::executor_driver small_executor, large_executor;
    small_executor.start(small_library, small);
    large_executor.start(large_library, large);
    REQUIRE(advance(large_executor, large_library, large) == givm::execution_state::finished);
    auto original_small = small;
    small = large;
    CHECK(small[large_hits] == 1);
    CHECK(small[large_library.history_field<std::uint64_t[]>(large_id, "payload")].size() == 33);
    large = original_small;
    CHECK(large[small_hits] == 0);
    CHECK(large[small_library.history_field<std::uint64_t[]>(small_id, "payload")].size() == 1);
    auto moved = std::move(small);
    CHECK(moved[large_hits] == 1);
    original_small = std::move(moved);
    CHECK(original_small[large_hits] == 1);
    CHECK(large[small_hits] == 0);

    const auto copy_assign = [](givm::table& target, const givm::table& source) { target = source; };
    copy_assign(original_small, original_small);
    CHECK(original_small[large_hits] == 1);

    const givm::table empty;
    auto formerly_populated = original_small;
    formerly_populated = empty;
    CHECK(formerly_populated.state().round_number == empty.state().round_number);
    CHECK(formerly_populated[givm::player_id{ 0 }].characters().empty());
    copy_assign(formerly_populated, formerly_populated);
    formerly_populated = original_small;
    CHECK(formerly_populated[large_hits] == 1);
    givm::table initially_empty;
    initially_empty = original_small;
    CHECK(initially_empty[large_hits] == 1);

    const auto make_scalar_library = []<class T>(T value)
    {
        auto sources = givm_test::make_source_library();
        REQUIRE(sources.add(scalar_summary_source<T>{ value }));
        return givm_test::require_success(compile(sources, givm_test::basic_sources, std::tuple{ givm::settle{}, givm::end_game{ givm::game_result::both_loss } },
            std::tuple{}, givm::compile_mode::normal));
    };
    const auto [double_library, double_ids] = make_scalar_library(1.25);
    const auto [integer_library, integer_ids] = make_scalar_library(std::int32_t{ 42 });
    const auto double_field = double_library.history_field<double>(
        double_ids.get_id<givm::history_summary_definition>("ScalarHistory"), "value");
    const auto integer_field = integer_library.history_field<std::int32_t>(
        integer_ids.get_id<givm::history_summary_definition>("ScalarHistory"), "value");
    givm::table doubles, integers;
    givm_test::executor_driver double_executor, integer_executor;
    double_executor.start(double_library, doubles);
    integer_executor.start(integer_library, integers);
    auto changing_layout = doubles;
    changing_layout = integers;
    CHECK(changing_layout[integer_field] == 42);
    changing_layout = doubles;
    CHECK(changing_layout[double_field] == 1.25);
    copy_assign(changing_layout, changing_layout);
    CHECK(changing_layout[double_field] == 1.25);
}

TEST_CASE("card history excludes initial decks and is available to cards generated later", "[history_summary]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const distinct_card_summary_source summary;
    const plain_card_source initial{ "InitialCard" }, first{ "GeneratedA" }, second{ "GeneratedB" };
    const future_card_source future;
    const givm::test::initialized_character_source character;
    std::vector<std::uint32_t> observed;
    const generation_driver_source driver{ &observed };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(driver, future, summary, initial, first, second, character));
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources,
        std::tuple{ givm::draw_cards{ .position = 0, .count = 1 }, givm::start_round{}, givm::settle{}, givm::begin_action{} }, std::tuple{}, mode));
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    const auto initial_id = ids.get_id<givm::card_definition>(initial.name());
    const auto first_id = ids.get_id<givm::card_definition>(first.name());
    const auto second_id = ids.get_id<givm::card_definition>(second.name());
    const auto future_id = ids.get_id<givm::card_definition>(future.name());
    const auto character_id = ids.get_id<givm::character_view>(character.name());
    load_deck(table, library,
        { .cards = { ids.get_id<givm::card_definition>(driver.name()), initial_id }, .characters = { character_id } },
        { .cards = { first_id }, .characters = { character_id } });
    const auto history_id = ids.get_id<givm::history_summary_definition>(magic_name);
    const auto counts = library.history_field<std::uint32_t[]>(history_id, "counts");
    const auto seen = library.history_field<std::uint64_t[]>(history_id, "seen");
    givm_test::executor_driver executor;
    executor.start(library, table);
    const auto words = table[seen].size() / 2;
    const auto first_mask = std::uint64_t{ 1 } << (first_id.value() % 64);
    CHECK((table[seen][first_id.value() / 64] & first_mask) == 0);
    CHECK((table[seen][words + first_id.value() / 64] & first_mask) != 0);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    const auto initial_action = executor.view_in<givm::execution_state::action_selection>();
    REQUIRE(initial_action.card_count() == 4);
    for(std::size_t index = 0; index < initial_action.card_count(); ++index)
        CHECK(table[initial_action.card_id(index)].definition_id() != future_id);
    for(const auto expected : std::array{ std::pair{ initial_id, 0u }, std::pair{ first_id, 1u },
            std::pair{ first_id, 1u }, std::pair{ second_id, 2u } })
    {
        const auto action = executor.view_in<givm::execution_state::action_selection>();
        std::size_t index = 0;
        while(index < action.card_count() && table[action.card_id(index)].definition_id() != expected.first) ++index;
        REQUIRE(index < action.card_count());
        executor.submitted(action.play_card(library, table, givm_test::zero_random, index, {}));
        REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
        CHECK(table[counts][0] == expected.second);
        CHECK(table[counts][1] == 0);
    }
    CHECK(observed == std::vector<std::uint32_t>{ 0, 1, 1, 2 });
    const auto action = executor.view_in<givm::execution_state::action_selection>();
    std::size_t future_index = 0;
    while(future_index < action.card_count() && table[action.card_id(future_index)].definition_id() != future_id) ++future_index;
    REQUIRE(future_index < action.card_count());
    CHECK(action.card_targets_validate(library, table, future_index) == givm::target_validation::valid_complete);
}

TEST_CASE("defeat history records confirmed nonterminal defeats before ordinary responses", "[history_summary][dying]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    enum class outcome { defeated, revived, terminal };
    const auto expected = GENERATE(outcome::defeated, outcome::revived, outcome::terminal);
    const defeat_summary_source summary;
    std::vector<std::uint32_t> observed;
    const auto observer = givm::test::with_passive_skill(defeat_observer_source{ expected == outcome::revived, &observed });
    const givm::test::initialized_character_source victim{ "HistoryVictim", { .max_health = 10, .health = 1 } };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(summary, observer, observer.passive, victim));
    const std::array damages{ givm::deal_damage{
        .source = { givm::relative_player::self, 0 }, .target = { givm::relative_player::opponent, 0 },
        .value = 1, .type = givm::damage_type::physical } };
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources,
        std::tuple{ damages[0], givm::settle{}, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, mode));
    const auto victim_id = ids.get_id<givm::character_view>(victim.name());
    givm::linked_deck defenders{ .characters = { victim_id } };
    if(expected == outcome::defeated) defenders.characters.push_back(victim_id);
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } }, { .active_character = target } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(observer.name()) } }, defenders);
    const auto counts = library.history_field<std::uint32_t[]>(
        ids.get_id<givm::history_summary_definition>(summary.name()), "counts");
    givm_test::executor_driver executor;
    executor.start(library, table);
    auto state = givm_test::advance_selecting_first_alive(executor, library, table, givm_test::zero_random);
    if(mode == givm::compile_mode::observed)
    {
        REQUIRE(state == givm::execution_state::health_reduced);
        state = givm_test::advance_selecting_first_alive(executor, library, table, givm_test::zero_random);
    }
    REQUIRE(state == givm::execution_state::finished);
    CHECK(table[counts][0] == 0);
    CHECK(table[counts][1] == (expected == outcome::defeated ? 1 : 0));
    CHECK(observed == (expected == outcome::defeated ? std::vector<std::uint32_t>{ 1 } : std::vector<std::uint32_t>{}));
    CHECK(table[target].state().health == (expected == outcome::revived ? 2 : 0));
    CHECK(executor.view_in<givm::execution_state::finished>().result()
        == (expected == outcome::terminal ? givm::game_result::player_0_win : givm::game_result::both_loss));
}
}
