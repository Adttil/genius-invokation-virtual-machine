#include "../../executor_access.hpp"
#include <algorithm>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/executor.hpp>

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

        execution_state execute(const definition_library&, card_table& table, detail::execution_context& context, random_fn&) const
        {
            auto&& [onpay, activation] = context.stack().top<
                frame<
                    detail::handler_id<cost_of_switch>,
                    cost_effect_argument<cost_of_switch>,
                    detail::stage_t
                >,
                frame<detail::execution_context::return_info, detail::stage_t>
            >();
            auto&& [current_handler, argument, onpay_stage] = onpay;
            auto&& [return_info, activation_stage] = activation;
            (void)onpay_stage;
            (void)return_info;
            REQUIRE(activation_stage == detail::stage_t{});

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

    struct action_observer_source
    {
        using definition_category = support_view;

        struct definition_type
        {
            action_speed speed;
            std::vector<player_id>* before_actions;
        };

        action_speed speed;
        std::vector<player_id>* before_actions;

        constexpr std::string_view name() const noexcept
        {
            return "ActionObserver";
        }

        constexpr definition_type compile(definition_compile_context&) const noexcept
        {
            return { speed, before_actions };
        }

        static program_entry<before_action> handle(
            const definition_type& data,
            const support_view&,
            before_action&,
            const card_table& table,
            random_fn&
        )
        {
            data.before_actions->push_back(table.state().active_player);
            return program_entry<before_action>::null();
        }

        static program_entry<onpay_context<cost_of_switch>> handle(
            const definition_type& data,
            const support_view&,
            cost_of_switch& event,
            const card_table&,
            random_fn&
        )
        {
            event.requirement.speed = data.speed;
            return program_entry<onpay_context<cost_of_switch>>::null();
        }
    };

    struct switch_observer_source
    {
        using definition_category = support_view;

        struct definition_type
        {
            std::vector<character_id>* active_characters;
        };

        std::vector<character_id>* active_characters;

        constexpr std::string_view name() const noexcept { return "SwitchObserver"; }

        constexpr definition_type compile(definition_compile_context&) const noexcept
        {
            return { active_characters };
        }

        static program_entry<active_character_changed> handle(
            const definition_type& data,
            const support_view&,
            active_character_changed& event,
            const card_table& table,
            random_fn&
        )
        {
            const auto active = table[event.current.player_id].state().active_character;
            REQUIRE(active.has_value());
            CHECK(*active == event.current);
            data.active_characters->push_back(*active);
            return program_entry<active_character_changed>::null();
        }
    };

    struct terminal_switch_source
    {
        using definition_category = support_view;

        struct definition_type
        {
            std::uint32_t* preview_count;
            program_entry<onpay_context<cost_of_switch>> entry;
        };

        std::uint32_t* preview_count;

        constexpr std::string_view name() const noexcept
        {
            return "TerminalSwitch";
        }

        definition_type compile(definition_compile_context& context) const
        {
            return {
                .preview_count = preview_count,
                .entry = context.add_program<onpay_context<cost_of_switch>>(std::tuple{
                    end_game{ .result = game_result::player_1_win }
                })
            };
        }

        static program_entry<onpay_context<cost_of_switch>> handle(
            const definition_type& data,
            const support_view&,
            cost_of_switch&,
            const card_table&,
            random_fn&
        )
        {
            ++*data.preview_count;
            return data.entry;
        }
    };

    struct round_observer_source
    {
        using definition_category = support_view;

        struct definition_type
        {
            std::vector<player_id>* declarations;
            std::vector<player_id>* endings;
        };

        std::vector<player_id>* declarations;
        std::vector<player_id>* endings;

        constexpr std::string_view name() const noexcept { return "RoundObserver"; }

        constexpr definition_type compile(definition_compile_context&) const noexcept
        {
            return { declarations, endings };
        }

        static program_entry<round_end_declared> handle(
            const definition_type& data,
            const support_view&,
            round_end_declared&,
            const card_table& table,
            random_fn&
        )
        {
            data.declarations->push_back(table.state().active_player);
            return program_entry<round_end_declared>::null();
        }

        static program_entry<round_ended> handle(
            const definition_type& data,
            const support_view&,
            round_ended&,
            const card_table& table,
            random_fn&
        )
        {
            data.endings->push_back(table.state().active_player);
            return program_entry<round_ended>::null();
        }
    };

    struct grant_action_die
    {
        using context_type = before_action;

        execution_state execute(const definition_library&, card_table& table, detail::execution_context& context, random_fn&) const
        {
            ++table[table.state().active_player].state().dice[elemental_dice::pyro];
            return context.enter_next();
        }
    };

    struct action_response_source
    {
        using definition_category = support_view;

        struct definition_type
        {
            program_entry<before_action> entry;
            std::uint32_t* calls;
        };

        std::string_view source_name;
        bool enabled;
        std::uint32_t* calls;

        constexpr std::string_view name() const noexcept { return source_name; }

        definition_type compile(definition_compile_context& context) const
        {
            return {
                .entry = enabled
                    ? context.add_program<before_action>(std::tuple{ grant_action_die{} })
                    : program_entry<before_action>::null(),
                .calls = calls
            };
        }

        static program_entry<before_action> handle(
            const definition_type& data,
            const support_view&,
            before_action&,
            const card_table&,
            random_fn&
        )
        {
            ++*data.calls;
            return data.entry;
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

        execution_state execute(const definition_library&, card_table&, detail::execution_context& context, random_fn&) const noexcept
        {
            return context.yield(execution_state::action);
        }
    };

    void run_until_blocked(const definition_library& library, executor& target, card_table& table, zero_random& random)
    {
        REQUIRE(target.run(library, table, random) == execution_state::action);
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

    card_table table{};
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
    target.enter_entry(library);
    zero_random random;
    run_until_blocked(library, target, table, random);
    REQUIRE(action_phase_count == 1);

    for(stack_count_t action_index = 0; action_index < 2; ++action_index)
    {
        target.view_in<execution_state::action>().request_cost(action_index);
        run_until_blocked(library, target, table, random);

        const auto costs = target.view_in<execution_state::action>().costs();
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
    target.view_in<execution_state::action>().execute_action_with_cost(1, { .paid_dice = paid_dice });
    run_until_blocked(library, target, table, random);

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
    CHECK(target.view_in<execution_state::action>().costs().empty());
}

TEST_CASE(
    "actions skip the player who declared round end until the next round",
    "[begin_action][game-flow]"
)
{
    const auto initial_player = GENERATE(player_id{ 0 }, player_id{ 1 });
    const auto speed = GENERATE(action_speed::combat, action_speed::fast);
    CAPTURE(initial_player, speed);

    std::vector<player_id> before_actions;
    const action_observer_source observer{ speed, &before_actions };
    const test::named_definition_source<character_view> character_source{ "Character" };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{ start_round{} },
        std::tuple{ begin_action{}, end_round{}, start_round{} },
        observer,
        character_source
    );

    card_table table{};
    table[initial_player].add(id_map.get_id<support_view>(observer.name()), {});
    const auto character_definition = id_map.get_id<character_view>(character_source.name());
    for(auto player : table.players())
    {
        const auto active = player.add(character_definition, {
            .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
        }).id();
        player.add(character_definition, {
            .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
        });
        player.state().active_character = active;
    }
    table.state().active_player = initial_player;

    executor target;
    target.enter_entry(library);
    zero_random random;
    run_until_blocked(library, target, table, random);
    REQUIRE(before_actions == std::vector<player_id>{ initial_player });
    for(auto player : table.players())
    {
        player.state().dice[elemental_dice::pyro] = 3;
    }

    dice_counts paid_dice;
    paid_dice[elemental_dice::pyro] = 1;
    const auto switch_active = [&]
    {
        before_actions.clear();
        target.view_in<execution_state::action>().execute_action(0, { .paid_dice = paid_dice });
        run_until_blocked(library, target, table, random);
    };
    const auto declare_round_end = [&]
    {
        before_actions.clear();
        target.view_in<execution_state::action>().declare_round_end();
        run_until_blocked(library, target, table, random);
    };

    switch_active();
    const auto first_ended = speed == action_speed::combat
        ? other_player(initial_player)
        : initial_player;
    REQUIRE(table.state().active_player == first_ended);
    REQUIRE(before_actions == std::vector<player_id>{ first_ended });
    REQUIRE_FALSE(table.state().first_ended);

    declare_round_end();
    const auto continuing_player = other_player(first_ended);
    REQUIRE(table.state().active_player == continuing_player);
    REQUIRE(table.state().first_ended);
    REQUIRE(before_actions == std::vector<player_id>{ continuing_player });

    for(int action = 0; action < 2; ++action)
    {
        const auto active_before = table[continuing_player].state().active_character;
        const auto dice_before = table[continuing_player].state().dice.total();
        switch_active();
        REQUIRE(table.state().active_player == continuing_player);
        CHECK(table.state().first_ended);
        CHECK(table[continuing_player].state().active_character != active_before);
        CHECK(table[continuing_player].state().dice.total() == dice_before - 1);
        CHECK(before_actions == std::vector<player_id>{ continuing_player });
    }

    declare_round_end();
    REQUIRE(table.state().round_number == 2);
    REQUIRE_FALSE(table.state().first_ended);
    REQUIRE(table.state().active_player == first_ended);
    REQUIRE(before_actions == std::vector<player_id>{ first_ended });

    table[first_ended].state().dice[elemental_dice::pyro] = 1;
    switch_active();
    const auto next_player = speed == action_speed::combat ? continuing_player : first_ended;
    CHECK(table.state().active_player == next_player);
    CHECK(before_actions == std::vector<player_id>{ next_player });
}

TEST_CASE(
    "switch actions offer only living standby characters",
    "[begin_action][switch-targets]"
)
{
    const bool has_living_targets = GENERATE(true, false);
    CAPTURE(has_living_targets);

    std::vector<player_id> before_actions;
    const action_observer_source observer{ action_speed::fast, &before_actions };
    const test::named_definition_source<character_view> character_source{ "Character" };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{ begin_action{} },
        std::tuple{ stop_execution{} },
        observer,
        character_source
    );

    card_table table{};
    const auto player = player_id{ 0 };
    const auto opponent = player_id{ 1 };
    table[player].add(id_map.get_id<support_view>(observer.name()), {});
    const auto character_definition = id_map.get_id<character_view>(character_source.name());
    std::vector<character_id> characters;
    for(const auto health : { 0u, 1u, 10u, 10u, 0u, 10u })
    {
        characters.push_back(table[player].add(character_definition, {
            .max_health = 10, .max_energy = 3, .health = health, .energy = 0
        }).id());
    }
    table[characters[3]].erase();
    table[player].state().active_character = characters[2];
    table[opponent].state().active_character = table[opponent].add(character_definition, {
        .max_health = 10, .max_energy = 3, .health = 10, .energy = 0
    }).id();
    table.state().active_player = player;
    table[player].state().dice[elemental_dice::pyro] = 2;

    if(not has_living_targets)
    {
        table[characters[1]].state().health = 0;
        table[characters[5]].state().health = 0;
    }

    executor target;
    target.enter_entry(library);
    zero_random random;
    run_until_blocked(library, target, table, random);

    const auto check_targets = [&](const std::vector<character_id>& expected)
    {
        auto&& [handlers, costs, onpay_items, onpay_cursor, argument, request, stage] =
            detail::executor_access::stack(target).top<
                detail::handler_id<cost_of_switch>[],
                cost_of_switch[],
                onpay_item<cost_of_switch>[],
                stack_count_t,
                action_argument,
                detail::action_request,
                detail::stage_t
            >();
        const auto visible_costs = target.view_in<execution_state::action>().costs();
        REQUIRE(visible_costs.size() == expected.size());
        REQUIRE(costs.size() == expected.size());
        REQUIRE(handlers.size() == 1);
        CHECK(onpay_items.size() == expected.size());
        for(size_t index = 0; index < expected.size(); ++index)
        {
            CHECK(visible_costs[index].target == expected[index]);
        }
    };

    if(has_living_targets)
    {
        check_targets({ characters[1], characters[5] });
        for(stack_count_t index = 0; index < 2; ++index)
        {
            target.view_in<execution_state::action>().request_cost(index);
            run_until_blocked(library, target, table, random);
            check_targets({ characters[1], characters[5] });
        }

        dice_counts paid_dice;
        paid_dice[elemental_dice::pyro] = 1;
        target.view_in<execution_state::action>().execute_action_with_cost(1, { .paid_dice = paid_dice });
        run_until_blocked(library, target, table, random);
        REQUIRE(table[player].state().active_character == characters[5]);
        CHECK(table[player].state().dice.total() == 1);
        check_targets({ characters[1], characters[2] });

        target.view_in<execution_state::action>().execute_action(0, { .paid_dice = paid_dice });
        run_until_blocked(library, target, table, random);
        REQUIRE(table[player].state().active_character == characters[1]);
        CHECK(table[player].state().dice.total() == 0);
        check_targets({ characters[2], characters[5] });
    }
    else
    {
        check_targets({});
        target.view_in<execution_state::action>().declare_round_end();
        run_until_blocked(library, target, table, random);
        CHECK(table.state().active_player == opponent);
        CHECK(table.state().first_ended);
        CHECK(table[player].state().active_character == characters[2]);
        CHECK(table[player].state().dice.total() == 2);
        check_targets({});
    }

    CHECK(static_cast<bool>(table[characters[0]]));
    CHECK(static_cast<bool>(table[characters[4]]));
    CHECK(table[characters[0]].state().health == 0);
    CHECK(table[characters[4]].state().health == 0);
    CHECK_FALSE(static_cast<bool>(table[characters[3]]));
}

TEST_CASE("an observed switch exposes its destination after payment and before applying it", "[begin_action][execution-view]")
{
    std::vector<player_id> before_actions;
    const action_observer_source observer{ action_speed::fast, &before_actions };
    const test::named_definition_source<character_view> character_source{ "Character" };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{ begin_action{} }, std::tuple{ stop_execution{} }, observer, character_source
    );
    card_table table{};
    const player_id player{ 0 };
    table[player].add(id_map.get_id<support_view>(observer.name()), {});
    const auto character_definition = id_map.get_id<character_view>(character_source.name());
    const auto previous = table[player].add(character_definition, { .health = 10 }).id();
    const auto current = table[player].add(character_definition, { .health = 10 }).id();
    table[player].state().active_character = previous;
    table[player].state().dice[elemental_dice::pyro] = 1;
    table.state().active_player = player;

    executor target;
    target.enter_entry(library);
    zero_random random;
    REQUIRE(target.run(library, table, random) == execution_state::action);
    REQUIRE(before_actions == std::vector<player_id>{ player });
    dice_counts paid;
    paid[elemental_dice::pyro] = 1;
    target.view_in<execution_state::action>().execute_action(0, { .paid_dice = paid });

    REQUIRE(target.step(library, table, random) == execution_state::active_character_changed);
    const auto view = target.view_in<execution_state::active_character_changed>();
    CHECK(view.character() == current);
    CHECK(table[player].state().active_character == previous);
    CHECK(table[player].state().dice.total() == 0);
    CHECK(before_actions == std::vector<player_id>{ player });

    REQUIRE(target.step(library, table, random) == execution_state::action);
    CHECK(table[player].state().active_character == current);
    CHECK(before_actions == std::vector<player_id>{ player, player });
}

