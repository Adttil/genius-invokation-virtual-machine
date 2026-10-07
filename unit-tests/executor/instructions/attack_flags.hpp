#include <cstddef>
#include <array>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../../table/test_definition_library.hpp"

namespace givm_test::executor_instructions::attack_flags
{
constexpr std::array<std::size_t, 1> draw_positions_1{ 0 };

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
        static constexpr auto category = givm::definition_category::skill;
        struct definition_type { attack_log* log; givm::normal_effect damage; };
        attack_log* log;
        bool normal;
        std::string_view name() const { return normal ? "Normal" : "Elemental"; }
        auto tags() const { return std::array{ std::string_view{ normal ? "normal_attack" : "elemental_skill" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_normal_effect(std::tuple{ givm::deal_damage{} }) };
        }
        static givm::action_cost_requirement query(const definition_type&, const givm::skill_initial_cost&)
        {
            return { .dice_requirement = { .any = 3 } };
        }
        static givm::preview_effect handle(const definition_type& data,
                                         givm::cost_of_skill& event, givm::handle_context<givm::skill_view, givm::event_category::preview>& context)
        {
            const auto self = context.entity();
            if(self.id() != event.skill) return {};
            data.log->costs.push_back(event.flags);
            if(event.flags.contains(givm::skill_flag_bits::charged_attack)) --event.requirement.dice_requirement.any;
            return {};
        }
        static givm::immediate_effect handle(const definition_type& data,
                                         givm::skill_will_be_used& event, givm::handle_context<givm::skill_view, givm::event_category::immediate>& context, std::uint32_t = 0)
        {
            const auto self = context.entity();
            if(self.id() == event.skill && data.log->fast_skill) event.speed = givm::action_speed::fast;
            return {};
        }
        static givm::normal_effect handle(const definition_type& data,
                                         givm::this_skill_use& event, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity();
            data.log->effects.push_back(event.flags);
            return context.invoke(data.damage, givm::deal_damage_input{ std::array{ givm::damage{
                .source = self.id(), .target = givm::relative_character_target{ givm::relative_player::opponent },
                .value = 1, .type = givm::damage_type::physical, .flags = event.flags.to_damage_flags()
            } } });
        }
        static givm::immediate_effect handle(const definition_type& data,
                                         givm::damage_effect& event, givm::handle_context<givm::skill_view, givm::event_category::immediate>& context, std::uint32_t = 0)
        {
            const auto self = context.entity();
            if(const auto id = event.source.template get_if<givm::entity_category::skill>(); id && *id == self.id())
                data.log->damage.push_back(event.flags);
            return {};
        }
        static givm::normal_effect handle(const definition_type& data,
                                         givm::skill_used& event, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity();
            if(self.id() == event.skill) data.log->used.push_back(event.flags);
            return {};
        }
    };

    struct attack_character
    {
        static constexpr auto category = givm::definition_category::character;
        using definition_type = std::array<givm::optional_definition_id<givm::definition_category::skill>, 2>;
        constexpr std::string_view name() const { return "AttackCharacter"; }
        constexpr auto skill_dependencies() const { return std::array<std::string_view, 2>{ "Normal", "Elemental" }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_id<givm::definition_category::skill>("Normal"), context.resolve_id<givm::definition_category::skill>("Elemental") };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::optional_definition_id<givm::definition_category::skill> query(const definition_type& data, const givm::character_initial_skill& query)
        {
            return query.skill_index < data.size() ? data[query.skill_index] : givm::optional_definition_id<givm::definition_category::skill>{};
        }
    };

    struct attack_card
    {
        static constexpr auto category = givm::definition_category::card;
        struct definition_type { givm::card_state state; givm::normal_effect effect; };
        bool fast = true;
        bool switch_character = false;
        std::int32_t switch_offset = 1;
        constexpr std::string_view name() const { return "AttackCard"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { { .cost = { .speed = fast ? givm::action_speed::fast : givm::action_speed::combat } },
                switch_character ? context.add_normal_effect(std::tuple{
                    givm::set_active_character{ givm::relative_character_target{ givm::relative_player::self, switch_offset } }
                }) : givm::normal_effect{} };
        }
        static givm::card_state query(const definition_type& data, const givm::card_initial_state&) { return data.state; }
        static givm::normal_effect handle(const definition_type& data,
                                         givm::this_card_play&, givm::handle_context<givm::hand_card_view>& context, std::uint32_t = 0)
        {
            return data.effect ? context.invoke(data.effect) : givm::normal_effect{};
        }
    };

