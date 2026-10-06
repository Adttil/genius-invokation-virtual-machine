#include <optional>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../table/test_definition_library.hpp"
#include "test_character_source.hpp"

namespace givm_test::executor::settlement_payment
{
namespace
{
    constexpr givm::character_id first{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id second{ givm::player_id{ 0 }, 1 };
    constexpr std::size_t target_count = 257;

    enum class input_error { none, response_index, nested_type };

    struct payment_log
    {
        std::vector<std::uint32_t> quote_indices;
        std::vector<givm::character_id> active;
        std::vector<std::array<std::uint32_t, 2>> energy;
    };

    struct payment_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            payment_log* log;
            input_error error;
            givm::program_entry payment;
            givm::program_entry relay;
            givm::program_entry leaf;
        };

        payment_log* log;
        input_error error = input_error::none;

        std::string_view name() const { return "DeferredPayment"; }

        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto leaf = context.add_program(
                givm::replace_cards{ .player = givm::player_id{ 0 } },
                givm::modify_energy{}, givm::set_active_character{}, givm::return_response{});
            const auto relay = context.add_program(givm::defer_program{}, givm::return_response{});
            return { log, error, context.add_program(givm::defer_program{}, givm::return_response{}), relay, leaf };
        }

        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .max_energy = 10000, .health = 10 };
        }

        static givm::program_entry handle(const definition_type& data, givm::cost_of_switch& event,
            givm::handle_context<givm::skill_view>& context, std::uint32_t index = 0)
        {
            data.log->quote_indices.push_back(index);
            if(index != 0) return {};
            event.requirement.dice_requirement.any = 0;
            const auto self = context.entity().character().id();
            const auto original = *context.table()[self.player_id].state().active_character;
            const std::vector<givm::character_id> targets(target_count, self);
            const auto target = givm::set_active_character_input{ .current = self == first ? event.target : original };
            const auto leaf = data.error == input_error::nested_type
                ? givm::defer_invoke(data.leaf, givm::set_energy_input{ self, 7 }, target, givm::return_response_input{ 23 })
                : givm::defer_invoke(data.leaf, givm::modify_energy_input{ targets, 1 }, target, givm::return_response_input{ 23 });
            return context.invoke(givm::substack_t{}, data.payment,
                givm::defer_invoke(data.relay, leaf, givm::return_response_input{ 19 }),
                givm::return_response_input{
                    data.error == input_error::response_index ? givm::return_response::dynamic : 17u });
        }

        static givm::program_entry handle(const definition_type& data, givm::active_character_changed& event,
            givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            if(context.entity().character().id() != first) return {};
            data.log->active.push_back(event.current);
            data.log->energy.push_back({ context.table()[first].state().energy, context.table()[second].state().energy });
            (void)context.random();
            return {};
        }
    };

    struct counting_random
    {
        std::size_t calls = 0;
        std::uint32_t operator()() noexcept { ++calls; return 0; }
    };

    givm::execution_state finish_observation(givm::executor& execution, const givm::definition_library& library,
        givm::table& table, counting_random& random, givm::execution_state state)
    {
        for(;;)
        {
            if(state == givm::execution_state::active_character_changed)
                state = execution.view_in<givm::execution_state::active_character_changed>().resume(library, table, random);
            else if(state == givm::execution_state::action_started)
                state = execution.view_in<givm::execution_state::action_started>().resume(library, table, random);
            else return state;
        }
    }

    givm::table make_table(const givm::definition_library& library, const givm::issued_id_map& ids)
    {
        givm::table result{ { .self_player = givm::player_id{ 0 } }, { .active_character = first },
            { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
        const auto source = ids.get_id<givm::character_view>("DeferredPayment");
        load_deck(result, library, { .characters = { source, source } },
            { .characters = { ids.get_id<givm::character_view>("Character") } });
        return result;
    }
}

TEST_CASE("payment previews cache nested deferred inputs once and settle each responder before the next", "[settlement][onpay]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    payment_log log;
    const auto source = givm::test::with_passive_skill(payment_source{ &log });
    const givm::test::initialized_character_source plain;
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::begin_action{} }, std::tuple{}, source, plain);
    auto table = make_table(library, ids);
    givm::executor execution;
    counting_random random;
    REQUIRE(finish_observation(execution, library, table, random, execution.start(library, table).resume(library, table, random))
        == givm::execution_state::action_selection);
    REQUIRE(random.calls == 0);
    const auto action = execution.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.switch_target_count() == 1);
    const auto quote_1 = action.calculate_switch_cost(library, table, 0);
    CHECK(action.switch_cost(quote_1).requirement.dice_requirement.any == 0);
    CHECK(action.switch_cost(quote_1).requirement.dice_requirement.any == 0);
    CHECK(log.quote_indices == std::vector<std::uint32_t>{ 0, 0 });
    CHECK(log.active.empty());
    CHECK(log.energy.empty());
    CHECK(random.calls == 0);
    CHECK(table[first].state().energy == 0);
    CHECK(table[second].state().energy == 0);
    CHECK(table[givm::player_id{ 0 }].state().active_character == first);

    REQUIRE(finish_observation(execution, library, table, random,
        action.switch_active_character_with_cached_cost(library, table, random, quote_1, {}))
        == givm::execution_state::card_selection);
    CHECK(log.active.empty());
    CHECK(random.calls == 0);
    auto copied = execution;
    auto copied_table = table;

    const auto finish = [&](givm::executor& current, givm::table& current_table)
    {
        counting_random branch_random;
        log.active.clear();
        log.energy.clear();
        REQUIRE(finish_observation(current, library, current_table, branch_random,
            current.view_in<givm::execution_state::card_selection>().select(library, current_table, branch_random, {}))
            == givm::execution_state::card_selection);
        CHECK(log.active == std::vector{ second });
        CHECK(log.energy == std::vector{ std::array<std::uint32_t, 2>{ target_count, 0 } });
        CHECK(current_table[first].state().energy == target_count);
        CHECK(current_table[second].state().energy == 0);
        CHECK(branch_random.calls == 1);

        REQUIRE(finish_observation(current, library, current_table, branch_random,
            current.view_in<givm::execution_state::card_selection>().select(library, current_table, branch_random, {}))
            == givm::execution_state::action_selection);
        CHECK(log.active == std::vector{ second, first, second });
        CHECK(log.energy == std::vector{
            std::array<std::uint32_t, 2>{ target_count, 0 },
            std::array<std::uint32_t, 2>{ target_count, target_count },
            std::array<std::uint32_t, 2>{ target_count, target_count } });
        CHECK(current_table[first].state().energy == target_count);
        CHECK(current_table[second].state().energy == target_count);
        CHECK(current_table[givm::player_id{ 0 }].state().active_character == second);
        CHECK(branch_random.calls == 3);
        CHECK(log.quote_indices == std::vector<std::uint32_t>{ 0, 0 });
    };
    finish(execution, table);
    finish(copied, copied_table);
}

