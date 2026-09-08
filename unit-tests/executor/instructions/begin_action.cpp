#include <algorithm>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/executor/instructions/begin_action.hpp>

#include "../../table/test_definition_library.hpp"

using namespace givm;

namespace
{
    struct payment_record
    {
        support_id handler;
        std::uint32_t reduction;
        std::uint32_t dice_total;

        friend bool operator==(const payment_record&, const payment_record&) = default;
    };

    using payment_log = std::vector<payment_record>;

    constexpr std::uint32_t reduction_total(
        const cost_effect_argument<cost_of_switch>& argument
    ) noexcept
    {
        return argument.reduced_dice.fixed.total()
            + argument.reduced_dice.same
            + argument.reduced_dice.any;
    }

    struct consume_fixed_switch_discount
    {
        using context_type = onpay_context<cost_of_switch>;

        payment_log* log;

        bool execute(card_table& table, execution_context& context, random_fn&) const
        {
            auto&& [onpay, activation] = context.stack().top<
                frame<
                    detail::handler_id<cost_of_switch>,
                    cost_effect_argument<cost_of_switch>,
                    stage_t
                >,
                frame<execution_context::return_info, stage_t>
            >();
            auto&& [current_handler, argument, onpay_stage] = onpay;
            auto&& [return_info, activation_stage] = activation;
            (void)onpay_stage;
            (void)return_info;
            REQUIRE(activation_stage == stage_t{});

            const auto* support = std::get_if<support_id>(&current_handler);
            REQUIRE(support != nullptr);
            const auto reduction = reduction_total(argument);
            REQUIRE(table[*support].state().count >= reduction);
            table[*support].state().count -= reduction;
            log->push_back({
                .handler = *support,
                .reduction = reduction,
                .dice_total = table[support->player_id].state().dice.total()
            });
            return context.enter_next();
        }
    };

    struct switch_tax_source
    {
        using definition_category = support_view;

        struct definition_type
        {
            std::uint8_t amount;
            std::uint32_t* action_phase_count;
        };

        std::string_view source_name;
        std::uint8_t amount;
        std::uint32_t* action_phase_count;

        constexpr std::string_view name() const noexcept
        {
            return source_name;
        }

        constexpr definition_type compile(definition_compile_context&) const noexcept
        {
            return {
                .amount = amount,
                .action_phase_count = action_phase_count
            };
        }

        static program_entry<action_phase_started> handle(
            const definition_type& data,
            const support_view&,
            action_phase_started&,
            const card_table&,
            random_fn&
        )
        {
            ++*data.action_phase_count;
            return program_entry<action_phase_started>::null();
        }

        static program_entry<onpay_context<cost_of_switch>> handle(
            const definition_type& data,
            const support_view& self,
            cost_of_switch& event,
            const card_table&,
            random_fn&
        )
        {
            if(event.target.player_id != self.player().id())
            {
                return program_entry<onpay_context<cost_of_switch>>::null();
            }
            event.requirement.dice_requirement.any += data.amount;
            event.effect_argument.reduced_dice.fixed[elemental_dice::pyro] = 7;
            return program_entry<onpay_context<cost_of_switch>>::null();
        }
    };

    struct fixed_switch_discount_source
    {
        using definition_category = support_view;

        struct definition_type
        {
            payment_log* log;
            std::uint8_t amount;
            program_entry<onpay_context<cost_of_switch>> onpay_entry;
        };

        std::string_view source_name;
        payment_log* log;
        std::uint8_t amount;

        constexpr std::string_view name() const noexcept
        {
            return source_name;
        }

        definition_type compile(definition_compile_context& context) const
        {
            return {
                .log = log,
                .amount = amount,
                .onpay_entry = context.add_program<onpay_context<cost_of_switch>>(
                    std::tuple{ consume_fixed_switch_discount{ .log = log } }
                )
            };
        }

        static program_entry<onpay_context<cost_of_switch>> handle(
            const definition_type& data,
            const support_view& self,
            cost_of_switch& event,
            const card_table&,
            random_fn&
        )
        {
            if(event.target.player_id != self.player().id())
            {
                return program_entry<onpay_context<cost_of_switch>>::null();
            }
            auto& remaining = event.requirement.dice_requirement.any;
            const auto reduction = std::min(remaining, data.amount);
            if(reduction == 0 || self.state().count < reduction)
            {
                return program_entry<onpay_context<cost_of_switch>>::null();
            }

            remaining -= reduction;
            event.effect_argument.reduced_dice.any = reduction;
            return data.onpay_entry;
        }
    };

    struct zero_random
    {
        std::uint32_t operator()() const noexcept
        {
            return 0;
        }
    };

    struct stop_execution
    {
        using context_type = void;

        bool execute(card_table&, execution_context& context, random_fn&) const noexcept
        {
            return context.yield();
        }
    };

    void run_until_blocked(executor& target, card_table& table, zero_random& random)
    {
        while(target.execute_next(table, random))
        {
        }
    }

    void submit_action(
        executor& target,
        const card_table& table,
        action_request request,
        action_argument argument = {}
    )
    {
        REQUIRE(target.status() == game_result::no_result);
        REQUIRE(table.definition_library().instruction(target.position()).is<begin_action>());
        auto&& [stored_argument, stored_request, stage] =
            target.stack().top<action_argument, action_request, stage_t>();
        (void)stage;
        stored_argument = argument;
        stored_request = request;
    }
}

