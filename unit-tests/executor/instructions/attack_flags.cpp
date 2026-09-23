#include <array>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../../table/test_definition_library.hpp"

namespace
{
    constexpr auto combined_flags = givm::skill_flag_bits::charged_attack | givm::skill_flag_bits::plunging_attack;
    static_assert(combined_flags.contains(givm::skill_flag_bits::charged_attack));
    static_assert(combined_flags.to_damage_flags().contains(givm::damage_flag_bits::plunging_attack));
    struct attack_log
    {
        std::vector<givm::skill_flags> costs;
        std::vector<givm::skill_flags> effects;
        std::vector<givm::skill_flags> used;
        std::vector<givm::damage_flags> damage;
        bool fast_skill = false;
    };

    struct attack_source
    {
        using definition_category = givm::skill_view;
        struct definition_type { attack_log* log; givm::program_entry damage; };
        attack_log* log;
        bool normal;
        std::string_view name() const { return normal ? "Normal" : "Elemental"; }
        auto tags() const { return std::array{ std::string_view{ normal ? "normal_attack" : "elemental_skill" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_program(std::tuple{ givm::deal_damage{} }) };
        }
        static givm::action_cost_requirement query(const definition_type&, const givm::skill_initial_cost&)
        {
            return { .dice_requirement = { .any = 3 } };
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view& self,
                                         givm::cost_of_skill& event, givm::handle_context&)
        {
            if(self.id() != event.skill) return {};
            data.log->costs.push_back(event.flags);
            if(event.flags.contains(givm::skill_flag_bits::charged_attack)) --event.requirement.dice_requirement.any;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view& self,
                                         givm::skill_will_be_used& event, givm::handle_context&)
        {
            if(self.id() == event.skill && data.log->fast_skill) event.speed = givm::action_speed::fast;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view& self,
                                         givm::skill_effect& event, givm::handle_context& context)
        {
            data.log->effects.push_back(event.flags);
            return context.invoke(data.damage, givm::damage{
                .source = self.id(), .target = givm::relative_character_target{ givm::relative_player::opponent },
                .value = 1, .type = givm::damage_type::physical, .flags = event.flags.to_damage_flags()
            });
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view& self,
                                         givm::after_damage& event, givm::handle_context&)
        {
            if(const auto* id = std::get_if<givm::skill_id>(&event.source); id && *id == self.id())
                data.log->damage.push_back(event.flags);
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view& self,
                                         givm::skill_used& event, givm::handle_context&)
        {
            if(self.id() == event.skill) data.log->used.push_back(event.flags);
            return {};
        }
    };

    struct attack_character
    {
        using definition_category = givm::character_view;
        using definition_type = std::array<givm::definition_id<givm::skill_view>, 2>;
        constexpr std::string_view name() const { return "AttackCharacter"; }
        constexpr auto skill_dependencies() const { return std::array<std::string_view, 2>{ "Normal", "Elemental" }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_id<givm::skill_view>("Normal"), context.resolve_id<givm::skill_view>("Elemental") };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::definition_id<givm::skill_view> query(const definition_type& data, const givm::character_initial_skill& query)
        {
            return query.skill_index < data.size() ? data[query.skill_index] : givm::definition_id<givm::skill_view>{};
        }
    };

    struct attack_card
    {
        using definition_category = givm::card_definition;
        struct definition_type { givm::card_state state; givm::program_entry effect; };
        bool fast = true;
        bool switch_character = false;
        std::int32_t switch_offset = 1;
        constexpr std::string_view name() const { return "AttackCard"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { { .cost = { .speed = fast ? givm::action_speed::fast : givm::action_speed::combat } },
                switch_character ? context.add_program(std::tuple{
                    givm::set_active_character{ givm::relative_character_target{ givm::relative_player::self, switch_offset } }
                }) : givm::program_entry{} };
        }
        static givm::card_state query(const definition_type& data, const givm::card_initial_state&) { return data.state; }
        static givm::program_entry handle(const definition_type& data, const givm::hand_card_view&,
                                         givm::card_effect&, givm::handle_context& context)
        {
            return data.effect ? context.invoke(data.effect) : givm::program_entry{};
        }
    };

