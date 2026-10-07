#include <cstddef>
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

namespace givm_test::executor::program_inputs
{
constexpr std::array<std::size_t, 1> draw_positions_1{ 0 };

namespace
{
    struct input_log
    {
        std::vector<givm::character_id> active;
        std::vector<givm::character_id> quote_active;
    };

    struct input_source
    {
        static constexpr auto category = givm::definition_category::character;
        struct definition_type
        {
            input_log* log;
            givm::normal_effect main;
            givm::normal_effect nested;
            bool runtime_inputs;
        };
        input_log* log;
        bool runtime_commands;

        std::string_view name() const noexcept { return "InputSource"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto commands = std::tuple{
                givm::set_active_character{}, givm::settle{},
                givm::replace_cards{ .player = givm::player_id{ 0 } },
                givm::set_active_character{},
                givm::draw_cards{ .position = 0, .count = 1 },
                givm::set_active_character{}
            };
            const auto main = runtime_commands
                ? context.add_normal_effect(std::vector<givm::any_command>{
                    givm::set_active_character{}, givm::settle{},
                    givm::replace_cards{ .player = givm::player_id{ 0 } },
                    givm::set_active_character{},
                    givm::draw_cards{ .position = 0, .count = 1 },
                    givm::set_active_character{}
                })
                : context.add_normal_effect(commands);
            return { log, main, context.add_normal_effect(std::tuple{ givm::set_active_character{} }), runtime_commands };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::normal_effect handle(const definition_type& data, givm::round_started&,
                           givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            const givm::set_active_character_input first{ .current = { givm::player_id{ 0 }, 1 } };
            const givm::set_active_character_input second{ .current = { givm::player_id{ 0 }, 2 } };
            const givm::set_active_character_input third{ .current = { givm::player_id{ 0 }, 0 } };
            if(data.runtime_inputs)
            {
                const auto inputs = givm::pack_inputs(first, second, third);
                return context.invoke(data.main, inputs);
            }
            return context.invoke(data.main, first, second, third);
        }
        static givm::normal_effect handle(const definition_type& data, givm::active_character_changed& event,
                           givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            CHECK(context.table()[event.current.player_id()].state().active_character == event.current);
            data.log->active.push_back(event.current);
            if(event.current.index() == 1)
                return context.invoke(data.nested, givm::set_active_character_input{ .current = { givm::player_id{ 0 }, 2 } });
            return {};
        }
    };

    struct cached_input_source
    {
        static constexpr auto category = givm::definition_category::character;
        struct definition_type
        {
            input_log* log;
            givm::preview_effect payment;
        };
        input_log* log;

        std::string_view name() const noexcept { return "CachedInputSource"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_preview_effect(std::vector<givm::any_command>{
                givm::set_active_character{}, givm::settle{},
                givm::replace_cards{ .player = givm::player_id{ 0 } },
                givm::set_active_character{}
            }) };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::preview_effect handle(const definition_type& data, givm::cost_of_switch& event,
                           givm::handle_context<givm::skill_view, givm::event_category::preview>& context)
        {
            const auto self = context.entity().character();
            const auto previous = *context.table()[self.player().id()].state().active_character;
            data.log->quote_active.push_back(previous);
            event.requirement.dice_requirement.any = 0;
            const givm::set_active_character_input first{ .current = self.id().index() == 0 ? event.target : self.id() };
            const givm::set_active_character_input second{ .current = previous };
            return context.invoke(data.payment, first, second);
        }
        static givm::normal_effect handle(const definition_type& data, givm::active_character_changed& event,
                           givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity().character();
            if(self.id().index() == 0) data.log->active.push_back(event.current);
            return {};
        }
    };

    struct zero_random { std::uint32_t operator()() const noexcept { return 0; } };