TEST_CASE(
    "switch cost previews commit only the selected fixed onpay row",
    "[begin_action][fixed-program][onpay]"
)
{
    STATIC_REQUIRE(instruction_compatible_with<
        consume_fixed_switch_discount,
        onpay_context<cost_of_switch>
    >);
    STATIC_REQUIRE(not instruction_compatible_with<
        consume_fixed_switch_discount,
        cost_of_switch
    >);

    payment_log log;
    std::uint32_t action_phase_count = 0;
    const switch_tax_source tax_source{ "SwitchTax", 4, &action_phase_count };
    const fixed_switch_discount_source first_fixed_source{
        "FirstFixedSwitchDiscount", &log, 1
    };
    const fixed_switch_discount_source middle_fixed_source{
        "MiddleFixedSwitchDiscount", &log, 1
    };
    const fixed_switch_discount_source second_fixed_source{
        "SecondFixedSwitchDiscount", &log, 2
    };
    const test::named_definition_source<character_view> character_source{ "Character" };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{ begin_action{} },
        std::tuple{ stop_execution{} },
        tax_source,
        first_fixed_source,
        middle_fixed_source,
        second_fixed_source,
        character_source
    );

    const auto tax_definition = id_map.get_id<support_view>(tax_source.name());
    const auto first_fixed_definition =
        id_map.get_id<support_view>(first_fixed_source.name());
    const auto middle_fixed_definition =
        id_map.get_id<support_view>(middle_fixed_source.name());
    const auto second_fixed_definition =
        id_map.get_id<support_view>(second_fixed_source.name());
    const auto character_definition =
        id_map.get_id<character_view>(character_source.name());

    card_table table{ library };
    const auto player = player_id{ 0 };
    const auto opponent = player_id{ 1 };
    const auto tax = table[player].add(tax_definition, { .count = 10 }).id();
    const auto first_fixed = table[player].add(
        first_fixed_definition,
        { .count = 10 }
    ).id();
    const auto middle_fixed = table[player].add(
        middle_fixed_definition,
        { .count = 10 }
    ).id();
    const auto second_fixed = table[player].add(
        second_fixed_definition,
        { .count = 10 }
    ).id();

    const auto active = table[player].add(character_definition, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    }).id();
    table[player].add(character_definition, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    });
    const auto selected_target = table[player].add(character_definition, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    }).id();
    const auto opponent_active = table[opponent].add(character_definition, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    }).id();
    table[player].state().active_character = active;
    table[opponent].state().active_character = opponent_active;
    table.state().active_player = player;
    table[player].state().dice[elemental_dice::pyro] = 5;

    executor target;
    target.enter_entry(table.definition_library());
    zero_random random;
    run_until_blocked(target, table, random);
    REQUIRE(action_phase_count == 1);

    for(stack_count_t action_index = 0; action_index < 2; ++action_index)
    {
        submit_action(target, table, {
            .request_kind = action_request_kind::calculate_cost,
            .action_kind = action_kind::switch_active,
            .action_index = action_index
        });
        run_until_blocked(target, table, random);

        auto&& [
            handlers,
            costs,
            onpay_items,
            onpay_cursor,
            argument,
            request,
            stage
        ] = target.stack().top<
            detail::handler_id<cost_of_switch>[],
            cost_of_switch[],
            onpay_item<cost_of_switch>[],
            stack_count_t,
            action_argument,
            action_request,
            stage_t
        >();
        (void)handlers;
        (void)onpay_items;
        (void)onpay_cursor;
        (void)argument;
        (void)request;
        (void)stage;
        REQUIRE(costs.size() == 2);
        CHECK(costs[action_index].requirement.dice_requirement.any == 1);
    }

    CHECK(log.empty());
    CHECK(table[tax].state().count == 10);
    CHECK(table[first_fixed].state().count == 10);
    CHECK(table[middle_fixed].state().count == 10);
    CHECK(table[second_fixed].state().count == 10);
    CHECK(table[player].state().dice.total() == 5);

    dice_counts paid_dice;
    paid_dice[elemental_dice::pyro] = 1;
    submit_action(
        target,
        table,
        {
            .request_kind = action_request_kind::do_action_with_cost,
            .action_kind = action_kind::switch_active,
            .action_index = 1
        },
        { .paid_dice = paid_dice }
    );
    run_until_blocked(target, table, random);

    CHECK(log == payment_log{
        {
            .handler = first_fixed,
            .reduction = 1,
            .dice_total = 5
        },
        {
            .handler = middle_fixed,
            .reduction = 1,
            .dice_total = 5
        },
        {
            .handler = second_fixed,
            .reduction = 2,
            .dice_total = 5
        }
    });
    CHECK(table[tax].state().count == 10);
    CHECK(table[first_fixed].state().count == 9);
    CHECK(table[middle_fixed].state().count == 9);
    CHECK(table[second_fixed].state().count == 8);
    CHECK(table[player].state().dice.total() == 4);
    CHECK(table[player].state().active_character == selected_target);
    CHECK(table.state().active_player == opponent);
    CHECK(action_phase_count == 1);
    CHECK(target.status() == game_result::no_result);
    CHECK(table.definition_library().instruction(target.position()).is<begin_action>());
}
