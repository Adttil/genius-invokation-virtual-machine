#include <array>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "test_character_source.hpp"

namespace givm_test::executor::settlement_records
{
namespace
{
    using namespace givm;
    constexpr character_id source{ player_id{ 0 }, 0 };
    constexpr character_id target(std::size_t index) { return { player_id{ 1 }, static_cast<std::uint32_t>(index) }; }
    enum class scenario { aggregate, repeat_dying, enter_dying, revive };

    struct record_log
    {
        scenario kind;
        std::vector<std::size_t> dying;
        std::vector<std::size_t> damage_targets;
        std::vector<std::uint32_t> damage_values;
        std::vector<bool> defeated;
        std::vector<std::uint32_t> healed_values;
        std::vector<healing_kind> healed_kinds;
        std::vector<int> order;
        std::size_t calculations = 0;
        bool revived = false;
    };

    struct observer_source
    {
        static constexpr auto category = givm::definition_category::character;
        struct definition_type
        {
            record_log* log;
            normal_effect heal;
            immediate_effect prevent;
            immediate_effect nested;
        };
        record_log* log;
        std::string_view name() const { return "RecordObserver"; }
        auto character_dependencies() const { return std::array{ std::string_view{ "RecordVictim" } }; }
        definition_type compile(definition_compile_context& context) const
        {
            const auto nested = log->kind == scenario::enter_dying
                ? context.add_immediate_effect(enter_character{ player_id{ 1 }, context.resolve_id<givm::definition_category::character>("RecordVictim") },
                    deal_damage{}, givm::heal{})
                : context.add_immediate_effect(deal_damage{}, givm::heal{});
            return { log, context.add_normal_effect(givm::heal{}), context.add_immediate_effect(givm::heal{}), nested };
        }
        static character_state query(const definition_type&, const character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static immediate_effect handle(const definition_type& data, character_will_be_defeated& event,
            handle_context<skill_view, event_category::immediate>& context, std::uint32_t = 0)
        {
            auto& log = *data.log;
            log.dying.push_back(event.target.index());
            CHECK(context.table()[event.target].state().alive);
            CHECK(context.table()[event.target].state().health == 0);
            if(log.kind == scenario::repeat_dying)
            {
                if(log.dying.size() == 1)
                    return context.invoke(data.prevent, heal_input{ std::array{ heal_input::item{ source, event.target, 2, healing_kind::prevent_defeat } } });
                if(event.target == target(1))
                    return context.invoke(data.nested,
                        deal_damage_input{ std::array{ damage{ .source = source, .target = target(0), .value = 2, .type = damage_type::physical } } },
                        heal_input{ std::array{ heal_input::item{ source, event.target, 1, healing_kind::prevent_defeat } } });
            }
            if(log.kind == scenario::enter_dying && event.target == target(0))
                return context.invoke(data.nested,
                    deal_damage_input{ std::array{ damage{ .source = source, .target = target(2), .value = 1, .type = damage_type::physical } } },
                    heal_input{ std::array{ heal_input::item{ source, event.target, 1, healing_kind::prevent_defeat } } });
            return {};
        }
        static immediate_effect handle(const definition_type& data, healing& event,
            handle_context<skill_view, event_category::immediate>&, std::uint32_t = 0)
        {
            ++data.log->calculations;
            event.value = 0;
            return {};
        }
        static normal_effect handle(const definition_type& data, healed& event,
            handle_context<skill_view>& context, std::uint32_t = 0)
        {
            data.log->healed_values.push_back(event.value);
            data.log->healed_kinds.push_back(event.kind);
            data.log->order.push_back(1);
            if(data.log->kind == scenario::revive && event.kind == healing_kind::normal && not data.log->revived)
            {
                data.log->revived = true;
                CHECK_FALSE(context.table()[target(0)].state().alive);
                return context.invoke(data.heal, heal_input{ std::array{ heal_input::item{ source, target(0), 2, healing_kind::revive } } });
            }
            return {};
        }
        static normal_effect handle(const definition_type& data, character_revived& event,
            handle_context<skill_view>& context, std::uint32_t = 0)
        {
            CHECK(event.target == target(0));
            CHECK(context.table()[event.target].state().alive);
            data.log->order.push_back(2);
            return {};
        }
        static normal_effect handle(const definition_type& data, after_damage& event,
            handle_context<skill_view>& context, std::uint32_t = 0)
        {
            data.log->damage_targets.push_back(event.target.index());
            data.log->damage_values.push_back(event.value);
            data.log->defeated.push_back(event.defeated);
            data.log->order.push_back(3);
            if(data.log->kind == scenario::aggregate && event.target == target(1))
            {
                CHECK(event.value == 2);
                CHECK(event.type.count() == 2);
                CHECK(event.type[damage_type::physical]);
                CHECK(event.type[damage_type::pyro]);
                CHECK(event.flags.contains(damage_flag_bits::normal_attack));
                CHECK(event.flags.contains(damage_flag_bits::elemental_skill));
                CHECK(event.reaction.none());
            }
            if(data.log->kind == scenario::revive)
            {
                CHECK(event.defeated);
                CHECK(context.table()[event.target].state().alive);
                CHECK(context.table()[event.target].state().health == 2);
            }
            return {};
        }
    };