    givm::execution_state advance(givm_test::executor_driver& execution, const givm::definition_library& library,
                                  givm::table& table, zero_random& random)
    {
        auto state = execution.advance(library, table, random);
        while(state == givm::execution_state::active_character_changed or state == givm::execution_state::action_started)
            state = execution.advance(library, table, random);
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
    const givm::test::named_definition_source<givm::definition_category::card> card{ "InputCard" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, source, plain, card);
    givm::table table{ { .self_player = givm::player_id{ 0 } } };
    const auto plain_id = ids.get_id<givm::definition_category::character>(plain.name());
    load_deck(table, library, {
        .cards = { ids.get_id<givm::definition_category::card>(card.name()) },
        .characters = { ids.get_id<givm::definition_category::character>(source.name()), plain_id, plain_id }
    }, {});
    givm_test::executor_driver execution;
    execution.start(library, table);
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
    const auto finish = [&](givm_test::executor_driver& current, givm::table& current_table)
    {
        log.active = prefix;
        current.submitted(current.view_in<givm::execution_state::card_selection>().select(library, current_table, random, {}));
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
    const auto source_id = ids.get_id<givm::definition_category::character>(source.name());
    const auto plain_id = ids.get_id<givm::definition_category::character>(plain.name());
    load_deck(table, library, { .characters = { source_id, source_id, plain_id } }, { .characters = { plain_id } });
    givm_test::executor_driver execution;
    execution.start(library, table);
    zero_random random;
    REQUIRE(advance(execution, library, table, random) == givm::execution_state::action_selection);
    log.active.clear();
    const auto action = execution.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.switch_target_count() == 2);
    const auto quote_1 = action.calculate_switch_cost(library, table, 1);
    CHECK(action.switch_cost(quote_1).requirement.dice_requirement.any == 0);
    const auto quote_2 = action.calculate_switch_cost(library, table, 0);
    CHECK(action.switch_cost(quote_2).requirement.dice_requirement.any == 0);
    CHECK(action.switch_cost(quote_1).requirement.dice_requirement.any == 0);
    CHECK(action.switch_cost(quote_2).requirement.dice_requirement.any == 0);
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
        branch.submitted(branch.view_in<givm::execution_state::action_selection>().switch_active_character_with_cached_cost(library, branch_table, random, candidate == 0 ? quote_2 : quote_1, {}));
        REQUIRE(advance(branch, library, branch_table, random) == givm::execution_state::card_selection);
        CHECK(log.active == std::vector{ selected });
        auto paused_copy = branch;
        auto paused_table = branch_table;
        const auto finish = [&](givm_test::executor_driver& current, givm::table& current_table)
        {
            log.active = { selected };
            current.submitted(current.view_in<givm::execution_state::card_selection>().select(library, current_table, random, {}));
            REQUIRE(advance(current, library, current_table, random) == givm::execution_state::card_selection);
            CHECK(log.active == std::vector{ selected, original, other_responder });
            current.submitted(current.view_in<givm::execution_state::card_selection>().select(library, current_table, random, {}));
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
        static constexpr auto category = givm::definition_category::character;
        struct definition_type
        {
            givm::normal_effect entry;
            givm::preview_effect preview_entry;
            std::size_t count;
            bool runtime_inputs;
        };
        std::size_t count;
        bool runtime_inputs;

        std::string_view name() const noexcept { return "ArrayInputSource"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            std::vector<givm::any_command> commands{ givm::set_energy{} };
            for(std::size_t i = 0; i != count; ++i) commands.emplace_back(givm::deal_damage{});
            commands.emplace_back(givm::replace_cards{ .player = givm::player_id{ 0 } });
            commands.emplace_back(givm::deal_damage{});
            commands.emplace_back(givm::set_energy{});
            return { context.add_normal_effect(commands), context.add_preview_effect(commands), count, runtime_inputs };
        }

        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .max_energy = 3, .health = 10 };
        }
        template<class Event, class... TIndex>
        requires (std::same_as<Event, givm::round_started> or std::same_as<Event, givm::cost_of_switch>)
        static givm::effect<Event::category> handle(const definition_type& data,
            Event& event, givm::handle_context<givm::skill_view, Event::category>& context, TIndex...)
        {
            const auto entry = [&]
            {
                if constexpr(Event::category == givm::event_category::preview) return data.preview_entry;
                else return data.entry;
            }();
            const auto self = context.entity().character();
            if constexpr(std::same_as<Event, givm::cost_of_switch>)
                event.requirement.dice_requirement.any = 0;
            const givm::character_id target{ givm::player_id{ 1 }, 0 };
            // Both ranges are local: invoke must retain their contents through
            // the later selection pause and through deferred payment execution.
            const givm::set_energy_input begin{ self.id(), 1 };
            const givm::set_energy_input end{ self.id(), 3 };
            std::vector<givm::program_inputs> parts{ givm::pack_inputs(begin) };
            for(std::size_t i = 0; i != data.count; ++i)
                parts.push_back(givm::pack_inputs(givm::deal_damage_input{ std::array{ givm::damage{ .source = self.id(), .target = target,
                    .value = 1, .type = givm::damage_type::physical } } }));
            parts.push_back(givm::pack_inputs(givm::deal_damage_input{ std::array{ givm::damage{ .source = self.id(), .target = target,
                .value = 2, .type = givm::damage_type::physical } } }, end));
            const auto inputs = givm::concat_inputs(parts);
            if constexpr(std::same_as<Event, givm::cost_of_switch>)
                return context.invoke(entry, inputs);
            else return context.invoke(entry, inputs);

        }
    };
}

TEST_CASE("dynamic damage array inputs retain their contents through cached and copied continuations", "[program-input][onpay][deal_damage]")
{
    std::optional<givm::switch_cost_id> quote_1;
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto count = GENERATE(std::size_t{ 0 }, std::size_t{ 1 }, std::size_t{ 4 });
    const bool runtime_inputs = GENERATE(false, true);
    const bool cached = GENERATE(false, true);
    CAPTURE(mode, count, runtime_inputs, cached);
    const auto source = givm::test::with_passive_skill(array_input_source{ count, runtime_inputs });
    const givm::test::initialized_character_source plain;
    const auto program = cached ? std::vector<givm::any_command>{ givm::begin_action{} }
        : std::vector<givm::any_command>{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } };
    const auto [compiled_library, ids] = givm::test::compile_definitions_with_program(mode, program, std::tuple{}, source, plain);
    const givm::definition_library library = compiled_library;
    const givm::character_id owner{ givm::player_id{ 0 }, 0 };
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    givm::table table{ { .self_player = owner.player_id() }, { .active_character = owner }, { .active_character = target } };
    const auto plain_id = ids.get_id<givm::definition_category::character>(plain.name());
    load_deck(table, library,
        { .characters = { ids.get_id<givm::definition_category::character>(source.name()), plain_id } }, { .characters = { plain_id } });
    givm_test::executor_driver execution;
    execution.start(library, table);
    zero_random random;
    const auto advance_array = [&](givm_test::executor_driver& current, givm::table& current_table)
    {
        auto state = current.advance(library, current_table, random);
        while(state == givm::execution_state::health_reduced
            || state == givm::execution_state::active_character_changed
            || state == givm::execution_state::action_started)
            state = current.advance(library, current_table, random);
        return state;
    };
    if(cached)
    {
        REQUIRE(advance_array(execution, table) == givm::execution_state::action_selection);
        const auto action = execution.view_in<givm::execution_state::action_selection>();
        REQUIRE(action.switch_target_count() == 1);
        quote_1 = action.calculate_switch_cost(library, table, 0);
        CHECK(table[target].state().health == 10);
        CHECK(table[owner].state().energy == 0);
        execution.submitted(action.switch_active_character_with_cached_cost(library, table, random, *quote_1, {}));
    }
    REQUIRE(advance_array(execution, table) == givm::execution_state::card_selection);
    CHECK(table[target].state().health == 10 - count);
    CHECK(table[owner].state().energy == 1);
    auto copied_execution = execution;
    auto copied_table = table;
    const auto finish = [&](givm_test::executor_driver& current, givm::table& current_table)
    {
        current.submitted(current.view_in<givm::execution_state::card_selection>().select(library, current_table, random, {}));
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
        runtime_wrong_order,
        null_entry,
        repeated,
        foreign_entry
    };