    inline auto compile_attacks(givm::compile_mode mode, attack_log& log, std::uint32_t dice = 6, attack_card card = {})
    {
        return givm::test::compile_definitions_with_program(mode, std::tuple{
            givm::select_active_character_both{},
            givm::start_dice_roll_phase{ .count = dice, .reroll_count = { 0, 0 } },
            givm::draw_cards{ .count = 1 }, givm::begin_action{}
        }, std::tuple{}, attack_source{ &log, true }, attack_source{ &log, false }, attack_character{}, card);
    }

    inline givm::table attack_table(const givm::definition_library& library, const givm::issued_id_map& ids)
    {
        givm::table table{ { .self_player = givm::player_id{ 0 } } };
        const auto character = ids.get_id<givm::character_view>("AttackCharacter");
        load_deck(table, library, { .cards = { ids.get_id<givm::card_definition>("AttackCard") },
            .characters = { character, character } }, { .characters = { character } });
        return table;
    }

    inline givm::execution_state advance(givm::executor& executor, const givm::definition_library& library, givm::table& table, bool select_initial = true)
    {
        auto random = []() -> std::uint32_t { return 0; };
        auto state = executor.step(library, table, random);
        if(select_initial && state == givm::execution_state::initial_active_character_selection)
        {
            executor.view_in<givm::execution_state::initial_active_character_selection>().select({ givm::player_id{ 0 }, 0 });
            REQUIRE(executor.step(library, table, random) == givm::execution_state::remaining_active_character_selection);
            executor.view_in<givm::execution_state::remaining_active_character_selection>().select({ givm::player_id{ 1 }, 0 });
            state = executor.step(library, table, random);
        }
        while(state == givm::execution_state::active_character_changed || state == givm::execution_state::action_started
            || state == givm::execution_state::initial_active_characters_selected
            || state == givm::execution_state::health_reduced || state == givm::execution_state::round_end_declared)
            state = executor.step(library, table, random);
        return state;
    }

