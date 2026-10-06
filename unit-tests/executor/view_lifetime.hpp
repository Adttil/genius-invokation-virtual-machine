#include <bitset>
#include <string>
#include <tuple>
#include <utility>
#include <variant>

#include <catch2/catch_test_macros.hpp>
#include <givm/executor.hpp>

namespace givm_test::executor::view_lifetime
{
    template<class TExecutor>
    concept has_public_step = requires(TExecutor& execution, const givm::definition_library& library,
        givm::table& table) { execution.step(library, table, givm_test::zero_random); };

    template<class TView>
    concept resumable = requires(TView view, const givm::definition_library& library, givm::table& table)
        { view.resume(library, table, givm_test::zero_random); };

    static_assert(not has_public_step<givm::executor>);
    static_assert(resumable<givm::execution_view<givm::execution_state::initialized>>);
    static_assert(not resumable<givm::execution_view<givm::execution_state::finished>>);

    inline auto selection_library()
    {
        auto sources = givm_test::make_source_library();
        return givm_test::require_success(compile(sources, givm_test::basic_sources,
            std::tuple{ givm::replace_cards{ .player = givm::player_id{ 0 } },
                givm::replace_cards{ .player = givm::player_id{ 0 } },
                givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, givm::compile_mode::normal));
    }

    TEST_CASE("start exposes an initialized pause which can be independently copied", "[execution-view][lifecycle]")
    {
        const auto [library, ids] = selection_library();
        givm::table table;
        givm::executor execution;
        const auto initialized = execution.start(library, table);
        auto branch = execution;
        auto branch_table = table;
        REQUIRE(initialized.resume(library, table, givm_test::zero_random) == givm::execution_state::card_selection);
        REQUIRE(branch.view_in<givm::execution_state::initialized>().resume(library, branch_table, givm_test::zero_random)
            == givm::execution_state::card_selection);
        REQUIRE(execution.view_in<givm::execution_state::card_selection>().select(library, table, givm_test::zero_random, {})
            == givm::execution_state::card_selection);
        REQUIRE(execution.view_in<givm::execution_state::card_selection>().select(library, table, givm_test::zero_random, {})
            == givm::execution_state::finished);
        CHECK(execution.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
        CHECK(branch.view_in<givm::execution_state::card_selection>().player() == givm::player_id{ 0 });
    }

#ifndef NDEBUG
    TEST_CASE("view diagnostics distinguish incorrect states and expired versions at the same state", "[execution-view][lifecycle][debug]")
    {
        const auto [library, ids] = selection_library();
        givm::table table;
        givm::executor execution;
        const auto initialized = execution.start(library, table);
        try
        {
            (void)execution.view_in<givm::execution_state::finished>();
            FAIL("an initialized executor must not expose a finished view");
        }
        catch(const givm::execution_view_error& error)
        {
            const auto* reason = std::get_if<givm::unexpected_execution_state>(&error.reason);
            REQUIRE(reason);
            CHECK(reason->expected == givm::execution_state::finished);
            CHECK(reason->actual == givm::execution_state::initialized);
            CHECK(std::string{ error.what() }.find("initialized") != std::string::npos);
        }
        REQUIRE(initialized.resume(library, table, givm_test::zero_random) == givm::execution_state::card_selection);
        const auto old = execution.view_in<givm::execution_state::card_selection>();
        REQUIRE(old.select(library, table, givm_test::zero_random, {}) == givm::execution_state::card_selection);
        try
        {
            (void)old.player();
            FAIL("the prior selection view must expire even when the state is unchanged");
        }
        catch(const givm::execution_view_error& error)
        {
            const auto* reason = std::get_if<givm::expired_execution_view>(&error.reason);
            REQUIRE(reason);
            CHECK(reason->expected_version != reason->actual_version);
        }
        REQUIRE_THROWS_AS(old.selection_validate(table, {}), givm::execution_view_error);
        REQUIRE_THROWS_AS(old.select(library, table, givm_test::zero_random, {}), givm::execution_view_error);
    }

    TEST_CASE("invalid view input reports structured details and leaves the current input retryable", "[execution-view][input][debug]")
    {
        const auto [library, ids] = selection_library();
        givm::table table;
        givm::executor execution;
        REQUIRE(execution.start(library, table).resume(library, table, givm_test::zero_random)
            == givm::execution_state::card_selection);
        const auto input = execution.view_in<givm::execution_state::card_selection>();
        const std::bitset<givm::selection_capacity> invalid{ 1 };
        try
        {
            (void)input.select(library, table, givm_test::zero_random, invalid);
            FAIL("selecting a nonexistent hand card must be rejected");
        }
        catch(const givm::view_input_error<givm::invalid_card_positions>& error)
        {
            CHECK(error.operation == "card_selection.select");
            CHECK(error.reason.selected == invalid);
            CHECK(error.reason.card_count == 0);
            CHECK(error_string(error) == error.what());
        }
        CHECK(input.player() == givm::player_id{ 0 });
        REQUIRE(input.select(library, table, givm_test::zero_random, {}) == givm::execution_state::card_selection);
    }

    TEST_CASE("start copy assignment and moves invalidate previously acquired views", "[execution-view][lifecycle][debug]")
    {
        const auto [library, ids] = selection_library();
        givm::table table;
        givm::executor source;
        source.start(library, table);
        SECTION("restart")
        {
            const auto stale = source.view_in<givm::execution_state::initialized>();
            source.start(library, table);
            REQUIRE_THROWS_AS(stale.resume(library, table, givm_test::zero_random), givm::execution_view_error);
        }
        SECTION("copy assignment")
        {
            givm::executor target;
            const auto stale = target.start(library, table);
            target = source;
            REQUIRE_THROWS_AS(stale.resume(library, table, givm_test::zero_random), givm::execution_view_error);
            REQUIRE(target.view_in<givm::execution_state::initialized>().resume(library, table, givm_test::zero_random)
                == givm::execution_state::card_selection);
            REQUIRE(source.view_in<givm::execution_state::initialized>().resume(library, table, givm_test::zero_random)
                == givm::execution_state::card_selection);
        }
        SECTION("move construction")
        {
            const auto stale = source.view_in<givm::execution_state::initialized>();
            givm::executor moved{ std::move(source) };
            REQUIRE_THROWS_AS(stale.resume(library, table, givm_test::zero_random), givm::execution_view_error);
            REQUIRE(moved.view_in<givm::execution_state::initialized>().resume(library, table, givm_test::zero_random)
                == givm::execution_state::card_selection);
        }
        SECTION("move assignment")
        {
            givm::executor target;
            const auto old_target = target.start(library, table);
            const auto old_source = source.view_in<givm::execution_state::initialized>();
            target = std::move(source);
            REQUIRE_THROWS_AS(old_target.resume(library, table, givm_test::zero_random), givm::execution_view_error);
            REQUIRE_THROWS_AS(old_source.resume(library, table, givm_test::zero_random), givm::execution_view_error);
            REQUIRE(target.view_in<givm::execution_state::initialized>().resume(library, table, givm_test::zero_random)
                == givm::execution_state::card_selection);
        }
    }
#endif
}