    inline auto compile_attacks(givm::compile_mode mode, attack_log& log, std::uint32_t dice = 6, attack_card card = {})
    {
        return givm::test::compile_definitions_with_program(mode, std::tuple{
            givm::select_active_character_both{},
            givm::start_dice_roll_phase{ .count = dice, .reroll_count = { 0, 0 } },
            givm::draw_cards{ .position = 0, .count = 1 }, givm::begin_action{}
        }, std::tuple{}, attack_source{ &log, true }, attack_source{ &log, false }, attack_character{}, card);
    }

    inline givm::table attack_table(const givm::definition_library& library, const givm::issued_id_map& ids)
    {
        givm::table table{ { .self_player = givm::player_id{ 0 } } };
        const auto character = ids.get_id<givm::definition_category::character>("AttackCharacter");
        load_deck(table, library, { .cards = { ids.get_id<givm::definition_category::card>("AttackCard") },
            .characters = { character, character } }, { .characters = { character } });
        return table;
    }

    inline givm::execution_state advance(givm_test::executor_driver& executor, const givm::definition_library& library, givm::table& table, bool select_initial = true)
    {
        auto random = []() -> std::uint32_t { return std::to_underlying(givm::elemental_dice::omni); };
        auto state = executor.advance(library, table, random);
        if(select_initial && state == givm::execution_state::initial_active_character_selection)
        {
            executor.submitted(executor.view_in<givm::execution_state::initial_active_character_selection>().select(library, table, givm_test::omni_random, { givm::player_id{ 0 }, 0 }));
            REQUIRE(executor.advance(library, table, random) == givm::execution_state::remaining_active_character_selection);
            executor.submitted(executor.view_in<givm::execution_state::remaining_active_character_selection>().select(library, table, givm_test::omni_random, { givm::player_id{ 1 }, 0 }));
            state = executor.advance(library, table, random);
        }
        while(state == givm::execution_state::active_character_changed || state == givm::execution_state::action_started
            || state == givm::execution_state::initial_active_characters_selected
            || state == givm::execution_state::health_reduced || state == givm::execution_state::round_end_declared)
            state = executor.advance(library, table, random);
        return state;
    }

