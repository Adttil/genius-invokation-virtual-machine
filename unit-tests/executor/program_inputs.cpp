#include <array>
#include <concepts>
#include <cstdint>
#include <optional>
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
            bool runtime_inputs;
        };
        input_log* log;
        bool runtime_commands;

        std::string_view name() const noexcept { return "InputSource"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto commands = std::tuple{
                givm::set_active_character{},
                givm::replace_cards{ .player = givm::player_id{ 0 } },
                givm::set_active_character{},
                givm::draw_cards{ .count = 1 },
                givm::set_active_character{}
            };
            const auto main = runtime_commands
                ? context.add_program(std::vector<givm::any_command>{
                    givm::set_active_character{},
                    givm::replace_cards{ .player = givm::player_id{ 0 } },
                    givm::set_active_character{},
                    givm::draw_cards{ .count = 1 },
                    givm::set_active_character{}
                })
                : context.add_program(commands);
            return { log, main, context.add_program(std::tuple{ givm::set_active_character{} }), runtime_commands };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&, givm::test_event&,
                           givm::handle_context& context)
        {
            const givm::set_active_character_input first{ .current = { givm::player_id{ 0 }, 1 } };
            const givm::set_active_character_input second{ .current = { givm::player_id{ 0 }, 2 } };
            const givm::set_active_character_input third{ .current = { givm::player_id{ 0 }, 0 } };
            if(data.runtime_inputs)
            {
                const std::array<givm::any_command_input, 3> inputs{ first, second, third };
                return context.invoke(data.main, std::span<const givm::any_command_input>{ inputs });
            }
            return context.invoke(data.main, first, second, third);
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&, givm::active_character_changed& event,
                           givm::handle_context& context)
        {
            CHECK(context.table()[event.current.player_id].state().active_character == event.current);
            data.log->active.push_back(event.current);
            if(event.current.index == 1)
                return context.invoke(data.nested, givm::set_active_character_input{ .current = { givm::player_id{ 0 }, 2 } });
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
                givm::set_active_character{},
                givm::replace_cards{ .player = givm::player_id{ 0 } },
                givm::set_active_character{}
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
            const givm::set_active_character_input first{ .current = self.id().index == 0 ? event.target : self.id() };
            const givm::set_active_character_input second{ .current = previous };
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
    const auto source = givm::test::with_passive_skill(input_source{ &log, runtime_commands });
    const givm::test::initialized_character_source plain;
    const givm::test::named_definition_source<givm::card_definition> card{ "InputCard" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::test_command{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, source, plain, card);
    givm::table table{ { .self_player = givm::player_id{ 0 } } };
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
        CHECK(log.active == std::vector{ first, second, third });
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
    const auto source = givm::test::with_passive_skill(cached_input_source{ &log });
    const givm::test::initialized_character_source plain;
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::begin_action{} }, std::tuple{}, source, plain);
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
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

namespace
{
    struct array_input_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            givm::program_entry entry;
            std::size_t count;
            bool runtime_inputs;
        };
        std::size_t count;
        bool runtime_inputs;

        std::string_view name() const noexcept { return "ArrayInputSource"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.add_program(std::tuple{
                givm::set_energy{}, givm::deal_damage{},
                givm::replace_cards{ .player = givm::player_id{ 0 } },
                givm::deal_damage{}, givm::set_energy{}
            }), count, runtime_inputs };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .max_energy = 3, .health = 10 };
        }
        template<class Event>
        requires (std::same_as<Event, givm::test_event> or std::same_as<Event, givm::cost_of_switch>)
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            Event& event, givm::handle_context& context)
        {
            if constexpr(std::same_as<Event, givm::cost_of_switch>)
                event.requirement.dice_requirement.any = 0;
            const givm::character_id target{ givm::player_id{ 1 }, 0 };
            // Both ranges are local: invoke must retain their contents through
            // the later selection pause and through deferred payment execution.
            const std::vector<givm::damage> first(data.count,
                { .source = self.id(), .target = target, .value = 1, .type = givm::damage_type::physical });
            const std::array second{
                givm::damage{ .source = self.id(), .target = target, .value = 2, .type = givm::damage_type::physical } };
            const givm::set_energy_input begin{ self.id(), 1 };
            const givm::set_energy_input end{ self.id(), 3 };
            const auto invoke = [&](const auto&... inputs)
            {
                if constexpr(std::same_as<Event, givm::cost_of_switch>)
                    return context.invoke(givm::substack_t{}, data.entry, inputs...);
                else
                    return context.invoke(data.entry, inputs...);
            };
            if(data.runtime_inputs)
            {
                const std::array<givm::any_command_input, 4> inputs{
                    begin, givm::deal_damage_input{ first }, givm::deal_damage_input{ second }, end };
                return invoke(std::span<const givm::any_command_input>{ inputs });
            }
            return invoke(begin, givm::deal_damage_input{ first }, givm::deal_damage_input{ second }, end);
        }
    };
}

