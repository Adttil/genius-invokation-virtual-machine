#include <array>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/basic_definitions.hpp>
#include <givm/executor.hpp>

#include "../test_character_source.hpp"

namespace
{
    constexpr givm::player_id owner{ 0 };
    constexpr givm::player_id opponent{ 1 };
    constexpr givm::character_id actor{ owner, 0 };
    constexpr givm::character_id ally{ owner, 1 };
    constexpr givm::character_id target{ opponent, 0 };

    struct prepared_log
    {
        const givm::definition_library* library = nullptr;
        std::vector<std::string> events;
        std::vector<givm::attachment_id> effects;
        std::vector<givm::attachment_id> removed;
        std::vector<givm::player_id> opportunities;
        std::vector<bool> plunge_at_damage;
        std::vector<std::size_t> remaining_at_removal;
        std::size_t skill_notifications = 0;
        std::size_t technique_notifications = 0;
        std::size_t payment_notifications = 0;
        std::size_t switches = 0;
        bool pause_removal = false;
    };

    struct preparation_source
    {
        using definition_category = givm::attachment_view;
        struct definition_type
        {
            prepared_log* log;
            std::string_view name;
            givm::action_speed speed;
            givm::program_entry effect;
        };
        prepared_log* log;
        std::string_view source_name;
        givm::action_speed speed = givm::action_speed::combat;
        std::string_view name() const { return source_name; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, source_name, speed, context.add_program(std::tuple{ givm::deal_damage{} }) };
        }
        static givm::program_entry handle(const definition_type& data, const givm::attachment_view& self,
            givm::prepared_skill_effect& event, givm::handle_context& context)
        {
            CHECK(event.attachment == self.id());
            CHECK_FALSE(self.is_valid());
            CHECK(self.definition_id().is_valid());
            CHECK(self.character().id() == actor);
            CHECK(self.state().count != 0);
            data.log->events.push_back("effect:" + std::string{ data.name });
            data.log->effects.push_back(self.id());
            event.speed = data.speed;
            return context.invoke(data.effect, givm::deal_damage_input{ std::array{ givm::damage{
                .source = self.id(), .target = givm::relative_character_target{ givm::relative_player::opponent },
                .value = self.state().count, .type = givm::damage_type::physical,
                .flags = givm::damage_flag_bits::prepared_skill } } });
        }
    };

    struct tagged_attachment
    {
        using definition_category = givm::attachment_view;
        struct definition_type {};
        std::string_view source_name;
        std::string_view tag;
        std::string_view name() const { return source_name; }
        auto tags() const { return std::array{ tag }; }
        definition_type compile(givm::definition_compile_context&) const { return {}; }
    };

    struct observer_source
    {
        using definition_category = givm::skill_view;
        struct definition_type { prepared_log* log; givm::program_entry pause; };
        prepared_log* log;
        std::string_view name() const { return "PreparedObserver"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_program(std::tuple{ givm::replace_cards{ owner } }) };
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::before_action&, givm::handle_context& context)
        {
            data.log->opportunities.push_back(context.table().state().active_player);
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::attachment_removed& event, givm::handle_context& context)
        {
            const auto removed = context.table()[event.attachment];
            const auto definition = (*data.log->library)[removed.definition_id()];
            if(not definition.can_handle<givm::prepared_skill_effect, givm::attachment_view>()) return {};
            CHECK_FALSE(removed.is_valid());
            data.log->events.push_back("removed:" + std::string{ definition.name() });
            data.log->removed.push_back(event.attachment);
            std::size_t remaining = 0;
            for(const auto attachment : removed.character().attachments())
                if((*data.log->library)[attachment.definition_id()].can_handle<givm::prepared_skill_effect, givm::attachment_view>())
                    ++remaining;
            data.log->remaining_at_removal.push_back(remaining);
            if(data.log->pause_removal)
            {
                data.log->pause_removal = false;
                return context.invoke(data.pause);
            }
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::after_damage& event, givm::handle_context& context)
        {
            if(not event.flags.contains(givm::damage_flag_bits::prepared_skill)) return {};
            REQUIRE(std::holds_alternative<givm::attachment_id>(event.source));
            const auto source = std::get<givm::attachment_id>(event.source);
            CHECK_FALSE(context.table()[source].is_valid());
            CHECK_FALSE(event.flags.contains(givm::damage_flag_bits::skill_damage));
            CHECK_FALSE(event.flags.contains(givm::damage_flag_bits::charged_attack));
            CHECK_FALSE(event.flags.contains(givm::damage_flag_bits::plunging_attack));
            data.log->events.push_back("damage:" + std::string{ (*data.log->library)[context.table()[source].definition_id()].name() });
            data.log->plunge_at_damage.push_back(context.table()[owner].state().can_plunge);
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::active_character_changed&, givm::handle_context&)
        {
            ++data.log->switches;
            return {};
        }
        static givm::program_entry handle(const definition_type&, const givm::skill_view&,
            givm::cost_of_switch& event, givm::handle_context&)
        {
            event.requirement.dice_requirement.any = 0;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::skill_will_be_used&, givm::handle_context&)
        {
            ++data.log->skill_notifications;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::skill_used&, givm::handle_context&)
        {
            ++data.log->skill_notifications;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::technique_will_be_used&, givm::handle_context&)
        {
            ++data.log->technique_notifications;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::technique_used&, givm::handle_context&)
        {
            ++data.log->technique_notifications;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::dice_removed&, givm::handle_context&)
        {
            ++data.log->payment_notifications;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::energy_changed&, givm::handle_context&)
        {
            ++data.log->payment_notifications;
            return {};
        }
    };

    struct observer_character
    {
        using definition_category = givm::character_view;
        struct definition_type { givm::definition_id<givm::skill_view> observer; };
        std::string_view name() const { return "PreparedCharacter"; }
        auto skill_dependencies() const { return std::array{ std::string_view{ "PreparedObserver" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_id<givm::skill_view>("PreparedObserver") };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 20, .max_energy = 3, .health = 20, .energy = 3 };
        }
        static givm::definition_id<givm::skill_view> query(const definition_type& data, const givm::character_initial_skill& query)
        {
            return query.skill_index == 0 ? data.observer : givm::definition_id<givm::skill_view>{};
        }
    };

    template<class TProgram>
    auto compile_scenario(prepared_log& log, bool observed, TProgram program,
        std::span<const givm::any_command> round = {})
    {
        const observer_source observer{ &log };
        const observer_character character;
        const givm::test::initialized_character_source ordinary{ "PlainCharacter", { .max_health = 20, .health = 20 } };
        const preparation_source fast{ &log, "FastPreparation", givm::action_speed::fast };
        const preparation_source combat{ &log, "CombatPreparation" };
        const preparation_source later{ &log, "LaterPreparation" };
        const tagged_attachment immunity{ "Immunity", "control_immunity" };
        const tagged_attachment plain{ "PlainAttachment", "ordinary" };
        const givm::definition_source_library sources{
            givm::genshin_impact::dendro_core_3_3_0, givm::genshin_impact::catalyzing_field_3_4_0,
            givm::genshin_impact::burning_flame_3_3_0, givm::genshin_impact::frozen_3_3_0,
            observer, character, ordinary, fast, combat, later, immunity, plain };
        const auto ids = sources.make_issued_id_map();
        return compile(sources, program(ids), round,
            observed ? givm::compile_mode::observed : givm::compile_mode::normal);
    }

    givm::table make_table(prepared_log& log, const givm::definition_library& library, const givm::issued_id_map& ids)
    {
        log.library = &library;
        givm::dice_counts dice;
        dice[givm::elemental_dice::omni] = 2;
        givm::table table{ { .self_player = owner },
            { .dice = dice, .active_character = actor, .can_plunge = true },
            { .active_character = target } };
        const auto ordinary = ids.get_id<givm::character_view>("PlainCharacter");
        load_deck(table, library,
            { .characters = { ids.get_id<givm::character_view>("PreparedCharacter"), ordinary } },
            { .characters = { ordinary, ordinary } });
        return table;
    }

    givm::add_attachment add(const givm::issued_id_map& ids, std::string_view name, std::uint32_t count = 1)
    {
        return { .definition = ids.get_id<givm::attachment_view>(name), .state = { count } };
    }

    struct zero_random { std::uint32_t operator()() const { return 0; } };

    givm::execution_state advance(givm::executor& executor, const givm::definition_library& library, givm::table& table)
    {
        zero_random random;
        for(std::size_t steps = 0; steps != 100; ++steps)
        {
            const auto state = executor.step(library, table, random);
            switch(state)
            {
            case givm::execution_state::action_started:
            case givm::execution_state::active_character_changed:
            case givm::execution_state::health_reduced:
            case givm::execution_state::round_started:
            case givm::execution_state::round_end_declared:
            case givm::execution_state::round_ending:
                break;
            default:
                return state;
            }
        }
        FAIL("prepared action did not reach an input or finish");
        return givm::execution_state::finished;
    }
}

