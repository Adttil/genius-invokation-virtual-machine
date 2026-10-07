#include <array>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <span>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/basic_definitions.hpp>
#include <givm/executor.hpp>

#include "../test_character_source.hpp"

namespace givm_test::executor_instructions::control_frozen
{
constexpr std::array<std::size_t, 1> draw_positions_1{ 0 };

namespace
{
    constexpr givm::character_id actor{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id ally{ givm::player_id{ 0 }, 1 };
    constexpr givm::character_id target{ givm::player_id{ 1 }, 0 };
    constexpr givm::character_id reserve{ givm::player_id{ 1 }, 1 };

    struct control_log
    {
        bool shield = false;
        bool initialized = false;
        const givm::definition_library* library = nullptr;
        std::size_t reapplied = 0;
        std::vector<givm::character_id> switches;
        std::vector<std::uint32_t> values_before_shield;
        std::vector<bool> controlled_before_shield;
        std::vector<std::uint32_t> final_values;
        std::vector<bool> controlled_after_damage;
        std::vector<bool> controlled_at_end_phase;
        std::vector<bool> controlled_at_checkpoint;
        std::size_t rounds_started = 0;
        std::vector<std::array<std::uint32_t, 2>> dice_at_round_start;
        std::size_t frozen_removed = 0;
    };

    struct tagged_attachment
    {
        static constexpr auto category = givm::definition_category::attachment;
        struct definition_type { control_log* log; };
        std::string_view source_name;
        std::string_view source_tag;
        control_log* log;
        std::string_view name() const { return source_name; }
        auto tags() const { return std::array{ source_tag }; }
        definition_type compile(givm::definition_compile_context&) const { return { log }; }
        static givm::normal_effect handle(const definition_type& data,
            givm::this_attachment_reapply&, givm::handle_context<givm::attachment_view>&, std::uint32_t = 0)
        {
            ++data.log->reapplied;
            return {};
        }
    };

    template<class TProgram>
    struct control_driver
    {
        static constexpr auto category = givm::definition_category::skill;
        struct definition_type
        {
            control_log* log;
            givm::normal_effect initialization;
            givm::normal_effect dynamic_operations;
            givm::normal_effect end_phase;
            givm::normal_effect round_start;
            givm::optional_definition_id<givm::definition_category::attachment> control;
        };
        control_log* log;
        TProgram program;
        bool dynamic_operations;
        std::span<const givm::any_command> end_phase;
        bool pause_round_start = false;
        std::string_view name() const { return "ControlDriver"; }
        auto attachment_dependencies() const
        {
            return std::array<std::string_view, 4>{ "Control", "Immunity", "Ordinary", "frozen-3.3.0-genshin_impact" };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_normal_effect(program(context)),
                dynamic_operations ? context.add_normal_effect(std::tuple{
                    givm::attach{}, givm::add_attachment{}, givm::set_active_character{} }) : givm::normal_effect{},
                end_phase.empty() ? givm::normal_effect{} : context.add_normal_effect(end_phase),
                pause_round_start ? context.add_normal_effect(std::tuple{ givm::replace_cards{ givm::player_id{ 0 } } }) : givm::normal_effect{},
                context.resolve_id<givm::definition_category::attachment>("Control") };
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::battle_started&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            if(not data.log->initialized)
            {
                data.log->initialized = true;
                return context.invoke(data.initialization);
            }
            if(not data.dynamic_operations)
            {
                data.log->controlled_at_checkpoint.push_back(data.log->library->is_controlled(context.table()[target]));
                return {};
            }
            return context.invoke(data.dynamic_operations,
                givm::attach_input{ .target = actor, .definition = data.control.get(), .state = { 1 } },
                givm::add_attachment_input{ .target = actor, .definition = data.control.get(), .state = { 1 } },
                givm::set_active_character_input{ .current = ally });
        }
        static givm::preview_effect handle(const definition_type&,
            givm::cost_of_switch& event, givm::handle_context<givm::skill_view, givm::event_category::preview>&)
        {
            event.requirement.dice_requirement.any = 0;
            return {};
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::active_character_changed& event, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            CHECK(context.table()[event.current.player_id()].state().active_character == event.current);
            data.log->switches.push_back(event.current);
            return {};
        }
        static givm::immediate_effect handle(const definition_type& data,
            givm::damage_effect& event, givm::handle_context<givm::skill_view, givm::event_category::immediate>& context, std::uint32_t = 0)
        {
            data.log->values_before_shield.push_back(event.value);
            data.log->controlled_before_shield.push_back(data.log->library->is_controlled(context.table()[event.target]));
            if(data.log->shield && (event.type == givm::damage_type::physical || event.type == givm::damage_type::pyro))
                event.value = 0;
            return {};
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::after_damage& event, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            data.log->final_values.push_back(event.value);
            data.log->controlled_after_damage.push_back(data.log->library->is_controlled(context.table()[event.target]));
            return {};
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::round_ended&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            data.log->controlled_at_end_phase.push_back(data.log->library->is_controlled(context.table()[target]));
            return data.end_phase ? context.invoke(data.end_phase) : givm::normal_effect{};
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::round_started&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            ++data.log->rounds_started;
            data.log->dice_at_round_start.push_back({ context.table()[givm::player_id{ 0 }].state().dice.total(),
                context.table()[givm::player_id{ 1 }].state().dice.total() });
            return data.round_start ? context.invoke(data.round_start) : givm::normal_effect{};
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::attachment_removed& event, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            if(data.log->library->name(context.table()[event.attachment].definition_id()) == givm::genshin_impact::frozen_3_3_0.name())
                ++data.log->frozen_removed;
            return {};
        }
    };