    void finish(executor_driver& execution, const definition_library& library, givm::table& table)
    {
        auto state = advance_selecting_first_alive(execution, library, table, zero_random);
        while(state == execution_state::health_reduced)
            state = advance_selecting_first_alive(execution, library, table, zero_random);
        REQUIRE(state == execution_state::finished);
    }

    struct hand_log
    {
        std::vector<definition_id<givm::definition_category::card>> definitions;
        std::vector<bool> overflow;
        std::vector<bool> valid;
        std::vector<char> order;
    };

    template<bool Gate>
    struct card_source
    {
        static constexpr auto category = givm::definition_category::card;
        struct definition_type { hand_log* log; normal_effect remove; };
        std::string_view source_name;
        hand_log* log;
        std::string_view name() const { return source_name; }
        auto card_dependencies() const
        {
            if constexpr(Gate) return std::array{ std::string_view{ "Retained" } };
            else return std::array<std::string_view, 0>{};
        }
        definition_type compile(definition_compile_context& context) const
        {
            if constexpr(Gate)
                return { log, context.add_normal_effect(discard_hand_card{
                    .definition = context.resolve_id<givm::definition_category::card>("Retained") }) };
            else return { log, {} };
        }
        static normal_effect handle(const definition_type& data, this_hand_card_discard&,
            handle_context<hand_card_view>& context, std::uint32_t = 0)
        {
            if constexpr(Gate)
            {
                CHECK_FALSE(context.entity().is_valid());
                data.log->order.push_back('G');
                return context.invoke(data.remove);
            }
            else return {};
        }
    };