    struct mismatched_input_source
    {
        static constexpr auto category = givm::definition_category::character;
        struct definition_type
        {
            input_mismatch error;
            givm::normal_effect entry;
            givm::preview_effect preview_entry;
            givm::normal_effect* foreign;
        };
        input_mismatch error;
        givm::normal_effect* exported = nullptr;
        givm::normal_effect* foreign = nullptr;

        std::string_view name() const noexcept { return "MismatchedInput"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            context.add_normal_effect(std::tuple{});
            std::vector<givm::any_command> commands;
            switch(error)
            {
            case input_mismatch::typed_missing:
            case input_mismatch::runtime_missing:
                commands = { givm::set_active_character{} };
                break;
            case input_mismatch::typed_wrong_type:
            case input_mismatch::runtime_wrong_type:
                commands = { givm::remove_summon{} };
                break;
            case input_mismatch::typed_wrong_order:
            case input_mismatch::runtime_wrong_order:
                commands = { givm::set_active_character{ { givm::relative_player::self, 0 } }, givm::set_active_character{}, givm::remove_summon{} };
                break;
            default:
                commands = {
                    givm::set_active_character{ givm::relative_character_target{ givm::relative_player::self, 0 } } };
            }
            const auto entry = context.add_normal_effect(commands);
            const auto preview_entry = context.add_preview_effect(commands);
            if(exported) *exported = entry;
            return { error, entry, preview_entry, foreign };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        template<class TEvent, class... TIndex>
        requires (std::same_as<TEvent, givm::round_started> or std::same_as<TEvent, givm::cost_of_switch>)
        static givm::effect<TEvent::category> handle(const definition_type& data,
                           TEvent&, givm::handle_context<givm::skill_view, TEvent::category>& context, TIndex...)
        {
            const auto self = context.entity().character();
            const auto invoke = [&](auto... inputs)
            {
                if constexpr(std::same_as<TEvent, givm::cost_of_switch>)
                    return context.invoke(data.preview_entry, inputs...);
                else
                    return context.invoke(data.foreign ? *data.foreign : data.entry, inputs...);
            };
            const givm::set_active_character_input active{ self.id() };
            const std::array summon_ids{ givm::summon_id{ self.id().player_id(), 0 } };
            const givm::remove_summon_input summon{ summon_ids };
            const givm::remove_support_input support{ { self.id().player_id(), 0 } };
            static_assert(sizeof(active) == sizeof(support));
            switch(data.error)
            {
            case input_mismatch::null_entry:
                return context.invoke(givm::effect<TEvent::category>{});
            case input_mismatch::repeated:
                invoke();
                return invoke();
            case input_mismatch::foreign_entry:
                return invoke();
            case input_mismatch::typed_missing:
                return invoke();
            case input_mismatch::typed_extra:
                return invoke(active);
            case input_mismatch::runtime_missing:
                return invoke(givm::pack_inputs());
            case input_mismatch::runtime_extra:
            {
                return invoke(givm::pack_inputs(active));
            }
            case input_mismatch::typed_wrong_type:
                return invoke(support);
            case input_mismatch::runtime_wrong_type:
            {
                return invoke(givm::pack_inputs(support));
            }
            case input_mismatch::typed_wrong_order:
                return invoke(summon, active);
            case input_mismatch::runtime_wrong_order:
            {
                const std::array inputs{ givm::pack_inputs(summon), givm::pack_inputs(active) };
                return invoke(givm::concat_inputs(inputs));
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
                               input_mismatch::typed_wrong_order, input_mismatch::runtime_wrong_order,
                               input_mismatch::null_entry, input_mismatch::repeated);
    CAPTURE(mode, cached, error);
    const auto source = givm::test::with_passive_skill(mismatched_input_source{ error });
    const givm::test::initialized_character_source plain;
    const auto program = cached
        ? std::vector<givm::any_command>{ givm::begin_action{} }
        : std::vector<givm::any_command>{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode, program, std::tuple{}, source, plain);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = cached ? givm::optional_character_id{ givm::character_id{ givm::player_id{ 0 }, 0 } } : nullptr },
        { .active_character = cached ? givm::optional_character_id{ givm::character_id{ givm::player_id{ 1 }, 0 } } : nullptr } };
    const auto plain_id = ids.get_id<givm::definition_category::character>(plain.name());
    load_deck(table, library, {
        .characters = { ids.get_id<givm::definition_category::character>(source.name()), plain_id }
    }, { .characters = { plain_id } });
    givm_test::executor_driver execution;
    execution.start(library, table);
    zero_random random;
    const auto verify_error = [&](auto&& operation)
    {
        try { operation(); FAIL("expected structured invocation error"); }
        catch(const std::exception& exception)
        {
            const auto* diagnostic = dynamic_cast<const givm::program_input_error*>(&exception);
            REQUIRE(diagnostic);
            CHECK(givm::error_string(*diagnostic) == exception.what());
            if(error == input_mismatch::null_entry)
            {
                CHECK(std::get<givm::invalid_effect>(diagnostic->reason) == givm::invalid_effect::null_entry);
                return;
            }
            REQUIRE(diagnostic->source);
            CHECK(diagnostic->source->name == std::string{ source.name() });
            CHECK(diagnostic->program_index == (cached ? 2 : 1));
            if(error == input_mismatch::repeated)
                CHECK(std::holds_alternative<givm::repeated_program_invocation>(diagnostic->reason));
            else if(const auto* reason = std::get_if<givm::program_input_count_mismatch>(&diagnostic->reason))
            {
                const bool missing = error == input_mismatch::typed_missing || error == input_mismatch::runtime_missing;
                CHECK(reason->expected == (missing ? 1 : 0));
                CHECK(reason->actual == (missing ? 0 : 1));
            }
            else
            {
                const auto& mismatch = std::get<givm::program_input_type_mismatch>(diagnostic->reason);
                const bool order = error == input_mismatch::typed_wrong_order || error == input_mismatch::runtime_wrong_order;
                CHECK(mismatch.input_index == 0);
                CHECK(mismatch.command_index == (order ? 1 : 0));
                CHECK(mismatch.expected == (order ? "set_active_character_input" : "remove_summon_input"));
                CHECK(mismatch.actual == (order ? "remove_summon_input" : "remove_support_input"));
            }
        }
    };
    if(cached)
    {
        REQUIRE(advance(execution, library, table, random) == givm::execution_state::action_selection);
        const auto action = execution.view_in<givm::execution_state::action_selection>();
        REQUIRE(action.switch_target_count() == 1);
        verify_error([&] { action.calculate_switch_cost(library, table, 0); });
        CHECK(table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 0 });
    }
    else
    {
        REQUIRE_FALSE(table[givm::player_id{ 0 }].state().active_character);
        verify_error([&] { execution.advance(library, table, random); });
        CHECK_FALSE(table[givm::player_id{ 0 }].state().active_character);
    }
}

TEST_CASE("debug entries reject foreign libraries and remain usable in library copies", "[program-input][debug]")
{
    givm::normal_effect exported;
    givm::normal_effect foreign;
    const auto source = givm::test::with_passive_skill(mismatched_input_source{ input_mismatch::foreign_entry, &exported, &foreign });
    const auto init = std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } };
    const auto [first, first_ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal, init, std::tuple{}, source);
    foreign = exported;
    const auto [second, second_ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal, init, std::tuple{}, source);
    const auto run = [&](const givm::definition_library& library, const givm::issued_id_map& ids)
    {
        const givm::character_id owner{ givm::player_id{0}, 0 };
        givm::table table{ { .self_player = owner.player_id() }, { .active_character = owner } };
        load_deck(table, library, { .characters = { ids.get_id<givm::definition_category::character>(source.name()) } }, {});
        givm_test::executor_driver execution;
        execution.start(library, table);
        zero_random random;
        return execution.advance(library, table, random);
    };
    try { run(second, second_ids); FAIL("expected foreign entry error"); }
    catch(const givm::program_input_error& error)
    {
        CHECK(std::get<givm::invalid_effect>(error.reason) == givm::invalid_effect::different_library);
    }
    const auto copied = first;
    CHECK(run(copied, first_ids) == givm::execution_state::finished);
}
#endif
}