    struct driver_character
    {
        static constexpr auto category = givm::definition_category::character;
        struct definition_type { givm::optional_definition_id<givm::definition_category::skill> skill; };
        std::string_view name() const { return "DriverCharacter"; }
        auto skill_dependencies() const { return std::array{ std::string_view{ "ControlDriver" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_id<givm::definition_category::skill>("ControlDriver") };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 20, .health = 20 };
        }
        static givm::optional_definition_id<givm::definition_category::skill> query(const definition_type& data, const givm::character_initial_skill& query)
        {
            return query.skill_index == 0 ? data.skill : givm::optional_definition_id<givm::definition_category::skill>{};
        }
    };

    struct talent_card
    {
        static constexpr auto category = givm::definition_category::card;
        struct definition_type {};
        std::string_view name() const { return "ControlledTalent"; }
        definition_type compile(givm::definition_compile_context&) const { return {}; }
        static givm::target_validation query(const definition_type&, const givm::card_target_validation& query)
        {
            const auto active = *query.table[query.card.player().id()].state().active_character;
            return query.target_count == 0 && not query.library.is_controlled(query.table[active])
                ? givm::target_validation::valid_complete : givm::target_validation::invalid;
        }
    };

    template<class TProgram>
    auto compile_scenario(control_log& log, bool observed, TProgram program,
        bool dynamic_operations = false, std::span<const givm::any_command> end_phase = {}, bool pause_round_start = false,
        std::span<const givm::any_command> round = {})
    {
        const driver_character character;
        const control_driver driver{ &log, program, dynamic_operations, end_phase, pause_round_start };
        const givm::test::initialized_character_source ordinary{ "PlainCharacter", { .max_health = 20, .health = 20 } };
        const tagged_attachment control{ "Control", "control", &log };
        const tagged_attachment immunity{ "Immunity", "control_immunity", &log };
        const tagged_attachment ordinary_attachment{ "Ordinary", "ordinary", &log };
        const talent_card card;
        const auto basics = givm::genshin_impact::reaction_names_3_3_0;
        givm::definition_source_library sources;
        sources.add(givm::genshin_impact::reaction_sources_3_3_0());
        REQUIRE(sources.add(character, driver, ordinary, control, immunity, ordinary_attachment, card,
            givm::genshin_impact::frozen_3_3_0));
        return givm_test::require_success(compile(sources, basics, std::tuple{ givm::start_battle{}, givm::settle{} }, round,
            observed ? givm::compile_mode::observed : givm::compile_mode::normal));
    }