TEST_CASE("a terminal onpay entry waits for action confirmation", "[begin_action][onpay][terminal]")
{
    const bool observed = GENERATE(false, true);
    CAPTURE(observed);
    std::uint32_t preview_count = 0;
    const terminal_switch_source terminal_source{ &preview_count };
    const test::named_definition_source<character_view> character_source{ "Character" };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{ begin_action{} }, std::tuple{ stop_execution{} }, terminal_source, character_source
    );
    card_table table{};
    const player_id player{ 0 };
    table[player].add(id_map.get_id<support_view>(terminal_source.name()), {});
    const auto character_definition = id_map.get_id<character_view>(character_source.name());
    const auto active = table[player].add(character_definition, { .health = 10 }).id();
    table[player].add(character_definition, { .health = 10 });
    table[player].state().active_character = active;
    table[player].state().dice[elemental_dice::pyro] = 1;
    table.state().active_player = player;
    executor target;
    target.enter_entry(library);
    zero_random random;

    REQUIRE(target.run(library, table, random) == execution_state::action);
    CHECK(preview_count == 0);
    for(std::uint32_t preview = 1; preview <= 2; ++preview)
    {
        target.view_in<execution_state::action>().request_cost(0);
        const auto state = observed ? target.step(library, table, random) : target.run(library, table, random);
        REQUIRE(state == execution_state::action);
        CHECK(preview_count == preview);
        REQUIRE(target.view_in<execution_state::action>().costs().size() == 1);
        CHECK(table[player].state().active_character == active);
        CHECK(table[player].state().dice[elemental_dice::pyro] == 1);
    }

    dice_counts paid;
    paid[elemental_dice::pyro] = 1;
    target.view_in<execution_state::action>().execute_action_with_cost(0, { .paid_dice = paid });
    const auto state = observed ? target.step(library, table, random) : target.run(library, table, random);
    REQUIRE(state == execution_state::finished);
    CHECK(target.view_in<execution_state::finished>().result() == game_result::player_1_win);
    CHECK(preview_count == 2);
    CHECK(table[player].state().active_character == active);
    CHECK(table[player].state().dice[elemental_dice::pyro] == 1);
}

