#include <concepts>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/givm.hpp>

#include "../table/test_definition_library.hpp"
#include "test_character_source.hpp"

namespace
{
    struct input_log
    {
        std::vector<givm::character_id> active;
        std::vector<givm::character_id> quote_active;
    };

    struct input_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            input_log* log;
            givm::program_entry main;
            givm::program_entry nested;
        };
        input_log* log;
        bool runtime_commands;

        std::string_view name() const noexcept { return "InputSource"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto commands = std::tuple{
                givm::set_active_character_from_input{},
                givm::replace_cards{ .player = givm::player_id{ 0 } },
                givm::set_active_character_from_input{},
                givm::draw_cards{ .count = 1 },
                givm::set_active_character_from_input{}
            };
            const auto main = runtime_commands
                ? context.add_program(std::vector<givm::any_command>{
                    givm::set_active_character_from_input{},
                    givm::replace_cards{ .player = givm::player_id{ 0 } },
                    givm::set_active_character_from_input{},
                    givm::draw_cards{ .count = 1 },
                    givm::set_active_character_from_input{}
                })
                : context.add_program(commands);
            return { log, main, context.add_program(std::tuple{ givm::set_active_character_from_input{} }) };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&, givm::test_event&,
                           givm::handle_context& context)
        {
            const givm::active_character_changed first{ .current = { givm::player_id{ 0 }, 1 } };
            const givm::active_character_changed second{ .current = { givm::player_id{ 0 }, 2 } };
            const givm::active_character_changed third{ .current = { givm::player_id{ 0 }, 0 } };
            return context.invoke(data.main, first, second, third);
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&, givm::active_character_changed& event,
                           givm::handle_context& context)
        {
            CHECK(context.table()[event.current.player_id].state().active_character == event.current);
            data.log->active.push_back(event.current);
            if(event.current.index == 1)
                return context.invoke(data.nested, givm::active_character_changed{ .current = { givm::player_id{ 0 }, 2 } });
            return {};
        }
    };

    struct cached_input_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            input_log* log;
            givm::program_entry payment;
        };
        input_log* log;

        std::string_view name() const noexcept { return "CachedInputSource"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_program(std::vector<givm::any_command>{
                givm::set_active_character_from_input{},
                givm::replace_cards{ .player = givm::player_id{ 0 } },
                givm::set_active_character_from_input{}
            }) };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self, givm::cost_of_switch& event,
                           givm::handle_context& context)
        {
            const auto previous = *context.table()[self.player().id()].state().active_character;
            data.log->quote_active.push_back(previous);
            event.requirement.dice_requirement.any = 0;
            const givm::active_character_changed first{ .current = self.id().index == 0 ? event.target : self.id() };
            const givm::active_character_changed second{ .current = previous };
            return context.invoke(givm::substack_t{}, data.payment, first, second);
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self, givm::active_character_changed& event,
                           givm::handle_context&)
        {
            if(self.id().index == 0) data.log->active.push_back(event.current);
            return {};
        }
    };

    struct zero_random { std::uint32_t operator()() const noexcept { return 0; } };

    givm::execution_state advance(givm::executor& execution, const givm::definition_library& library,
                                  givm::table& table, zero_random& random)
    {
        auto state = execution.step(library, table, random);
        while(state == givm::execution_state::active_character_changed or state == givm::execution_state::action_started)
            state = execution.step(library, table, random);
        return state;
    }
}

