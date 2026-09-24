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

namespace
{
    constexpr givm::character_id actor{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id ally{ givm::player_id{ 0 }, 1 };
    constexpr givm::character_id target{ givm::player_id{ 1 }, 0 };
    constexpr givm::character_id reserve{ givm::player_id{ 1 }, 1 };

    struct control_log
    {
        bool shield = false;
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
        using definition_category = givm::attachment_view;
        struct definition_type { control_log* log; };
        std::string_view source_name;
        std::string_view source_tag;
        control_log* log;
        std::string_view name() const { return source_name; }
        auto tags() const { return std::array{ source_tag }; }
        definition_type compile(givm::definition_compile_context&) const { return { log }; }
        static givm::program_entry handle(const definition_type& data, const givm::attachment_view&,
            givm::attachment_reapplication&, givm::handle_context&)
        {
            ++data.log->reapplied;
            return {};
        }
    };

    struct control_driver
    {
        using definition_category = givm::skill_view;
        struct definition_type
        {
            control_log* log;
            givm::program_entry dynamic_operations;
            givm::program_entry end_phase;
            givm::program_entry round_start;
            givm::definition_id<givm::attachment_view> control;
        };
        control_log* log;
        bool dynamic_operations;
        std::span<const givm::any_command> end_phase;
        bool pause_round_start = false;
        std::string_view name() const { return "ControlDriver"; }
        auto attachment_dependencies() const { return std::array{ std::string_view{ "Control" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log,
                dynamic_operations ? context.add_program(std::tuple{
                    givm::attach{}, givm::add_attachment{}, givm::set_active_character{} }) : givm::program_entry{},
                end_phase.empty() ? givm::program_entry{} : context.add_program(end_phase),
                pause_round_start ? context.add_program(std::tuple{ givm::replace_cards{ givm::player_id{ 0 } } }) : givm::program_entry{},
                context.resolve_id<givm::attachment_view>("Control") };
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::test_event&, givm::handle_context& context)
        {
            if(not data.dynamic_operations)
            {
                data.log->controlled_at_checkpoint.push_back(data.log->library->is_controlled(context.table()[target]));
                return {};
            }
            return context.invoke(data.dynamic_operations,
                givm::attachment_application{ .target = actor, .definition = data.control, .state = { 1 } },
                givm::attachment_addition{ .target = actor, .definition = data.control, .state = { 1 } },
                givm::active_character_changed{ .current = ally });
        }
        static givm::program_entry handle(const definition_type&, const givm::skill_view&,
            givm::cost_of_switch& event, givm::handle_context&)
        {
            event.requirement.dice_requirement.any = 0;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::active_character_changed& event, givm::handle_context& context)
        {
            CHECK(context.table()[event.current.player_id].state().active_character == event.current);
            data.log->switches.push_back(event.current);
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::damage_effect& event, givm::handle_context& context)
        {
            data.log->values_before_shield.push_back(event.value);
            data.log->controlled_before_shield.push_back(data.log->library->is_controlled(context.table()[event.target]));
            if(data.log->shield && (event.type == givm::damage_type::physical || event.type == givm::damage_type::pyro))
                event.value = 0;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::after_damage& event, givm::handle_context& context)
        {
            data.log->final_values.push_back(event.value);
            data.log->controlled_after_damage.push_back(data.log->library->is_controlled(context.table()[event.target]));
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::round_ended&, givm::handle_context& context)
        {
            data.log->controlled_at_end_phase.push_back(data.log->library->is_controlled(context.table()[target]));
            return data.end_phase ? context.invoke(data.end_phase) : givm::program_entry{};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::round_started&, givm::handle_context& context)
        {
            ++data.log->rounds_started;
            data.log->dice_at_round_start.push_back({ context.table()[givm::player_id{ 0 }].state().dice.total(),
                context.table()[givm::player_id{ 1 }].state().dice.total() });
            return data.round_start ? context.invoke(data.round_start) : givm::program_entry{};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::attachment_removed& event, givm::handle_context& context)
        {
            if(context.table()[event.attachment].definition_id() == data.log->library->frozen_id())
                ++data.log->frozen_removed;
            return {};
        }
    };

    struct driver_character
    {
        using definition_category = givm::character_view;
        struct definition_type { givm::definition_id<givm::skill_view> skill; };
        std::string_view name() const { return "DriverCharacter"; }
        auto skill_dependencies() const { return std::array{ std::string_view{ "ControlDriver" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_id<givm::skill_view>("ControlDriver") };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 20, .health = 20 };
        }
        static givm::definition_id<givm::skill_view> query(const definition_type& data, const givm::character_initial_skill& query)
        {
            return query.skill_index == 0 ? data.skill : givm::definition_id<givm::skill_view>{};
        }
    };

    struct talent_card
    {
        using definition_category = givm::card_definition;
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
        const control_driver driver{ &log, dynamic_operations, end_phase, pause_round_start };
        const givm::test::initialized_character_source ordinary{ "PlainCharacter", { .max_health = 20, .health = 20 } };
        const tagged_attachment control{ "Control", "control", &log };
        const tagged_attachment immunity{ "Immunity", "control_immunity", &log };
        const tagged_attachment ordinary_attachment{ "Ordinary", "ordinary", &log };
        const talent_card card;
        const givm::definition_source_library sources{
            givm::genshin_impact::dendro_core_3_3_0, givm::genshin_impact::catalyzing_field_3_4_0,
            givm::genshin_impact::burning_flame_3_3_0, givm::genshin_impact::frozen_3_3_0,
            character, driver, ordinary, control, immunity, ordinary_attachment, card };
        const auto ids = sources.make_issued_id_map();
        return compile(sources, program(ids), round,
            observed ? givm::compile_mode::observed : givm::compile_mode::normal);
    }

    void load_scenario(givm::table& table, const givm::definition_library& library, const givm::issued_id_map& ids)
    {
        const auto ordinary = ids.get_id<givm::character_view>("PlainCharacter");
        load_deck(table, library,
            { .cards = { ids.get_id<givm::card_definition>("ControlledTalent") },
                .characters = { ids.get_id<givm::character_view>("DriverCharacter"), ordinary } },
            { .characters = { ordinary, ordinary } });
    }

    struct zero_random { std::uint32_t operator()() const { return 0; } };

    givm::execution_state advance(givm::executor& executor, const givm::definition_library& library,
        givm::table& table, zero_random& random)
    {
        for(;;)
        {
            const auto state = executor.step(library, table, random);
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
    const auto [library, ids] = compile_scenario(log, observed, [&](const givm::issued_id_map& ids)
    {
        std::vector<givm::any_command> commands;
        const auto control = ids.get_id<givm::attachment_view>("Control");
        if(preexisting) commands.emplace_back(givm::add_attachment{ .definition = control, .state = { 1 } });
        if(immune) commands.emplace_back(givm::add_attachment{ .definition = ids.get_id<givm::attachment_view>("Immunity") });
        commands.emplace_back(givm::add_attachment{ .definition = ids.get_id<givm::attachment_view>("Ordinary") });
        if(dynamic) commands.emplace_back(givm::test_command{});
        else
        {
            commands.emplace_back(givm::attach{ .definition = control, .state = { 1 } });
            commands.emplace_back(givm::add_attachment{ .definition = control, .state = { 1 } });
            commands.emplace_back(givm::set_active_character{ givm::relative_character_target{ givm::relative_player::self, 1 } });
        }
        commands.emplace_back(givm::draw_cards{ .count = 1 });
        commands.emplace_back(givm::begin_action{});
        commands.emplace_back(givm::end_game{ givm::game_result::both_loss });
        return commands;
    }, dynamic);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    log.library = &library;
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    REQUIRE(advance(executor, library, table, random) == givm::execution_state::action_selection);
    CHECK(library.is_control(ids.get_id<givm::attachment_view>("Control")));
    CHECK(library.is_control(library.frozen_id()));
    CHECK_FALSE(library.is_control(ids.get_id<givm::attachment_view>("Immunity")));
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
    view.calculate_switch_cost(library, table, 0);
    REQUIRE(view.switch_payment_validate(table, 0, {}) == givm::switch_payment_validation::valid);
    view.switch_active_character(0, {});
    REQUIRE(advance(executor, library, table, random) == givm::execution_state::action_selection);
    CHECK(table[givm::player_id{ 0 }].state().active_character == selected);
    CHECK(log.switches.back() == selected);
}

TEST_CASE("control immunity prevents overload switching without suppressing its damage", "[control][overload]")
{
    const bool observed = GENERATE(false, true);
    const bool immune = GENERATE(false, true);
    control_log log;
    const std::array damages{ givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::pyro } };
    const auto [library, ids] = compile_scenario(log, observed, [&](const givm::issued_id_map& ids)
    {
        std::vector<givm::any_command> commands;
        if(immune) commands.emplace_back(givm::attach{ .player = givm::relative_player::opponent,
            .definition = ids.get_id<givm::attachment_view>("Immunity") });
        commands.emplace_back(givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::electro });
        commands.emplace_back(givm::deal_damage{ .damages = damages });
        commands.emplace_back(givm::end_game{ givm::game_result::both_loss });
        return commands;
    });
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    log.library = &library;
    givm::executor executor;
    executor.enter_entry(library);
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
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = givm::damage_type::cryo },
        givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1, .type = shatter_type }
    };
    const auto [library, ids] = compile_scenario(log, observed, [&](const givm::issued_id_map& ids)
    {
        std::vector<givm::any_command> commands;
        if(immune) commands.emplace_back(givm::attach{ .player = givm::relative_player::opponent,
            .definition = ids.get_id<givm::attachment_view>("Immunity") });
        commands.emplace_back(givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::hydro });
        commands.emplace_back(givm::deal_damage{ .damages = damages });
        commands.emplace_back(givm::end_game{ givm::game_result::both_loss });
        return commands;
    });
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    log.library = &library;
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    std::size_t observations = 0;
    for(;;)
    {
        const auto state = advance(executor, library, table, random);
        if(state == givm::execution_state::finished) break;
        REQUIRE(observed);
        REQUIRE(state == givm::execution_state::health_reduced);
        const auto damage = executor.view_in<givm::execution_state::health_reduced>();
        CHECK(damage.reaction() == (observations == 0 ? givm::elemental_reaction::frozen : givm::elemental_reaction::none));
        ++observations;
    }
    CHECK(table[target].state().health == 18);
    CHECK(log.values_before_shield == std::vector<std::uint32_t>{ 2, immune ? 1u : 3u });
    CHECK(log.controlled_before_shield == std::vector<bool>{ false, false });
    CHECK(log.final_values == std::vector<std::uint32_t>{ 2, 0 });
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
    const std::array damages{ givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 1,
        .type = preexisting ? givm::damage_type::pyro : givm::damage_type::cryo } };
    const std::array<givm::any_command, 1> end_phase{ givm::deal_damage{ .damages = damages } };
    const std::array<givm::any_command, 3> round{
        givm::start_dice_roll_phase{ .count = 0, .reroll_count = { 0, 0 } }, givm::start_round{},
        givm::end_game{ givm::game_result::both_loss } };
    const auto [library, ids] = compile_scenario(log, observed, [&](const givm::issued_id_map&)
    {
        std::vector<givm::any_command> commands;
        commands.emplace_back(givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::hydro });
        if(preexisting) commands.emplace_back(givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::cryo });
        commands.emplace_back(givm::end_round{});
        commands.emplace_back(givm::test_command{});
        return commands;
    }, false, end_phase, false, round);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    log.library = &library;
    givm::executor executor;
    executor.enter_entry(library);
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
        const auto finish = [&](givm::executor& running, givm::table& current)
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
    const std::array damages{ givm::fixed_damage{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .value = 20, .type = givm::damage_type::cryo } };
    const auto [library, ids] = compile_scenario(log, false, [&](const givm::issued_id_map&)
    {
        std::vector<givm::any_command> commands;
        commands.emplace_back(givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::hydro });
        commands.emplace_back(givm::deal_damage{ .damages = damages });
        commands.emplace_back(givm::end_game{ givm::game_result::both_loss });
        return commands;
    });
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    log.library = &library;
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
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
    const std::array<givm::any_command, 3> round{
        givm::start_dice_roll_phase{ .count = 2, .reroll_count = { 1, 1 } }, givm::start_round{},
        givm::end_game{ givm::game_result::both_loss } };
    const auto [library, ids] = compile_scenario(log, observed, [&](const givm::issued_id_map& ids)
    {
        std::vector<givm::any_command> commands;
        commands.emplace_back(givm::attach{ .player = givm::relative_player::opponent,
            .definition = ids.get_id<givm::attachment_view>("frozen-3.3.0-genshin_impact") });
        return commands;
    }, false, {}, true, round);
    givm::table table{ { .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    log.library = &library;
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    auto state = advance(executor, library, table, random);
    if(observed)
    {
        REQUIRE(state == givm::execution_state::round_started);
        CHECK(table.state().round_number == 1);
        CHECK(log.rounds_started == 0);
        CHECK(library.is_controlled(table[target]));
        state = advance(executor, library, table, random);
    }
    REQUIRE(state == givm::execution_state::dice_selection);
    CHECK(table.state().round_number == 1);
    CHECK(log.rounds_started == 0);
    CHECK(library.is_controlled(table[target]));
    executor.view_in<givm::execution_state::dice_selection>().select({});
    REQUIRE(advance(executor, library, table, random) == givm::execution_state::dice_selection);
    CHECK(log.rounds_started == 0);
    CHECK(library.is_controlled(table[target]));
    executor.view_in<givm::execution_state::dice_selection>().select({});
    REQUIRE(advance(executor, library, table, random) == givm::execution_state::card_selection);
    CHECK(log.rounds_started == 1);
    CHECK(log.dice_at_round_start == std::vector<std::array<std::uint32_t, 2>>{ { 2, 2 } });
    CHECK(library.is_controlled(table[target]));
    auto copied_executor = executor;
    auto copied_table = table;
    const auto finish = [&](givm::executor& running, givm::table& current)
    {
        running.view_in<givm::execution_state::card_selection>().select({});
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
    const std::array<givm::any_command, 3> round{
        givm::start_dice_roll_phase{}, givm::start_round{}, givm::end_game{ givm::game_result::both_loss } };
    const auto [library, ids] = compile_scenario(log, observed, [&](const givm::issued_id_map& ids)
    {
        std::vector<givm::any_command> commands;
        commands.emplace_back(givm::attach{ .player = givm::relative_player::opponent,
            .definition = ids.get_id<givm::attachment_view>("frozen-3.3.0-genshin_impact") });
        return commands;
    }, false, {}, false, round);
    givm::table table{ givm::table_state{ .max_rounds = 0, .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    log.library = &library;
    givm::executor executor;
    executor.enter_entry(library);
    zero_random random;
    auto state = advance(executor, library, table, random);
    if(observed)
    {
        REQUIRE(state == givm::execution_state::round_started);
        state = advance(executor, library, table, random);
    }
    REQUIRE(state == givm::execution_state::finished);
    CHECK(table.state().round_number == 1);
    CHECK(log.rounds_started == 0);
    CHECK(log.frozen_removed == 0);
    CHECK(library.is_controlled(table[target]));
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 0);
    CHECK(table[givm::player_id{ 1 }].state().dice.total() == 0);
}