TEST_CASE("an action starts before its responses and response entries add no observation", "[begin_action][execution-view]")
{
    std::uint32_t skipped_calls = 0;
    std::uint32_t response_calls = 0;
    const action_response_source skipped_source{ "NoEffect", false, &skipped_calls };
    const action_response_source response_source{ "GrantDie", true, &response_calls };
    const test::named_definition_source<character_view> character_source{ "Character" };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{ begin_action{} }, std::tuple{ stop_execution{} },
        skipped_source, response_source, character_source
    );
    card_table table{};
    const player_id player{ 0 };
    table[player].add(id_map.get_id<support_view>(skipped_source.name()), {});
    table[player].add(id_map.get_id<support_view>(response_source.name()), {});
    const auto active = table[player].add(
        id_map.get_id<character_view>(character_source.name()), { .health = 10 }
    ).id();
    table[player].state().active_character = active;
    table.state().active_player = player;
    executor target;
    target.enter_entry(library);
    zero_random random;

    REQUIRE(target.step(library, table, random) == execution_state::action_started);
    (void)target.view_in<execution_state::action_started>();
    CHECK(skipped_calls == 0);
    CHECK(response_calls == 0);
    CHECK(table[player].state().dice[elemental_dice::pyro] == 0);

    REQUIRE(target.step(library, table, random) == execution_state::action);
    CHECK(table[player].state().dice[elemental_dice::pyro] == 1);
    CHECK(skipped_calls == 1);
    CHECK(response_calls == 1);
}