#ifndef NDEBUG
TEST_CASE("payment input validation rejects a dynamic return marker and a nested deferred type mismatch", "[settlement][onpay][debug]")
{
    std::optional<givm::switch_cost_id> quote_1;
    const auto error = GENERATE(input_error::response_index, input_error::nested_type);
    payment_log log;
    const auto source = givm::test::with_passive_skill(payment_source{ &log, error });
    const givm::test::initialized_character_source plain;
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::begin_action{} }, std::tuple{}, source, plain);
    auto table = make_table(library, ids);
    givm::executor execution;
    counting_random random;
    REQUIRE(execution.start(library, table).resume(library, table, random) == givm::execution_state::action_selection);
    try
    {
        quote_1 = execution.view_in<givm::execution_state::action_selection>().calculate_switch_cost(library, table, 0);
        FAIL("invalid cached inputs must be rejected during quotation");
    }
    catch(const givm::program_input_error& exception)
    {
        if(error == input_error::response_index)
        {
            const auto* reason = std::get_if<givm::invalid_response_index>(&exception.reason);
            REQUIRE(reason);
            CHECK(reason->index == givm::return_response::dynamic);
        }
        else
        {
            const auto* reason = std::get_if<givm::program_input_type_mismatch>(&exception.reason);
            REQUIRE(reason);
            CHECK(reason->input_index == 0);
            CHECK(reason->command == "modify_energy");
            CHECK(reason->expected == "modify_energy_input");
            CHECK(reason->actual == "set_energy_input");
        }
    }
    CHECK(log.quote_indices == std::vector<std::uint32_t>{ 0 });
    CHECK(log.active.empty());
    CHECK(random.calls == 0);
    CHECK(table[first].state().energy == 0);
    CHECK(table[second].state().energy == 0);
    CHECK(table[givm::player_id{ 0 }].state().active_character == first);
}
#endif
}