TEST_CASE("program inputs retain order across nested responses and copied input pauses", "[program-input][broadcast][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool runtime_commands = GENERATE(false, true);
    input_log log;
    const input_source source{ &log, runtime_commands };
    const givm::test::initialized_character_source plain;
    const givm::test::named_definition_source<givm::card_definition> card{ "InputCard" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::test_command{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, source, plain, card);
    givm::table table;
    const auto plain_id = ids.get_id<givm::character_view>(plain.name());
    load_deck(table, library, {
        .cards = { ids.get_id<givm::card_definition>(card.name()) },
        .characters = { ids.get_id<givm::character_view>(source.name()), plain_id, plain_id }
    }, {});
    givm::executor execution;
    execution.enter_entry(library);
    zero_random random;
    REQUIRE(advance(execution, library, table, random) == givm::execution_state::card_selection);
    const givm::character_id first{ givm::player_id{ 0 }, 1 };
    const givm::character_id second{ givm::player_id{ 0 }, 2 };
    const givm::character_id third{ givm::player_id{ 0 }, 0 };
    CHECK(log.active == std::vector{ first, second });
    CHECK(table[givm::player_id{ 0 }].state().active_character == second);
    auto copied_execution = execution;
    auto copied_table = table;
    const auto prefix = log.active;
    const auto finish = [&](givm::executor& current, givm::table& current_table)
    {
        log.active = prefix;
        current.view_in<givm::execution_state::card_selection>().select({});
        REQUIRE(advance(current, library, current_table, random) == givm::execution_state::finished);
        CHECK(log.active == std::vector{ first, second, second, third });
        CHECK(current_table[givm::player_id{ 0 }].state().active_character == third);
        CHECK(current_table[givm::player_id{ 0 }].hand_card_count() == 1);
        CHECK(current_table[givm::player_id{ 0 }].deck_card_count() == 0);
    };
    finish(execution, table);
    finish(copied_execution, copied_table);
}

TEST_CASE("cached payment inputs preserve quotation snapshots and candidate order after copies", "[program-input][onpay][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    input_log log;
    const cached_input_source source{ &log };
    const givm::test::initialized_character_source plain;
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{
            givm::set_active_character{ { givm::player_id{ 0 }, 0 } },
            givm::set_active_character{ { givm::player_id{ 1 }, 0 } }, givm::begin_action{}
        }, std::tuple{}, source, plain);
    givm::table table;
    const auto source_id = ids.get_id<givm::character_view>(source.name());
    const auto plain_id = ids.get_id<givm::character_view>(plain.name());
    load_deck(table, library, { .characters = { source_id, source_id, plain_id } }, { .characters = { plain_id } });
    givm::executor execution;
    execution.enter_entry(library);
    zero_random random;
    REQUIRE(advance(execution, library, table, random) == givm::execution_state::action_selection);
    log.active.clear();
    const auto action = execution.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.switch_target_count() == 2);
    CHECK(action.calculate_switch_cost(library, table, 1).requirement.dice_requirement.any == 0);
    CHECK(action.calculate_switch_cost(library, table, 0).requirement.dice_requirement.any == 0);
    CHECK(action.switch_cost(1).requirement.dice_requirement.any == 0);
    CHECK(action.switch_cost(0).requirement.dice_requirement.any == 0);
    const givm::character_id original{ givm::player_id{ 0 }, 0 };
    const givm::character_id other_responder{ givm::player_id{ 0 }, 1 };
    CHECK(log.quote_active == std::vector(4, original));
    CHECK(log.active.empty());
    CHECK(table[givm::player_id{ 0 }].state().active_character == original);
    for(std::size_t candidate = 0; candidate < 2; ++candidate)
    {
        auto branch = execution;
        auto branch_table = table;
        const auto selected = action.switch_target(candidate);
        log.active.clear();
        branch.view_in<givm::execution_state::action_selection>().switch_active_character(candidate, {});
        REQUIRE(advance(branch, library, branch_table, random) == givm::execution_state::card_selection);
        CHECK(log.active == std::vector{ selected });
        auto paused_copy = branch;
        auto paused_table = branch_table;
        const auto finish = [&](givm::executor& current, givm::table& current_table)
        {
            log.active = { selected };
            current.view_in<givm::execution_state::card_selection>().select({});
            REQUIRE(advance(current, library, current_table, random) == givm::execution_state::card_selection);
            CHECK(log.active == std::vector{ selected, original, other_responder });
            current.view_in<givm::execution_state::card_selection>().select({});
            REQUIRE(advance(current, library, current_table, random) == givm::execution_state::action_selection);
            CHECK(log.active == std::vector{ selected, original, other_responder, original, selected });
            CHECK(current_table[givm::player_id{ 0 }].state().active_character == selected);
            CHECK(log.quote_active == std::vector(4, original));
        };
        finish(branch, branch_table);
        finish(paused_copy, paused_table);
    }
}