TEST_CASE("action opportunities and round boundaries stop before their responses", "[begin_action][execution-view][round]")
{
    const auto initial_player = GENERATE(player_id{ 0 }, player_id{ 1 });
    const auto speed = GENERATE(action_speed::combat, action_speed::fast);
    CAPTURE(initial_player, speed);

    std::vector<player_id> before_actions;
    std::vector<player_id> declarations;
    std::vector<player_id> endings;
    const action_observer_source action_observer{ speed, &before_actions };
    const round_observer_source round_observer{ &declarations, &endings };
    std::vector<character_id> switched_characters;
    const switch_observer_source switch_observer{ &switched_characters };
    const test::named_definition_source<character_view> character_source{ "Character" };
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{ begin_action{}, end_round{}, end_game{ .result = game_result::both_loss } },
        std::tuple{ stop_execution{} }, action_observer, round_observer, switch_observer, character_source
    );
    card_table table{};
    table[initial_player].add(id_map.get_id<support_view>(action_observer.name()), {});
    table[initial_player].add(id_map.get_id<support_view>(round_observer.name()), {});
    table[initial_player].add(id_map.get_id<support_view>(switch_observer.name()), {});
    const auto character_definition = id_map.get_id<character_view>(character_source.name());
    for(auto player : table.players())
    {
        const auto active = player.add(character_definition, { .health = 10 }).id();
        player.add(character_definition, { .health = 10 });
        player.state().active_character = active;
        player.state().dice[elemental_dice::pyro] = 3;
    }
    table.state().active_player = initial_player;
    executor target;
    target.enter_entry(library);
    zero_random random;

    REQUIRE(target.step(library, table, random) == execution_state::action_started);
    (void)target.view_in<execution_state::action_started>();
    CHECK(table.state().active_player == initial_player);
    CHECK(before_actions.empty());
    REQUIRE(target.step(library, table, random) == execution_state::action);
    REQUIRE(before_actions == std::vector<player_id>{ initial_player });

    const auto switch_active = [&](player_id next_player)
    {
        const auto player = table.state().active_player;
        const auto next_character = target.view_in<execution_state::action>().costs()[0].target;
        const auto previous_character = table[player].state().active_character;
        const auto previous_dice = table[player].state().dice.total();
        const auto switch_count = switched_characters.size();
        const auto before_count = before_actions.size();
        dice_counts paid;
        paid[elemental_dice::pyro] = 1;
        target.view_in<execution_state::action>().execute_action(0, { .paid_dice = paid });
        REQUIRE(target.step(library, table, random) == execution_state::active_character_changed);
        CHECK(target.view_in<execution_state::active_character_changed>().character() == next_character);
        CHECK(table[player].state().active_character == previous_character);
        CHECK(table[player].state().dice.total() == previous_dice - 1);
        CHECK(switched_characters.size() == switch_count);
        CHECK(before_actions.size() == before_count);
        if(speed == action_speed::combat)
        {
            REQUIRE(target.step(library, table, random) == execution_state::action_started);
            (void)target.view_in<execution_state::action_started>();
            CHECK(table.state().active_player == next_player);
            CHECK(table[player].state().active_character == next_character);
            REQUIRE(switched_characters.size() == switch_count + 1);
            CHECK(switched_characters.back() == next_character);
            CHECK(before_actions.size() == before_count);
        }
        REQUIRE(target.step(library, table, random) == execution_state::action);
        CHECK(table.state().active_player == next_player);
        CHECK(table[player].state().active_character == next_character);
        REQUIRE(switched_characters.size() == switch_count + 1);
        CHECK(switched_characters.back() == next_character);
        CHECK(before_actions.size() == before_count + 1);
        CHECK(before_actions.back() == next_player);
    };

    const auto first_ended = speed == action_speed::combat
        ? other_player(initial_player)
        : initial_player;
    switch_active(first_ended);

    target.view_in<execution_state::action>().declare_round_end();
    REQUIRE(target.step(library, table, random) == execution_state::round_end_declared);
    (void)target.view_in<execution_state::round_end_declared>();
    CHECK(table.state().active_player == first_ended);
    CHECK(table.state().first_ended);
    CHECK(declarations.empty());
    const auto before_handoff = before_actions.size();
    const auto continuing_player = other_player(first_ended);
    REQUIRE(target.step(library, table, random) == execution_state::action_started);
    (void)target.view_in<execution_state::action_started>();
    CHECK(table.state().active_player == continuing_player);
    CHECK(declarations == std::vector<player_id>{ first_ended });
    CHECK(before_actions.size() == before_handoff);
    REQUIRE(target.step(library, table, random) == execution_state::action);

    // A combat action opens another opportunity even when the opponent has ended.
    switch_active(continuing_player);
    switch_active(continuing_player);

    target.view_in<execution_state::action>().declare_round_end();
    REQUIRE(target.step(library, table, random) == execution_state::round_end_declared);
    (void)target.view_in<execution_state::round_end_declared>();
    CHECK(table.state().active_player == continuing_player);
    CHECK(declarations == std::vector<player_id>{ first_ended });
    REQUIRE(target.step(library, table, random) == execution_state::round_ending);
    (void)target.view_in<execution_state::round_ending>();
    CHECK(declarations == std::vector<player_id>{ first_ended, continuing_player });
    CHECK(endings.empty());
    CHECK(table.state().active_player == continuing_player);
    CHECK(table.state().first_ended);

    REQUIRE(target.step(library, table, random) == execution_state::finished);
    CHECK(target.view_in<execution_state::finished>().result() == game_result::both_loss);
    CHECK(endings == std::vector<player_id>{ first_ended });
    CHECK(table.state().active_player == first_ended);
    CHECK_FALSE(table.state().first_ended);
}