TEST_CASE("dynamic damage array inputs retain their contents through cached and copied continuations", "[program-input][onpay][deal_damage]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto count = GENERATE(std::size_t{ 0 }, std::size_t{ 1 }, std::size_t{ 4 });
    const bool runtime_inputs = GENERATE(false, true);
    const bool cached = GENERATE(false, true);
    CAPTURE(mode, count, runtime_inputs, cached);
    const auto source = givm::test::with_passive_skill(array_input_source{ count, runtime_inputs });
    const givm::test::initialized_character_source plain;
    const auto program = cached ? std::vector<givm::any_command>{ givm::begin_action{} }
        : std::vector<givm::any_command>{ givm::test_command{}, givm::end_game{ givm::game_result::both_loss } };
    const auto [compiled_library, ids] = givm::test::compile_definitions_with_program(mode, program, std::tuple{}, source, plain);
    const givm::definition_library library = compiled_library;
    const givm::character_id owner{ givm::player_id{ 0 }, 0 };
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    givm::table table{ { .self_player = owner.player_id }, { .active_character = owner }, { .active_character = target } };
    const auto plain_id = ids.get_id<givm::character_view>(plain.name());
    load_deck(table, library,
        { .characters = { ids.get_id<givm::character_view>(source.name()), plain_id } }, { .characters = { plain_id } });
    givm::executor execution;
    execution.enter_entry(library);
    zero_random random;
    const auto advance_array = [&](givm::executor& current, givm::table& current_table)
    {
        auto state = current.step(library, current_table, random);
        while(state == givm::execution_state::health_reduced
            || state == givm::execution_state::active_character_changed
            || state == givm::execution_state::action_started)
            state = current.step(library, current_table, random);
        return state;
    };
    if(cached)
    {
        REQUIRE(advance_array(execution, table) == givm::execution_state::action_selection);
        const auto action = execution.view_in<givm::execution_state::action_selection>();
        REQUIRE(action.switch_target_count() == 1);
        action.calculate_switch_cost(library, table, 0);
        CHECK(table[target].state().health == 10);
        CHECK(table[owner].state().energy == 0);
        action.switch_active_character(0, {});
    }
    REQUIRE(advance_array(execution, table) == givm::execution_state::card_selection);
    CHECK(table[target].state().health == 10 - count);
    CHECK(table[owner].state().energy == 1);
    auto copied_execution = execution;
    auto copied_table = table;
    const auto finish = [&](givm::executor& current, givm::table& current_table)
    {
        current.view_in<givm::execution_state::card_selection>().select({});
        REQUIRE(advance_array(current, current_table)
            == (cached ? givm::execution_state::action_selection : givm::execution_state::finished));
        CHECK(current_table[target].state().health == 8 - count);
        CHECK(current_table[owner].state().energy == 3);
    };
    finish(execution, table);
    finish(copied_execution, copied_table);
}

#ifndef NDEBUG
namespace
{
    enum class input_mismatch
    {
        typed_missing,
        typed_extra,
        runtime_missing,
        runtime_extra,
        typed_wrong_type,
        runtime_wrong_type,
        typed_wrong_order,
        runtime_wrong_order
    };

    struct mismatched_input_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            input_mismatch error;
            givm::program_entry entry;
        };
        input_mismatch error;

        std::string_view name() const noexcept { return "MismatchedInput"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            givm::program_entry entry;
            switch(error)
            {
            case input_mismatch::typed_missing:
            case input_mismatch::runtime_missing:
                entry = context.add_program(std::tuple{ givm::set_active_character{} });
                break;
            case input_mismatch::typed_wrong_type:
            case input_mismatch::runtime_wrong_type:
                entry = context.add_program(std::tuple{ givm::remove_summon{} });
                break;
            case input_mismatch::typed_wrong_order:
            case input_mismatch::runtime_wrong_order:
                entry = context.add_program(std::tuple{ givm::set_active_character{}, givm::remove_summon{} });
                break;
            default:
                entry = context.add_program(std::tuple{
                    givm::set_active_character{ givm::relative_character_target{ givm::relative_player::self, 0 } } });
            }
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
            const givm::set_active_character_input active{ self.id() };
            const givm::remove_summon_input summon{ { self.id().player_id, 0 } };
            const givm::remove_support_input support{ { self.id().player_id, 0 } };
            static_assert(sizeof(summon) == sizeof(support));
            switch(data.error)
            {
            case input_mismatch::typed_missing:
                return invoke();
            case input_mismatch::typed_extra:
                return invoke(active);
            case input_mismatch::runtime_missing:
                return invoke(std::span<const givm::any_command_input>{});
            case input_mismatch::runtime_extra:
            {
                const std::array<givm::any_command_input, 1> inputs{ active };
                return invoke(std::span<const givm::any_command_input>{ inputs });
            }
            case input_mismatch::typed_wrong_type:
                return invoke(support);
            case input_mismatch::runtime_wrong_type:
            {
                const std::array<givm::any_command_input, 1> inputs{ support };
                return invoke(std::span<const givm::any_command_input>{ inputs });
            }
            case input_mismatch::typed_wrong_order:
                return invoke(summon, active);
            case input_mismatch::runtime_wrong_order:
            {
                const std::array<givm::any_command_input, 2> inputs{ summon, active };
                return invoke(std::span<const givm::any_command_input>{ inputs });
            }
            }
            return {};
        }
    };
}

TEST_CASE("debug invocation checks nominal input types count and order before execution or caching", "[program-input][onpay][debug]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool cached = GENERATE(false, true);
    const auto error = GENERATE(input_mismatch::typed_missing, input_mismatch::typed_extra,
                               input_mismatch::runtime_missing, input_mismatch::runtime_extra,
                               input_mismatch::typed_wrong_type, input_mismatch::runtime_wrong_type,
                               input_mismatch::typed_wrong_order, input_mismatch::runtime_wrong_order);
    CAPTURE(mode, cached, error);
    const auto source = givm::test::with_passive_skill(mismatched_input_source{ error });
    const givm::test::initialized_character_source plain;
    const auto program = cached
        ? std::vector<givm::any_command>{ givm::begin_action{} }
        : std::vector<givm::any_command>{ givm::test_command{}, givm::end_game{ givm::game_result::both_loss } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode, program, std::tuple{}, source, plain);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = cached ? std::optional{ givm::character_id{ givm::player_id{ 0 }, 0 } } : std::nullopt },
        { .active_character = cached ? std::optional{ givm::character_id{ givm::player_id{ 1 }, 0 } } : std::nullopt } };
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