    inline void return_to_player_zero(givm_test::executor_driver& executor, const givm::definition_library& library, givm::table& table)
    {
        if(table.state().active_player == givm::player_id{ 1 })
        {
            executor.submitted(executor.view_in<givm::execution_state::action_selection>().declare_round_end(library, table, givm_test::omni_random));
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
    givm_test::executor_driver executor;
    executor.start(library, table);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    auto action = executor.view_in<givm::execution_state::action_selection>();
    const auto quote_1 = action.calculate_skill_cost(library, table, 0);
    const auto flags = action.skill_cost(quote_1).flags;
    CHECK(flags.contains(givm::skill_flag_bits::normal_attack));
    CHECK(flags.contains(givm::skill_flag_bits::charged_attack));
    CHECK(flags.contains(givm::skill_flag_bits::plunging_attack));
    CHECK(action.skill_cost(quote_1).requirement.dice_requirement.any == 2);
    givm::dice_counts payment;
    payment[givm::elemental_dice::omni] = 2;
    REQUIRE(action.skill_payment_validate(table, quote_1, payment) == givm::skill_payment_validation::valid);
    executor.submitted(action.use_skill_with_cached_cost(library, table, givm_test::omni_random, quote_1, payment));
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
    const auto quote_2 = executor.view_in<givm::execution_state::action_selection>().calculate_skill_cost(library, table, 0);
    const auto next = executor.view_in<givm::execution_state::action_selection>().skill_cost(quote_2).flags;
    CHECK(next.contains(givm::skill_flag_bits::charged_attack));
    CHECK_FALSE(next.contains(givm::skill_flag_bits::plunging_attack));
}

TEST_CASE("charged attack uses even dice counts including zero and ignores non-normal skills", "[attack-flags]")
{
    const auto dice = GENERATE(0u, 5u, 6u);
    attack_log log;
    const auto [library, ids] = compile_attacks(givm::compile_mode::normal, log, dice);
    auto table = attack_table(library, ids);
    givm_test::executor_driver executor;
    executor.start(library, table);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    const auto action = executor.view_in<givm::execution_state::action_selection>();
    const auto quote_1 = action.calculate_skill_cost(library, table, 0);
    CHECK(action.skill_cost(quote_1).flags.contains(givm::skill_flag_bits::charged_attack) == (dice % 2 == 0));
    const auto quote_2 = action.calculate_skill_cost(library, table, 1);
    const auto flags = action.skill_cost(quote_2).flags;
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
    givm_test::executor_driver executor;
    executor.start(library, table);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    executor.submitted(executor.view_in<givm::execution_state::action_selection>().play_card(library, table, givm_test::omni_random, 0, {}));
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    return_to_player_zero(executor, library, table);
    CHECK(table[givm::player_id{ 0 }].state().can_plunge == (fast || switch_character));
    const auto quote_1 = executor.view_in<givm::execution_state::action_selection>().calculate_skill_cost(library, table, 0);
    const auto flags = executor.view_in<givm::execution_state::action_selection>().skill_cost(quote_1).flags;
    CHECK(flags.contains(givm::skill_flag_bits::plunging_attack) == (fast || switch_character));
}

TEST_CASE("skill speed modifiers take effect before consuming the plunging opportunity", "[attack-flags][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const bool fast = GENERATE(false, true);
    attack_log log{ .fast_skill = fast };
    const auto [library, ids] = compile_attacks(mode, log);
    auto table = attack_table(library, ids);
    givm_test::executor_driver executor;
    executor.start(library, table);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    givm::dice_counts payment;
    payment[givm::elemental_dice::omni] = 3;
    executor.submitted(executor.view_in<givm::execution_state::action_selection>().use_skill(library, table, givm_test::omni_random, 1, payment));
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    CHECK(table[givm::player_id{ 0 }].state().can_plunge == fast);
}

TEST_CASE("setting the already active character does not renew plunging opportunity", "[attack-flags][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    attack_log log;
    const auto [library, ids] = compile_attacks(mode, log, 6, attack_card{ false, true, 0 });
    auto table = attack_table(library, ids);
    givm_test::executor_driver executor;
    executor.start(library, table);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    executor.submitted(executor.view_in<givm::execution_state::action_selection>().play_card(library, table, givm_test::omni_random, 0, {}));
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
    CHECK(library.skill_flags(ids.get_id<givm::definition_category::skill>("Normal")).contains(givm::skill_flag_bits::normal_attack));
    auto table = attack_table(library, ids);
    givm_test::executor_driver executor;
    executor.start(library, table);
    REQUIRE(advance(executor, library, table, false) == givm::execution_state::initial_active_character_selection);
    executor.submitted(executor.view_in<givm::execution_state::initial_active_character_selection>().select(library, table, givm_test::omni_random, { givm::player_id{ 0 }, 1 }));
    REQUIRE(advance(executor, library, table, false) == givm::execution_state::remaining_active_character_selection);
    executor.submitted(executor.view_in<givm::execution_state::remaining_active_character_selection>().select(library, table, givm_test::omni_random, { givm::player_id{ 1 }, 0 }));
    REQUIRE(advance(executor, library, table, false) == givm::execution_state::action_selection);
    CHECK(table[givm::player_id{ 0 }].state().can_plunge);
    CHECK(table[givm::player_id{ 1 }].state().can_plunge);
    const auto quote_1 = executor.view_in<givm::execution_state::action_selection>().calculate_skill_cost(library, table, 0);
    CHECK(executor.view_in<givm::execution_state::action_selection>().skill_cost(quote_1)
        .flags.contains(givm::skill_flag_bits::plunging_attack));
}
}