TEST_CASE("a round starts before its limit check and dice reset", "[start_round][execution-view]")
{
    const bool observed = GENERATE(false, true);
    const bool exceeds_limit = GENERATE(false, true);
    CAPTURE(observed, exceeds_limit);
    const auto [library, id_map] = test::compile_definitions_with_program(
        std::tuple{ start_round{ .max_rounds = 2 }, stop_execution{} },
        std::tuple{ stop_execution{} }
    );
    card_table table{};
    table.state().round_number = exceeds_limit ? 2 : 1;
    for(auto player : table.players())
    {
        player.state().dice[elemental_dice::pyro] = 3;
    }
    executor target;
    target.enter_entry(library);
    zero_random random;
    const auto initial_stack_size = detail::executor_access::stack(target).size();

    if(observed)
    {
        REQUIRE(target.step(library, table, random) == execution_state::round_started);
        (void)target.view_in<execution_state::round_started>();
        CHECK(table.state().round_number == (exceeds_limit ? 3 : 2));
        CHECK(detail::executor_access::stack(target).size() == initial_stack_size);
        for(auto player : table.players())
        {
            CHECK(player.state().dice[elemental_dice::pyro] == 3);
        }
    }

    const auto state = observed ? target.step(library, table, random) : target.run(library, table, random);
    CHECK(table.state().round_number == (exceeds_limit ? 3 : 2));
    if(exceeds_limit)
    {
        REQUIRE(state == execution_state::finished);
        CHECK(target.view_in<execution_state::finished>().result() == game_result::both_loss);
        for(auto player : table.players())
        {
            CHECK(player.state().dice[elemental_dice::pyro] == 3);
        }
    }
    else
    {
        REQUIRE(state == execution_state::action);
        for(auto player : table.players())
        {
            CHECK(player.state().dice.total() == 0);
        }
    }
}