    struct hand_observer_source
    {
        static constexpr auto category = givm::definition_category::character;
        struct definition_type { hand_log* log; normal_effect create; };
        hand_log* log;
        std::string_view name() const { return "HandRecordObserver"; }
        auto card_dependencies() const
        {
            return std::array{ std::string_view{ "Gate" }, std::string_view{ "Retained" }, std::string_view{ "Overflow" } };
        }
        definition_type compile(definition_compile_context& context) const
        {
            const auto gate = context.resolve_id<givm::definition_category::card>("Gate");
            return { log, context.add_normal_effect(create_hand_card{ .definition = gate },
                discard_hand_card{ .definition = gate },
                create_hand_card{ .definition = context.resolve_id<givm::definition_category::card>("Retained") },
                create_hand_card{ .definition = context.resolve_id<givm::definition_category::card>("Overflow") }) };
        }
        static normal_effect handle(const definition_type& data, round_started&,
            handle_context<skill_view>& context, std::uint32_t = 0)
        {
            return context.invoke(data.create);
        }
        static normal_effect handle(const definition_type& data, hand_card_discarded&,
            handle_context<skill_view>&, std::uint32_t = 0)
        {
            data.log->order.push_back('D');
            return {};
        }
        static normal_effect handle(const definition_type& data, hand_card_added& event,
            handle_context<skill_view>& context, std::uint32_t = 0)
        {
            const auto card = context.table()[event.card];
            data.log->definitions.push_back(card.definition_id());
            data.log->valid.push_back(card.is_valid());
            data.log->overflow.push_back(event.overflow);
            data.log->order.push_back(event.overflow ? 'O' : 'A');
            return {};
        }
    };
}

TEST_CASE("damage records aggregate within a segment and dispatch nonfatal targets before fatal targets", "[settlement][records]")
{
    const auto mode = GENERATE(compile_mode::normal, compile_mode::observed);
    record_log log{ scenario::aggregate };
    const auto observer = givm::test::with_passive_skill(observer_source{ &log });
    const givm::test::initialized_character_source victim{ "RecordVictim", { .max_health = 10, .health = 10 } };
    const givm::test::initialized_character_source fragile{ "Fragile", { .max_health = 10, .health = 2 } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode, std::tuple{
        deal_damage{ .target = { relative_player::opponent, 0 }, .value = 2, .type = damage_type::physical },
        deal_damage{ .target = { relative_player::opponent, 1 }, .value = 1, .type = damage_type::physical,
            .flags = damage_flag_bits::normal_attack },
        deal_damage{ .target = { relative_player::opponent, 1 }, .value = 1, .type = damage_type::pyro,
            .flags = damage_flag_bits::elemental_skill },
        settle{}, end_game{ game_result::both_loss } }, std::tuple{}, observer, victim, fragile);
    givm::table table{ { .self_player = player_id{ 0 } }, { .active_character = source }, { .active_character = target(0) } };
    const auto id = ids.get_id<givm::definition_category::character>(victim.name());
    load_deck(table, library, { .characters = { ids.get_id<givm::definition_category::character>(observer.name()) } },
        { .characters = { ids.get_id<givm::definition_category::character>(fragile.name()), id, id } });
    executor_driver execution;
    execution.start(library, table);
    finish(execution, library, table);
    CHECK(log.damage_targets == std::vector<std::size_t>{ 1, 0 });
    CHECK(log.damage_values == std::vector<std::uint32_t>{ 2, 2 });
    CHECK(log.defeated == std::vector<bool>{ false, true });
}

TEST_CASE("segment sealing revisits saved characters killed by later dying responses", "[settlement][dying]")
{
    const auto mode = GENERATE(compile_mode::normal, compile_mode::observed);
    record_log log{ scenario::repeat_dying };
    const auto observer = givm::test::with_passive_skill(observer_source{ &log });
    const givm::test::initialized_character_source victim{ "RecordVictim", { .max_health = 10, .health = 1 } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode, std::tuple{
        deal_damage{ .target = { relative_player::opponent, 0 }, .value = 1, .type = damage_type::physical },
        deal_damage{ .target = { relative_player::opponent, 1 }, .value = 1, .type = damage_type::physical },
        settle{}, end_game{ game_result::both_loss } }, std::tuple{}, observer, victim);
    givm::table table{ { .self_player = player_id{ 0 } }, { .active_character = source }, { .active_character = target(0) } };
    const auto id = ids.get_id<givm::definition_category::character>(victim.name());
    load_deck(table, library, { .characters = { ids.get_id<givm::definition_category::character>(observer.name()) } },
        { .characters = { id, id, id } });
    executor_driver execution;
    execution.start(library, table);
    finish(execution, library, table);
    CHECK(log.dying == std::vector<std::size_t>{ 0, 1, 0 });
    CHECK(log.damage_targets == std::vector<std::size_t>{ 1, 0 });
    CHECK(log.damage_values == std::vector<std::uint32_t>{ 1, 3 });
    CHECK(log.defeated == std::vector<bool>{ false, true });
    CHECK(log.calculations == 0);
    CHECK(log.healed_values == std::vector<std::uint32_t>{ 2, 1 });
    CHECK_FALSE(table[target(0)].state().alive);
    CHECK(table[target(1)].state().alive);
}

TEST_CASE("characters entering during segment sealing can contribute new pending damage records", "[settlement][dying][enter]")
{
    record_log log{ scenario::enter_dying };
    const auto observer = givm::test::with_passive_skill(observer_source{ &log });
    const givm::test::initialized_character_source victim{ "RecordVictim", { .max_health = 10, .health = 1 } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(compile_mode::normal, std::tuple{
        deal_damage{ .target = { relative_player::opponent, 0 }, .value = 1, .type = damage_type::physical },
        settle{}, end_game{ game_result::both_loss } }, std::tuple{}, observer, victim);
    givm::table table{ { .self_player = player_id{ 0 } }, { .active_character = source }, { .active_character = target(0) } };
    const auto id = ids.get_id<givm::definition_category::character>(victim.name());
    load_deck(table, library, { .characters = { ids.get_id<givm::definition_category::character>(observer.name()) } },
        { .characters = { id, id } });
    executor_driver execution;
    execution.start(library, table);
    finish(execution, library, table);
    CHECK(table[player_id{ 1 }].characters<false>().size() == 3);
    CHECK(log.dying == std::vector<std::size_t>{ 0, 2 });
    CHECK(log.damage_targets == std::vector<std::size_t>{ 0, 2 });
    CHECK(log.defeated == std::vector<bool>{ false, true });
}

TEST_CASE("revival bypasses healing modifiers and preserves the segment defeat fact", "[settlement][heal][revive]")
{
    record_log log{ scenario::revive };
    const auto observer = givm::test::with_passive_skill(observer_source{ &log });
    const givm::test::initialized_character_source victim{ "RecordVictim", { .max_health = 10, .health = 1 } };
    const auto [library, ids] = givm::test::compile_definitions_with_program(compile_mode::normal, std::tuple{
        deal_damage{ .target = { relative_player::opponent, 0 }, .value = 1, .type = damage_type::physical },
        heal{ .target = { relative_player::self, 0 }, .value = 1 },
        settle{}, end_game{ game_result::both_loss } }, std::tuple{}, observer, victim);
    givm::table table{ { .self_player = player_id{ 0 } }, { .active_character = source }, { .active_character = target(0) } };
    const auto id = ids.get_id<givm::definition_category::character>(victim.name());
    load_deck(table, library, { .characters = { ids.get_id<givm::definition_category::character>(observer.name()) } },
        { .characters = { id, id } });
    executor_driver execution;
    execution.start(library, table);
    finish(execution, library, table);
    CHECK(log.calculations == 1);
    CHECK(log.healed_values == std::vector<std::uint32_t>{ 0, 2 });
    CHECK(log.healed_kinds == std::vector{ healing_kind::normal, healing_kind::revive });
    CHECK(log.order == std::vector<int>{ 1, 2, 1, 3 });
    CHECK(log.defeated == std::vector<bool>{ true });
    CHECK(table[player_id{ 1 }].state().active_character == target(0));
}

TEST_CASE("hand entry retention freezes at segment sealing and distinguishes overflow from later removal", "[settlement][hand]")
{
    hand_log log;
    const auto observer = givm::test::with_passive_skill(hand_observer_source{ &log });
    const auto [library, ids] = givm::test::compile_definitions_with_program(compile_mode::normal,
        std::tuple{ start_round{}, settle{}, end_game{ game_result::both_loss } }, std::tuple{},
        observer, card_source<true>{ "Gate", &log }, card_source<false>{ "Retained", &log },
        card_source<false>{ "Overflow", &log });
    givm::table table{ { .self_player = player_id{ 0 } }, { .active_character = source, .hand_limit = 1 }, {} };
    load_deck(table, library, { .characters = { ids.get_id<givm::definition_category::character>(observer.name()) } }, {});
    executor_driver execution;
    execution.start(library, table);
    finish(execution, library, table);
    CHECK(log.definitions == std::vector{ ids.get_id<givm::definition_category::card>("Retained"), ids.get_id<givm::definition_category::card>("Overflow") });
    CHECK(log.overflow == std::vector<bool>{ false, true });
    CHECK(log.valid == std::vector<bool>{ false, false });
    CHECK(log.order == std::vector<char>{ 'G', 'D', 'D', 'A', 'O' });
    CHECK(table[player_id{ 0 }].hand_card_count() == 0);
}
}