    void load_scenario(givm::table& table, const givm::definition_library& library, const givm::issued_id_map& ids)
    {
        const auto ordinary = ids.get_id<givm::definition_category::character>("PlainCharacter");
        load_deck(table, library,
            { .cards = { ids.get_id<givm::definition_category::card>("ControlledTalent") },
                .characters = { ids.get_id<givm::definition_category::character>("DriverCharacter"), ordinary } },
            { .characters = { ordinary, ordinary } });
    }

    struct zero_random { std::uint32_t operator()() const { return 0; } };

    givm::execution_state advance(givm_test::executor_driver& executor, const givm::definition_library& library,
        givm::table& table, zero_random& random)
    {
        for(;;)
        {
            const auto state = givm_test::advance_selecting_first_alive(executor, library, table, random);
            if(state != givm::execution_state::active_character_changed && state != givm::execution_state::action_started)
                return state;
        }
    }

}

TEST_CASE("control immunity blocks fixed and dynamic control commands but permits voluntary switching", "[control][action]")
{
    const bool observed = GENERATE(false, true);
    const bool dynamic = GENERATE(false, true);
    const bool immune = GENERATE(false, true);
    const bool preexisting = GENERATE(false, true);
    control_log log;
    const auto [library, ids] = compile_scenario(log, observed, [&](givm::definition_compile_context& context)
    {
        std::vector<givm::any_command> commands;
        const auto control = context.resolve_id<givm::definition_category::attachment>("Control");
        if(preexisting) commands.emplace_back(givm::add_attachment{ .definition = control, .state = { 1 } });
        if(immune) commands.emplace_back(givm::add_attachment{ .definition = context.resolve_id<givm::definition_category::attachment>("Immunity") });
        commands.emplace_back(givm::add_attachment{ .definition = context.resolve_id<givm::definition_category::attachment>("Ordinary") });
        if(dynamic) { commands.emplace_back(givm::start_battle{}); commands.emplace_back(givm::settle{}); }
        else
        {
            commands.emplace_back(givm::attach{ .definition = control, .state = { 1 } });
            commands.emplace_back(givm::add_attachment{ .definition = control, .state = { 1 } });
            commands.emplace_back(givm::set_active_character{ givm::relative_character_target{ givm::relative_player::self, 1 } });
        }
        commands.emplace_back(givm::draw_cards{ .position = 0, .count = 1 });
        commands.emplace_back(givm::settle{});
        commands.emplace_back(givm::begin_action{});
        commands.emplace_back(givm::settle{});
    commands.emplace_back(givm::end_game{ givm::game_result::both_loss });
        return commands;
    }, dynamic);
    givm::table table{ { .round_number = 1, .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    log.library = &library;
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    REQUIRE(advance(executor, library, table, random) == givm::execution_state::action_selection);
    CHECK(library.is_control(ids.get_id<givm::definition_category::attachment>("Control")));
    CHECK(library.is_control(ids.get_id<givm::definition_category::attachment>(givm::genshin_impact::frozen_3_3_0.name())));
    CHECK_FALSE(library.is_control(ids.get_id<givm::definition_category::attachment>("Immunity")));
    CHECK(library.is_controlled(table[actor]) == (preexisting || not immune));
    CHECK(library.is_control_immune(table[actor]) == immune);
    CHECK_FALSE(library.is_controlled(table[ally]));
    CHECK_FALSE(library.is_control_immune(table[ally]));
    CHECK(std::ranges::distance(table[actor].attachments()) == (immune ? 2 + std::size_t(preexisting) : 3));
    CHECK(log.reapplied == std::size_t(preexisting && not immune));
    CHECK(log.switches.size() == (immune ? 0 : 1));
    CHECK(table[givm::player_id{ 0 }].state().active_character == (immune ? actor : ally));
    const auto view = executor.view_in<givm::execution_state::action_selection>();
    CHECK(view.is_controlled(library, table) == (immune && preexisting));
    REQUIRE(view.card_count() == 1);
    CHECK(view.card_targets_validate(library, table, 0) == (immune && preexisting
        ? givm::target_validation::invalid : givm::target_validation::valid_complete));
    REQUIRE(view.switch_target_count() == 1);
    const auto selected = view.switch_target(0);
    const auto quote_1 = view.calculate_switch_cost(library, table, 0);
    REQUIRE(view.switch_payment_validate(table, quote_1, {}) == givm::switch_payment_validation::valid);
    executor.submitted(view.switch_active_character_with_cached_cost(library, table, random, quote_1, {}));
    REQUIRE(advance(executor, library, table, random) == givm::execution_state::action_selection);
    CHECK(table[givm::player_id{ 0 }].state().active_character == selected);
    CHECK(log.switches.back() == selected);
}

TEST_CASE("control immunity prevents overload switching without suppressing its damage", "[control][overload]")
{
    const bool observed = GENERATE(false, true);
    const bool immune = GENERATE(false, true);
    control_log log;
    const std::array damages{ givm::deal_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::pyro } };
    const auto [library, ids] = compile_scenario(log, observed, [&](givm::definition_compile_context& context)
    {
        std::vector<givm::any_command> commands;
        if(immune) commands.emplace_back(givm::attach{ .player = givm::relative_player::opponent,
            .definition = context.resolve_id<givm::definition_category::attachment>("Immunity") });
        commands.emplace_back(givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::electro });
        for(const auto& item : damages) commands.emplace_back(item);
        commands.emplace_back(givm::settle{});
    commands.emplace_back(givm::end_game{ givm::game_result::both_loss });
        return commands;
    });
    givm::table table{ { .round_number = 1, .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    log.library = &library;
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    for(;;)
    {
        const auto state = advance(executor, library, table, random);
        if(state == givm::execution_state::finished) break;
        REQUIRE(observed);
        REQUIRE(state == givm::execution_state::health_reduced);
        CHECK(executor.view_in<givm::execution_state::health_reduced>().value() == 3);
    }
    CHECK(table[target].state().health == 17);
    CHECK(table[target].state().aura == givm::element_aura::none);
    CHECK(table[givm::player_id{ 1 }].state().active_character == (immune ? target : reserve));
    CHECK(log.switches.size() == (immune ? 0 : 1));
}

TEST_CASE("frozen is attached between grouped hits and shatters before damage absorption", "[control][frozen][damage_group]")
{
    const bool observed = GENERATE(false, true);
    const bool immune = GENERATE(false, true);
    const auto shatter_type = GENERATE(givm::damage_type::physical, givm::damage_type::pyro);
    control_log log{ .shield = true };
    const std::array damages{
        givm::deal_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::cryo },
        givm::deal_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = shatter_type }
    };
    const auto [library, ids] = compile_scenario(log, observed, [&](givm::definition_compile_context& context)
    {
        std::vector<givm::any_command> commands;
        if(immune) commands.emplace_back(givm::attach{ .player = givm::relative_player::opponent,
            .definition = context.resolve_id<givm::definition_category::attachment>("Immunity") });
        commands.emplace_back(givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::hydro });
        for(const auto& item : damages) commands.emplace_back(item);
        commands.emplace_back(givm::settle{});
    commands.emplace_back(givm::end_game{ givm::game_result::both_loss });
        return commands;
    });
    givm::table table{ { .round_number = 1, .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    log.library = &library;
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    std::size_t observations = 0;
    for(;;)
    {
        const auto state = advance(executor, library, table, random);
        if(state == givm::execution_state::finished) break;
        REQUIRE(observed);
        REQUIRE(state == givm::execution_state::health_reduced);
        const auto damage = executor.view_in<givm::execution_state::health_reduced>();
        CHECK((damage.reaction() ? damage.reaction().get().slot() : givm::elemental_reaction::none) == (observations == 0 ? givm::elemental_reaction::frozen : givm::elemental_reaction::none));
        ++observations;
    }
    CHECK(table[target].state().health == 18);
    CHECK(log.values_before_shield == std::vector<std::uint32_t>{ 2, immune ? 1u : 3u });
    CHECK(log.controlled_before_shield == std::vector<bool>{ false, false });
    CHECK(log.final_values == std::vector<std::uint32_t>{ 2 });
    CHECK_FALSE(library.is_controlled(table[target]));
    CHECK(library.is_control_immune(table[target]) == immune);
    CHECK(log.frozen_removed == (immune ? 0 : 1));
    CHECK(observations == (observed ? 1 : 0));
}

TEST_CASE("frozen remains through the end phase and is removed by the next round start", "[control][frozen][end_round]")
{
    const bool observed = GENERATE(false, true);
    const bool preexisting = GENERATE(false, true);
    control_log log;
    const std::array damages{ givm::deal_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1,
        .type = preexisting ? givm::damage_type::pyro : givm::damage_type::cryo } };
    const std::array<givm::any_command, 1> end_phase{ damages[0] };
    const std::array<givm::any_command, 4> round{
        givm::start_dice_roll_phase{ .count = 0, .reroll_count = { 0, 0 } }, givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } };
    const auto [library, ids] = compile_scenario(log, observed, [&](givm::definition_compile_context&)
    {
        std::vector<givm::any_command> commands;
        commands.emplace_back(givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::hydro });
        if(preexisting) commands.emplace_back(givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::cryo });
        commands.emplace_back(givm::end_round{});
    commands.emplace_back(givm::settle{});
        commands.emplace_back(givm::start_battle{});
    commands.emplace_back(givm::settle{});
        return commands;
    }, false, end_phase, false, round);
    givm::table table{ { .round_number = 1, .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    log.library = &library;
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    auto state = advance(executor, library, table, random);
    if(observed)
    {
        REQUIRE(state == givm::execution_state::round_ending);
        state = advance(executor, library, table, random);
        REQUIRE(state == givm::execution_state::health_reduced);
        CHECK(log.rounds_started == 0);
        CHECK(executor.view_in<givm::execution_state::health_reduced>().value() == (preexisting ? 3 : 2));
        auto copied_executor = executor;
        auto copied_table = table;
        const auto finish = [&](givm_test::executor_driver& running, givm::table& current)
        {
            REQUIRE(advance(running, library, current, random) == givm::execution_state::round_started);
            CHECK(library.is_controlled(current[target]) == not preexisting);
            REQUIRE(advance(running, library, current, random) == givm::execution_state::finished);
            CHECK_FALSE(library.is_controlled(current[target]));
        };
        finish(executor, table);
        CHECK(log.rounds_started == 1);
        CHECK(log.frozen_removed == 1);
        CHECK(log.controlled_after_damage == std::vector<bool>{ not preexisting });
        CHECK(log.controlled_at_checkpoint == std::vector<bool>{ not preexisting });
        finish(copied_executor, copied_table);
        CHECK(log.rounds_started == 2);
        CHECK(copied_table[target].state().health == table[target].state().health);
    }
    else
    {
        REQUIRE(state == givm::execution_state::finished);
        CHECK(log.rounds_started == 1);
        CHECK(log.frozen_removed == 1);
        CHECK(log.controlled_after_damage == std::vector<bool>{ not preexisting });
        CHECK(log.controlled_at_checkpoint == std::vector<bool>{ not preexisting });
    }
    CHECK(log.controlled_at_end_phase == std::vector<bool>{ preexisting });
    CHECK(table[target].state().health == (preexisting ? 17 : 18));
    CHECK_FALSE(library.is_controlled(table[target]));
    CHECK(std::ranges::empty(table[target].attachments()));
}

TEST_CASE("a lethal frozen reaction does not attach control to the defeated character", "[control][frozen][defeat]")
{
    control_log log;
    const std::array damages{ givm::deal_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 20, .type = givm::damage_type::cryo } };
    const auto [library, ids] = compile_scenario(log, false, [&](givm::definition_compile_context&)
    {
        std::vector<givm::any_command> commands;
        commands.emplace_back(givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::hydro });
        for(const auto& item : damages) commands.emplace_back(item);
        commands.emplace_back(givm::settle{});
    commands.emplace_back(givm::end_game{ givm::game_result::both_loss });
        return commands;
    });
    givm::table table{ { .round_number = 1, .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    log.library = &library;
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    REQUIRE(givm_test::advance_selecting_first_alive(executor, library, table, random) == givm::execution_state::finished);
    CHECK(table[target].state().health == 0);
    CHECK(table[reserve].state().health == 20);
    CHECK_FALSE(library.is_controlled(table[target]));
    CHECK(std::ranges::empty(table[target].attachments()));
    CHECK(log.controlled_after_damage == std::vector<bool>{ false });
}

TEST_CASE("round start responses wait for both rerolls and resume independently after copying", "[frozen][round-start][dice]")
{
    const bool observed = GENERATE(false, true);
    control_log log;
    const std::array<givm::any_command, 4> round{
        givm::start_dice_roll_phase{ .count = 2, .reroll_count = { 1, 1 } }, givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } };
    const auto [library, ids] = compile_scenario(log, observed, [&](givm::definition_compile_context& context)
    {
        std::vector<givm::any_command> commands;
        commands.emplace_back(givm::attach{ .player = givm::relative_player::opponent,
            .definition = context.resolve_id<givm::definition_category::attachment>("frozen-3.3.0-genshin_impact") });
        return commands;
    }, false, {}, true, round);
    givm::table table{ { .round_number = 1, .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    log.library = &library;
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    auto state = advance(executor, library, table, random);
    if(observed)
    {
        REQUIRE(state == givm::execution_state::round_started);
        CHECK(table.state().round_number == 2);
        CHECK(log.rounds_started == 0);
        CHECK(library.is_controlled(table[target]));
        state = advance(executor, library, table, random);
    }
    REQUIRE(state == givm::execution_state::dice_selection);
    CHECK(table.state().round_number == 2);
    CHECK(log.rounds_started == 0);
    CHECK(library.is_controlled(table[target]));
    executor.submitted(executor.view_in<givm::execution_state::dice_selection>().select(library, table, random, {}));
    REQUIRE(advance(executor, library, table, random) == givm::execution_state::dice_selection);
    CHECK(log.rounds_started == 0);
    CHECK(library.is_controlled(table[target]));
    executor.submitted(executor.view_in<givm::execution_state::dice_selection>().select(library, table, random, {}));
    REQUIRE(advance(executor, library, table, random) == givm::execution_state::card_selection);
    CHECK(log.rounds_started == 1);
    CHECK(log.dice_at_round_start == std::vector<std::array<std::uint32_t, 2>>{ { 2, 2 } });
    CHECK(library.is_controlled(table[target]));
    auto copied_executor = executor;
    auto copied_table = table;
    const auto finish = [&](givm_test::executor_driver& running, givm::table& current)
    {
        running.submitted(running.view_in<givm::execution_state::card_selection>().select(library, current, random, {}));
        REQUIRE(advance(running, library, current, random) == givm::execution_state::finished);
        CHECK_FALSE(library.is_controlled(current[target]));
        CHECK(current[givm::player_id{ 0 }].state().dice.total() == 2);
        CHECK(current[givm::player_id{ 1 }].state().dice.total() == 2);
        CHECK(log.rounds_started == 1);
    };
    finish(executor, table);
    CHECK(library.is_controlled(copied_table[target]));
    finish(copied_executor, copied_table);
    CHECK(log.frozen_removed == 2);
}

TEST_CASE("exceeding the round limit prevents rolling and round start responses", "[round-start][dice]")
{
    const bool observed = GENERATE(false, true);
    control_log log;
    const std::array<givm::any_command, 4> round{
        givm::start_dice_roll_phase{}, givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } };
    const auto [library, ids] = compile_scenario(log, observed, [&](givm::definition_compile_context& context)
    {
        std::vector<givm::any_command> commands;
        commands.emplace_back(givm::attach{ .player = givm::relative_player::opponent,
            .definition = context.resolve_id<givm::definition_category::attachment>("frozen-3.3.0-genshin_impact") });
        return commands;
    }, false, {}, false, round);
    givm::table table{ givm::table_state{ .round_number = 1, .max_rounds = 1, .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    log.library = &library;
    givm_test::executor_driver executor;
    executor.start(library, table);
    zero_random random;
    auto state = advance(executor, library, table, random);
    if(observed)
    {
        REQUIRE(state == givm::execution_state::round_started);
        state = advance(executor, library, table, random);
    }
    REQUIRE(state == givm::execution_state::finished);
    CHECK(table.state().round_number == 2);
    CHECK(log.rounds_started == 0);
    CHECK(log.frozen_removed == 0);
    CHECK(library.is_controlled(table[target]));
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 0);
    CHECK(table[givm::player_id{ 1 }].state().dice.total() == 0);
}
}