#ifndef NDEBUG
namespace
{
    enum class input_size_mismatch
    {
        typed_missing,
        typed_extra,
        raw_missing,
        raw_extra
    };

    struct mismatched_input_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            input_size_mismatch error;
            givm::program_entry entry;
        };
        input_size_mismatch error;

        std::string_view name() const noexcept { return "MismatchedInputSize"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto entry = error == input_size_mismatch::typed_missing or error == input_size_mismatch::raw_missing
                ? context.add_program(std::tuple{ givm::set_active_character_from_input{} })
                : context.add_program(std::tuple{ givm::set_active_character{ { givm::player_id{ 0 }, 0 } } });
            return { error, entry };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        template<class TEvent>
        requires (std::same_as<TEvent, givm::test_event> or std::same_as<TEvent, givm::cost_of_switch>)
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
                           TEvent&, givm::handle_context& context)
        {
            const auto invoke = [&](auto... inputs)
            {
                if constexpr(std::same_as<TEvent, givm::cost_of_switch>)
                    return context.invoke(givm::substack_t{}, data.entry, inputs...);
                else
                    return context.invoke(data.entry, inputs...);
            };
            const unsigned char unexpected_byte = 0;
            switch(data.error)
            {
            case input_size_mismatch::typed_missing:
                return invoke();
            case input_size_mismatch::typed_extra:
                return invoke(givm::active_character_changed{ .current = self.id() });
            case input_size_mismatch::raw_missing:
                return invoke(std::span<const unsigned char>{});
            case input_size_mismatch::raw_extra:
                return invoke(std::span<const unsigned char>{ &unexpected_byte, 1 });
            }
            return {};
        }
    };
}

TEST_CASE("debug invocation checks input length before execution or caching", "[program-input][onpay][debug]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool cached = GENERATE(false, true);
    const auto error = GENERATE(input_size_mismatch::typed_missing, input_size_mismatch::typed_extra,
                               input_size_mismatch::raw_missing, input_size_mismatch::raw_extra);
    CAPTURE(mode, cached, error);
    const mismatched_input_source source{ error };
    const givm::test::initialized_character_source plain;
    const auto program = cached
        ? std::vector<givm::any_command>{
            givm::set_active_character{ { givm::player_id{ 0 }, 0 } },
            givm::set_active_character{ { givm::player_id{ 1 }, 0 } }, givm::begin_action{}
        }
        : std::vector<givm::any_command>{ givm::test_command{}, givm::end_game{ givm::game_result::both_loss } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode, program, std::tuple{}, source, plain);
    givm::table table;
    const auto plain_id = ids.get_id<givm::character_view>(plain.name());
    load_deck(table, library, {
        .characters = { ids.get_id<givm::character_view>(source.name()), plain_id }
    }, { .characters = { plain_id } });
    givm::executor execution;
    execution.enter_entry(library);
    zero_random random;
    if(cached)
    {
        REQUIRE(advance(execution, library, table, random) == givm::execution_state::action_selection);
        const auto action = execution.view_in<givm::execution_state::action_selection>();
        REQUIRE(action.switch_target_count() == 1);
        REQUIRE_THROWS_AS(action.calculate_switch_cost(library, table, 0), std::invalid_argument);
        CHECK(table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 0 });
    }
    else
    {
        REQUIRE_FALSE(table[givm::player_id{ 0 }].state().active_character);
        REQUIRE_THROWS_AS(execution.step(library, table, random), std::invalid_argument);
        CHECK_FALSE(table[givm::player_id{ 0 }].state().active_character);
    }
}
#endif