    inline void return_to_player_zero(givm::executor& executor, const givm::definition_library& library, givm::table& table)
    {
        if(table.state().active_player == givm::player_id{ 1 })
        {
            executor.view_in<givm::execution_state::action_selection>().declare_round_end();
            REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
        }
    }
}

TEST_CASE("normal attack previews preserve prepayment charged and plunging flags through damage", "[attack-flags][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    attack_log log;
    const auto [library, ids] = compile_attacks(mode, log);
    auto table = attack_table(library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    auto action = executor.view_in<givm::execution_state::action_selection>();
    const auto flags = action.calculate_skill_cost(library, table, 0).flags;
    CHECK(flags.contains(givm::skill_flag_bits::normal_attack));
    CHECK(flags.contains(givm::skill_flag_bits::charged_attack));
    CHECK(flags.contains(givm::skill_flag_bits::plunging_attack));
    CHECK(action.skill_cost(0).requirement.dice_requirement.any == 2);
    givm::dice_counts payment;
    payment[givm::elemental_dice::omni] = 2;
    REQUIRE(action.skill_payment_validate(table, 0, payment) == givm::skill_payment_validation::valid);
    action.use_skill(0, payment);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    CHECK(table[givm::player_id{ 0 }].state().dice.total() == 4);
    CHECK_FALSE(table[givm::player_id{ 0 }].state().can_plunge);
    REQUIRE(log.effects.size() == 1);
    REQUIRE(log.used.size() == 1);
    REQUIRE(log.damage.size() == 1);
    CHECK(log.effects[0].value() == flags.value());
    CHECK(log.used[0].value() == flags.value());
    CHECK(log.damage[0].contains(givm::damage_flag_bits::charged_attack));
    CHECK(log.damage[0].contains(givm::damage_flag_bits::plunging_attack));
    return_to_player_zero(executor, library, table);
    const auto next = executor.view_in<givm::execution_state::action_selection>().calculate_skill_cost(library, table, 0).flags;
    CHECK(next.contains(givm::skill_flag_bits::charged_attack));
    CHECK_FALSE(next.contains(givm::skill_flag_bits::plunging_attack));
}

TEST_CASE("charged attack uses even dice counts including zero and ignores non-normal skills", "[attack-flags]")
{
    const auto dice = GENERATE(0u, 5u, 6u);
    attack_log log;
    const auto [library, ids] = compile_attacks(givm::compile_mode::normal, log, dice);
    auto table = attack_table(library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    const auto action = executor.view_in<givm::execution_state::action_selection>();
    CHECK(action.calculate_skill_cost(library, table, 0).flags.contains(givm::skill_flag_bits::charged_attack) == (dice % 2 == 0));
    const auto flags = action.calculate_skill_cost(library, table, 1).flags;
    CHECK(flags.contains(givm::skill_flag_bits::elemental_skill));
    CHECK_FALSE(flags.contains(givm::skill_flag_bits::charged_attack));
    CHECK_FALSE(flags.contains(givm::skill_flag_bits::plunging_attack));
}

TEST_CASE("only final combat speed consumes plunging opportunity and effects may establish a new one", "[attack-flags][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool fast = GENERATE(false, true);
    const bool switch_character = GENERATE(false, true);
    attack_log log;
    const auto [library, ids] = compile_attacks(mode, log, 6, attack_card{ fast, switch_character });
    auto table = attack_table(library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    executor.view_in<givm::execution_state::action_selection>().play_card(library, table, 0, {});
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    return_to_player_zero(executor, library, table);
    CHECK(table[givm::player_id{ 0 }].state().can_plunge == (fast || switch_character));
    const auto flags = executor.view_in<givm::execution_state::action_selection>().calculate_skill_cost(library, table, 0).flags;
    CHECK(flags.contains(givm::skill_flag_bits::plunging_attack) == (fast || switch_character));
}

TEST_CASE("skill speed modifiers take effect before consuming the plunging opportunity", "[attack-flags][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool fast = GENERATE(false, true);
    attack_log log{ .fast_skill = fast };
    const auto [library, ids] = compile_attacks(mode, log);
    auto table = attack_table(library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    givm::dice_counts payment;
    payment[givm::elemental_dice::omni] = 3;
    executor.view_in<givm::execution_state::action_selection>().use_skill(library, table, 1, payment);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    CHECK(table[givm::player_id{ 0 }].state().can_plunge == fast);
}

TEST_CASE("setting the already active character does not renew plunging opportunity", "[attack-flags][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    attack_log log;
    const auto [library, ids] = compile_attacks(mode, log, 6, attack_card{ false, true, 0 });
    auto table = attack_table(library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    executor.view_in<givm::execution_state::action_selection>().play_card(library, table, 0, {});
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    CHECK_FALSE(table[givm::player_id{ 0 }].state().can_plunge);
}

TEST_CASE("initial character choices grant plunging opportunities and library copies preserve skill tags", "[attack-flags][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    attack_log log;
    const auto [original, ids] = givm::test::compile_definitions_with_program(mode, std::tuple{
        givm::select_active_character_both{}, givm::start_dice_roll_phase{ .count = 6, .reroll_count = { 0, 0 } },
        givm::begin_action{}
    }, std::tuple{}, attack_source{ &log, true }, attack_source{ &log, false }, attack_character{}, attack_card{});
    auto library = original;
    CHECK(library.skill_flags(ids.get_id<givm::skill_view>("Normal")).contains(givm::skill_flag_bits::normal_attack));
    auto table = attack_table(library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(advance(executor, library, table, false) == givm::execution_state::initial_active_character_selection);
    executor.view_in<givm::execution_state::initial_active_character_selection>().select({ givm::player_id{ 0 }, 1 });
    REQUIRE(advance(executor, library, table, false) == givm::execution_state::remaining_active_character_selection);
    executor.view_in<givm::execution_state::remaining_active_character_selection>().select({ givm::player_id{ 1 }, 0 });
    REQUIRE(advance(executor, library, table, false) == givm::execution_state::action_selection);
    CHECK(table[givm::player_id{ 0 }].state().can_plunge);
    CHECK(table[givm::player_id{ 1 }].state().can_plunge);
    CHECK(executor.view_in<givm::execution_state::action_selection>().calculate_skill_cost(library, table, 0)
        .flags.contains(givm::skill_flag_bits::plunging_attack));
}