TEST_CASE("prepared attachments consume consecutive action opportunities in order without payment", "[prepared-skill][action]")
{
    const bool observed = GENERATE(false, true);
    prepared_log log;
    const auto [library, ids] = compile_scenario(log, observed, [&](const givm::issued_id_map& ids)
    {
        return std::tuple{ add(ids, "FastPreparation", 1), add(ids, "CombatPreparation", 2),
            add(ids, "LaterPreparation", 3), givm::begin_action{}, givm::end_game{ givm::game_result::both_loss } };
    });
    auto table = make_table(log, library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    CHECK(log.events == std::vector<std::string>{ "removed:FastPreparation", "effect:FastPreparation", "damage:FastPreparation",
        "removed:CombatPreparation", "effect:CombatPreparation", "damage:CombatPreparation" });
    CHECK(log.opportunities == std::vector<givm::player_id>{ owner, owner, opponent });
    CHECK(log.plunge_at_damage == std::vector<bool>{ true, false });
    CHECK(log.remaining_at_removal == std::vector<std::size_t>{ 2, 1 });
    CHECK(log.effects == log.removed);
    CHECK(log.payment_notifications == 0);
    CHECK(log.skill_notifications == 0);
    CHECK(log.technique_notifications == 0);
    CHECK(table[owner].state().dice.total() == 2);
    CHECK(table[actor].state().energy == 3);
    CHECK(table[target].state().health == 17);
    CHECK(table.state().active_player == opponent);
    CHECK(table.state().self_player == owner);
    REQUIRE(std::ranges::distance(table[actor].attachments()) == 1);
    CHECK((*table[actor].attachments().begin()).definition_id() == ids.get_id<givm::attachment_view>("LaterPreparation"));
}

TEST_CASE("control preserves a prepared attachment through declarations until the next round", "[prepared-skill][control]")
{
    const bool observed = GENERATE(false, true);
    prepared_log log;
    const std::array<givm::any_command, 3> round{ givm::start_round{}, givm::begin_action{},
        givm::end_game{ givm::game_result::both_loss } };
    const auto [library, ids] = compile_scenario(log, observed, [&](const givm::issued_id_map& ids)
    {
        return std::tuple{ add(ids, "CombatPreparation", 2), add(ids, "frozen-3.3.0-genshin_impact"),
            givm::begin_action{}, givm::end_round{} };
    }, round);
    auto table = make_table(log, library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    CHECK(executor.view_in<givm::execution_state::action_selection>().is_controlled(library, table));
    CHECK(log.effects.empty());
    executor.view_in<givm::execution_state::action_selection>().declare_round_end();
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    CHECK(table.state().active_player == opponent);
    CHECK(log.effects.empty());
    CHECK(std::ranges::distance(table[actor].attachments()) == 2);
    executor.view_in<givm::execution_state::action_selection>().declare_round_end();
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    CHECK(table.state().round_number == 1);
    CHECK_FALSE(library.is_controlled(table[actor]));
    CHECK(table.state().active_player == opponent);
    CHECK(log.effects.size() == 1);
    CHECK(log.removed == log.effects);
    CHECK(std::ranges::empty(table[actor].attachments()));
    CHECK(table[target].state().health == 18);
}

TEST_CASE("only successful character changes cancel prepared attachments", "[prepared-skill][switch]")
{
    const bool observed = GENERATE(false, true);
    enum class scenario { unchanged, forced, immune, voluntary, overloaded };
    const auto choice = GENERATE(scenario::unchanged, scenario::forced, scenario::immune, scenario::voluntary, scenario::overloaded);
    const bool cancelled = choice == scenario::forced || choice == scenario::voluntary || choice == scenario::overloaded;
    prepared_log log;
    const std::array damage{ givm::fixed_damage{
        .source = { givm::relative_player::opponent, 0 }, .target = { givm::relative_player::self, 0 },
        .value = 1, .type = givm::damage_type::pyro } };
    const auto [library, ids] = compile_scenario(log, observed, [&](const givm::issued_id_map& ids)
    {
        std::vector<givm::any_command> commands{ add(ids, "CombatPreparation"), add(ids, "LaterPreparation"),
            add(ids, "PlainAttachment") };
        if(choice == scenario::voluntary)
        {
            commands.emplace_back(add(ids, "frozen-3.3.0-genshin_impact"));
            commands.emplace_back(givm::begin_action{});
        }
        else if(choice == scenario::overloaded)
        {
            commands.emplace_back(givm::apply_element{ .source = givm::relative_character_target{},
                .target = givm::relative_character_target{}, .element = givm::element::electro });
            commands.emplace_back(givm::deal_damage{ .damages = damage });
        }
        else
        {
            if(choice == scenario::immune) commands.emplace_back(add(ids, "Immunity"));
            commands.emplace_back(givm::set_active_character{
                givm::relative_character_target{ givm::relative_player::self, choice == scenario::unchanged ? 0 : 1 } });
        }
        commands.emplace_back(givm::end_game{ givm::game_result::both_loss });
        return commands;
    });
    auto table = make_table(log, library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    auto state = advance(executor, library, table);
    if(choice == scenario::voluntary)
    {
        REQUIRE(state == givm::execution_state::action_selection);
        const auto view = executor.view_in<givm::execution_state::action_selection>();
        REQUIRE(view.is_controlled(library, table));
        REQUIRE(view.switch_target_count() == 1);
        view.calculate_switch_cost(library, table, 0);
        REQUIRE(view.switch_payment_validate(table, 0, {}) == givm::switch_payment_validation::valid);
        view.switch_active_character(0, {});
        REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    }
    else REQUIRE(state == givm::execution_state::finished);
    CHECK(log.effects.empty());
    CHECK(log.removed.size() == (cancelled ? 2 : 0));
    if(cancelled) CHECK(log.remaining_at_removal == std::vector<std::size_t>{ 0, 0 });
    CHECK(log.switches == (cancelled ? 1 : 0));
    CHECK(table[owner].state().active_character == (cancelled ? ally : actor));
    const auto plain = ids.get_id<givm::attachment_view>("PlainAttachment");
    CHECK(std::ranges::any_of(table[actor].attachments(), [&](auto attachment) { return attachment.definition_id() == plain; }));
}

TEST_CASE("prepared effects resume after a removal response and copying the suspended game", "[prepared-skill][copy]")
{
    const bool observed = GENERATE(false, true);
    prepared_log log{ .pause_removal = true };
    const auto [library, ids] = compile_scenario(log, observed, [&](const givm::issued_id_map& ids)
    {
        return std::tuple{ add(ids, "CombatPreparation", 2), givm::begin_action{},
            givm::end_game{ givm::game_result::both_loss } };
    });
    auto table = make_table(log, library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(advance(executor, library, table) == givm::execution_state::card_selection);
    CHECK(log.effects.empty());
    REQUIRE(log.removed.size() == 1);
    CHECK_FALSE(table[log.removed.front()].is_valid());
    CHECK(table[target].state().health == 20);
    auto copied_executor = executor;
    auto copied_table = table;
    const auto finish = [&](givm::executor& running, givm::table& current)
    {
        running.view_in<givm::execution_state::card_selection>().select({});
        REQUIRE(advance(running, library, current) == givm::execution_state::action_selection);
        CHECK(current[target].state().health == 18);
        CHECK(current.state().active_player == opponent);
        CHECK(current.state().self_player == owner);
        CHECK_FALSE(current[owner].state().can_plunge);
    };
    finish(executor, table);
    CHECK(copied_table[target].state().health == 20);
    finish(copied_executor, copied_table);
    CHECK(log.effects == std::vector<givm::attachment_id>{ log.removed.front(), log.removed.front() });
}

TEST_CASE("switch cancellation resumes remaining removal notifications independently after copying", "[prepared-skill][switch][copy]")
{
    const bool observed = GENERATE(false, true);
    prepared_log log{ .pause_removal = true };
    const auto [library, ids] = compile_scenario(log, observed, [&](const givm::issued_id_map& ids)
    {
        return std::tuple{ add(ids, "CombatPreparation"), add(ids, "LaterPreparation"),
            add(ids, "PlainAttachment"),
            givm::set_active_character{ givm::relative_character_target{ givm::relative_player::self, 1 } },
            givm::end_game{ givm::game_result::both_loss } };
    });
    auto table = make_table(log, library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    if(observed)
    {
        zero_random random;
        REQUIRE(executor.step(library, table, random) == givm::execution_state::active_character_changed);
        CHECK(table[owner].state().active_character == actor);
        CHECK(std::ranges::distance(table[actor].attachments()) == 3);
        CHECK(log.removed.empty());
    }
    REQUIRE(advance(executor, library, table) == givm::execution_state::card_selection);
    REQUIRE(log.removed.size() == 1);
    CHECK(log.remaining_at_removal == std::vector<std::size_t>{ 0 });
    CHECK(log.switches == 0);
    CHECK(table[owner].state().active_character == ally);
    CHECK(std::ranges::distance(table[actor].attachments()) == 1);
    auto copied_executor = executor;
    auto copied_table = table;
    const auto finish = [&](givm::executor& running, givm::table& current)
    {
        running.view_in<givm::execution_state::card_selection>().select({});
        REQUIRE(advance(running, library, current) == givm::execution_state::finished);
        CHECK(current[owner].state().active_character == ally);
        CHECK(current.state().self_player == owner);
        REQUIRE(std::ranges::distance(current[actor].attachments()) == 1);
        CHECK((*current[actor].attachments().begin()).definition_id() == ids.get_id<givm::attachment_view>("PlainAttachment"));
        CHECK(current[target].state().health == 20);
    };
    finish(executor, table);
    REQUIRE(log.removed.size() == 2);
    CHECK(log.switches == 1);
    CHECK_FALSE(table[log.removed.front()].is_valid());
    CHECK_FALSE(table[log.removed.back()].is_valid());
    finish(copied_executor, copied_table);
    REQUIRE(log.removed.size() == 3);
    CHECK(log.removed[0] != log.removed[1]);
    CHECK(log.removed[1] == log.removed[2]);
    CHECK(log.remaining_at_removal == std::vector<std::size_t>{ 0, 0, 0 });
    CHECK(log.switches == 2);
    CHECK(log.effects.empty());
}
